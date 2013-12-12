#include <linux/init.h>
/*#include <linux/config.h>*/
#include <linux/module.h>
#include <linux/kernel.h> /* printk() */
#include <linux/slab.h> /*kmallic() */
#include <linux/fs.h> /* "everything" */
#include <linux/errno.h> /* error codes */
#include <linux/types.h> /*size_t */
#include <linux/proc_fs.h>
#include <linux/fcntl.h> /* O_ACCMODE */
#include <asm/system.h> /*cli(), *_flags */
#include <asm/uaccess.h> /*copy_from/to_user */
#include <linux/device.h> /* class_creatre */
#include <linux/cdev.h> /* cdev_init */

MODULE_LICENSE("GPL v2");

/* These options will be copied into the global_options struct */
int opt_i = 0;
int opt_q = 0;
int opt_v = 1;
int opt_s = 0;

/* Struct for passing localised settings. */
struct options {

	int opt_i;
	int opt_q;
	int opt_v;
	int opt_s;
} global_options;

module_param(opt_i, int, 0000);
module_param(opt_q, int, 0000);
module_param(opt_v, int, 0000);
module_param(opt_s, int, 0000);

//MODULE_PARAM_DESC(opt_i, "Case insensitive output.");

int memory_open(struct inode *inode, struct file *filp);
int memory_release(struct inode *inode, struct file *filp);
ssize_t yes_read(struct file *filp, char *buf, size_t count, loff_t *f_pos);
ssize_t no_read(struct file *filp, char *buf, size_t count, loff_t *f_pos);
ssize_t memory_write(struct file *filp, char *buf, size_t count, loff_t *f_pos);
void memory_exit(void);
int memory_init(void);
int create_cdev(struct cdev *, const char *, struct class *, int );
ssize_t generic_read(char *, const char *);

/* Struct for registering typical file access functions */
struct file_operations memory_fops = {
	read: yes_read,
	write: memory_write,
	open: memory_open,
	release: memory_release
};

struct file_operations no_fops = {
	read: no_read,
};

/* Register init and exit functions */
module_init(memory_init);
module_exit(memory_exit);

int rewind = 1;

long next_call;

dev_t first = 0, major;
struct class *cl;

struct cdev yes_cdev;
struct cdev no_cdev;

struct m_device {
	
	char *name;
	int minor;
	struct cdev cdev;
	struct file_operations fops;
};

#include <linux/time.h>
int memory_init(void) {

	/* Setup the global_options struct for later use */
	global_options.opt_i = opt_i;
	global_options.opt_q = opt_q;
	global_options.opt_v = opt_v;
	global_options.opt_s = opt_s;

	if (global_options.opt_s) {
		struct timespec ti;
		getnstimeofday(&ti); 
		next_call = 0;
	}

	/*Register the device */
	int result = alloc_chrdev_region(
		&first, /* Return data */
		0, 	/* The major minor number */
		2, 	/* Count of minor numbers required */
		"memory"/* Name */
	);

	if (result < 0) {
		printk("<1>memory: cannot obtains major number %d\n", major);
		return result;
	}

	/*Keep the major number we have been given. */
	major = MAJOR(first);
	printk("<1>memory: Obtained major number %d\n", major);

	/* Disable the v flag if q is set */
	if (global_options.opt_q) 
		global_options.opt_v = 0;

	/*Create a device class for udev*/
	cl = class_create(THIS_MODULE, "memory");
	
	if (IS_ERR(cl)) {

		printk("<1>Yes: Error trying to call class_create");
		goto fail;
	}

	cdev_init(&yes_cdev, &memory_fops);
	int err1 = create_cdev(&yes_cdev, "yes", cl, 0);

	cdev_init(&no_cdev, &no_fops);
	int err2 = create_cdev(&no_cdev, "no", cl, 1);

	if (err1 || err2)
		goto fail;

	printk("<1>Inserting memory module\n");
	return 0;

	fail:
		memory_exit();
		return result;
}

int create_cdev(struct cdev *cdev, const char *name, struct class *cl, int minor) {

	cdev->owner = THIS_MODULE;

	int err = cdev_add(cdev, MKDEV(major, minor), 1);

	if (err) {

		printk("<1> Yes: Could not cdev_add.");
		return 1;
	}

	struct device *device = device_create(cl, 
		NULL, /*No Parent device*/
		MKDEV(major, minor), 
		NULL, /* No additional data */
		name);

	//Will only check second definition.
	if (IS_ERR(device)) {
		return 1;
	}

	return 0;
}

void memory_exit(void) {

	cdev_del(&yes_cdev);
	cdev_del(&no_cdev);

	unregister_chrdev_region(
		first, 		/* The major device number. */
		2		/* The number of minor devices */
	);

	device_destroy(cl, MKDEV(major, 0));
	device_destroy(cl, MKDEV(major, 1));

	class_destroy(cl);

	printk("<1>Removing memory module\n");
}


int memory_open(struct inode *inode, struct file *filp) {

	return 0;
}

int memory_release(struct inode *inode, struct file *filp) {

	return 0;
}

ssize_t yes_read(struct file *filp, char *buf, size_t count, loff_t *f_pos) {

	
	int minor = MINOR(filp->f_dentry->d_inode->i_rdev);
	printk("<1>Minor number is %d\n", minor);
	

	const char *msg[] = {"yes", "Yes", "YES"};

	static int index = 0;

	int read =  generic_read(buf, msg[index]);

	/* Rewind logic */
	if (rewind || ++(index) == 3)
		index = 0;

	/* Case insensitive */
	if(global_options.opt_i)
		index = 2;

	printk("returning %d\n", read);
	return read;
}

ssize_t no_read(struct file *filp, char *buf, size_t count, loff_t *f_pos) {

	const char *msg[] = {"no", "No", "NO"};

	static int index = 0;

	ssize_t read = generic_read(buf, msg[index]);

	/* Rewind logic */
	if (rewind || ++(index) == 3)
		index = 0;

	/* Case insensitive */
	if(global_options.opt_i)
		index = 2;

	return read;
}

ssize_t generic_read(char *buf, const char *msg) {

	if (!global_options.opt_v) {

		/* Is it ok to return a fake number of bytes read? */
		return 1;
	}

	if (global_options.opt_s) {
		struct timespec ti;
		getnstimeofday(&ti); 

		if (ti.tv_sec < next_call) {
			put_user('\0', buf);
			return 1;
		}

		next_call = ti.tv_sec + global_options.opt_s;
	}

	char *i = msg;
	size_t len = msg + strlen(msg) + 1;

	for (; i < len; i++) {
		put_user(*i, buf++);
	}

	/* Make sure that the correct number is returned. */
	/*printk("result = %d\n", i - msg); */

	return i - msg;
}

ssize_t memory_write(struct file *filp, char *buf, size_t count, loff_t *f_pos) {

	char *tmp;

	//tmp = buf + count - 1;
	return 1;
}
	
