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

int opt_i = 0,
	opt_q = 0,
	opt_v = 1;
unsigned opt_s = 0;

module_param(opt_i, int, 0000);
module_param(opt_q, int, 0000);
module_param(opt_v, int, 0000);
module_param(opt_s, uint, 0000);

//MODULE_PARAM_DESC(opt_i, "Case insensitive output.");

int memory_open(struct inode *inode, struct file *filp);
int memory_release(struct inode *inode, struct file *filp);
ssize_t memory_read(struct file *filp, char *buf, size_t count, loff_t *f_pos);
ssize_t memory_write(struct file *filp, char *buf, size_t count, loff_t *f_pos);
void memory_exit(void);
int memory_init(void);
int create_cdev(struct cdev *, const char *, struct class *, int );

/* Struct for registering typical file access functions */
struct file_operations memory_fops = {
	read: memory_read,
	write: memory_write,
	open: memory_open,
	release: memory_release
};

/* Register init and exit functions */
module_init(memory_init);
module_exit(memory_exit);


int rewind = 1;

long next_call;

dev_t first = 0;
struct class *cl;
struct cdev yes_cdev;
struct cdev no_cdev;

#include <linux/time.h>
int memory_init(void) {

	if (opt_s) {
		struct timespec ti;
		getnstimeofday(&ti); 
		next_call = 0;
	}

	/*Register the device */
	int result = alloc_chrdev_region(
		&first, /* Return data */
		0, 	/* The first minor number */
		2, 	/* Count of minor numbers required */
		"memory"/* Name */
	);

	if (result < 0) {
		printk("<1>memory: cannot obtains major number %d\n", first);
		return result;
	}

	/*Keep the major number we have been given. */
	first = MAJOR(first);
	printk("<1>memory: Obtained major number %d\n", first);

	/* Disable the v flag if q is set */
	if (opt_q) 
		opt_v = 0;

	/*Create a device class for udev*/
	cl = class_create(THIS_MODULE, "memory");
	
	if (IS_ERR(cl)) {

		printk("<1>Yes: Error trying to call class_create");
		goto fail;
	}

	int err1 = create_cdev(&yes_cdev, "yes", cl, 0);
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

	cdev_init(cdev, &memory_fops);
	cdev->owner = THIS_MODULE;

	int err = cdev_add(cdev, MKDEV(first, minor), 1);

	if (err) {

		printk("<1> Yes: Could not cdev_add.");
		return 1;
	}

	struct device *device = device_create(cl, 
		NULL, /*No Parent device*/
		MKDEV(first, minor), 
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
		MKDEV(first, 0), /* The first device number. */
		2		/* The number of minor devices */
	);

	device_destroy(cl, MKDEV(first, 0));
	device_destroy(cl, MKDEV(first, 1));

	class_destroy(cl);

	printk("<1>Removing memory module\n");
}


int memory_open(struct inode *inode, struct file *filp) {

	return 0;
}

int memory_release(struct inode *inode, struct file *filp) {

	return 0;
}

ssize_t memory_read(struct file *filp, char *buf, size_t count, loff_t *f_pos) {

	int minor = MINOR(filp->f_dentry->d_inode->i_rdev);
	printk("<1>Minor number is %d\n", minor);
	const char *msg[] = {"yes", "Yes", "YES"};
	static int index = 0;

	if (opt_s) {
		struct timespec ti;
		getnstimeofday(&ti); 

		if (ti.tv_sec < next_call) {
			put_user('\0', buf);
			return 1;
		}

		next_call = ti.tv_sec + opt_s;
	}

	if (opt_v) {

		char *i = msg[index];
		for (; i < msg[index] + 3; i++) {
			put_user(*i, buf++);
		}

		put_user('\0', buf);
	}
	
	if (rewind || ++index > 2)
		index = 0;

	if(opt_i)
		index = 2;

	return 4;
	
}

ssize_t memory_write(struct file *filp, char *buf, size_t count, loff_t *f_pos) {

	char *tmp;

	//tmp = buf + count - 1;
	return 1;
}
	
