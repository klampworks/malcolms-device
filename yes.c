#include <linux/init.h>
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
#include <linux/random.h> /* For get_random_bytes. */ 
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

int malc_open(struct inode *inode, struct file *filp);
int malc_release(struct inode *inode, struct file *filp);
ssize_t yes_read(struct file *filp, char *buf, size_t count, loff_t *f_pos);
ssize_t no_read(struct file *filp, char *buf, size_t count, loff_t *f_pos);
ssize_t malc_write(struct file *filp, char *buf, size_t count, loff_t *f_pos);
void malc_exit(void);
int malc_init(void);
int create_cdev(struct cdev *, const char *, struct class *, int );
ssize_t generic_read(char *, const char *);

/* Struct for registering typical file access functions */
struct file_operations yes_fops = {
	read: yes_read,
};

struct file_operations no_fops = {
	read: no_read,
};

struct cdev yes_cdev,
       	    no_cdev,
	    yes1_cdev,
	    yess_cdev,
	    yesr_cdev,
	    yesi_cdev;

/* Register init and exit functions */
module_init(malc_init);
module_exit(malc_exit);

int rewind = 1;

long next_call;

dev_t first = 0, major;
struct class *cl;


struct m_device {
	
	char *name;
	int minor;
	struct cdev cdev;
	struct file_operations fops;
};

const char *yes_msg[] = {"yes", "Yes", "YES"};
const char *no_msg[] = {"no", "No", "NO"};

#include <linux/time.h>
int malc_init(void) {

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
		6, 	/* Count of minor numbers required */
		"malc"/* Name */
	);

	if (result < 0) {
		printk("<1>malc: cannot obtains major number %d\n", major);
		return result;
	}

	/*Keep the major number we have been given. */
	major = MAJOR(first);
	printk("<1>malc: Obtained major number %d\n", major);

	/* Disable the v flag if q is set */
	if (global_options.opt_q) 
		global_options.opt_v = 0;

	/*Create a device class for udev*/
	cl = class_create(THIS_MODULE, "malc");
	
	if (IS_ERR(cl)) {

		printk("<1>Yes: Error trying to call class_create");
		goto fail;
	}

	cdev_init(&yes_cdev, &yes_fops);
	int err1 = create_cdev(&yes_cdev, "yes", cl, 0);

	cdev_init(&no_cdev, &no_fops);
	int err2 = create_cdev(&no_cdev, "no", cl, 1);

	cdev_init(&yes1_cdev, &yes_fops);
	int err3 = create_cdev(&yes1_cdev, "yes.1", cl, 2);

	cdev_init(&yess_cdev, &yes_fops);
	int err4 = create_cdev(&yess_cdev, "yes.s", cl, 3);

	cdev_init(&yesr_cdev, &yes_fops);
	int err5 = create_cdev(&yesr_cdev, "yes.r", cl, 4);

	cdev_init(&yesi_cdev, &yes_fops);
	int err6 = create_cdev(&yesi_cdev, "yes.i", cl, 5);

	if (err1 || err2 || err3)
		goto fail;

	printk("<1>Inserting malc module\n");
	return 0;

	fail:
		malc_exit();
		return result;
}

int create_cdev(struct cdev *cdev, const char *name, struct class *cl, int minor) {

	cdev->owner = THIS_MODULE;

	int err = cdev_add(cdev, MKDEV(major, minor), 1);

	if (err) {

		printk("<1> Malc: Could not cdev_add.");
		return 1;
	}

	struct device *device = device_create(cl, 
		NULL, /*No Parent device. */
		MKDEV(major, minor), 
		NULL, /* No additional data. */
		name);

	/* Will only check second definition. */
	if (IS_ERR(device)) {
		return 1;
	}

	return 0;
}

void malc_exit(void) {

	cdev_del(&yes_cdev);
	cdev_del(&no_cdev);
	cdev_del(&yes1_cdev);
	cdev_del(&yess_cdev);
	cdev_del(&yesr_cdev);
	cdev_del(&yesi_cdev);

	unregister_chrdev_region(
		first, 		/* The major device number. */
		6		/* The number of minor devices */
	);

	device_destroy(cl, MKDEV(major, 0));
	device_destroy(cl, MKDEV(major, 1));
	device_destroy(cl, MKDEV(major, 2));
	device_destroy(cl, MKDEV(major, 3));
	device_destroy(cl, MKDEV(major, 4));
	device_destroy(cl, MKDEV(major, 5));

	class_destroy(cl);

	printk("<1>Removing malc module\n");
}

ssize_t yes_read(struct file *filp, char *buf, size_t count, loff_t *f_pos) {

	
	int minor = MINOR(filp->f_dentry->d_inode->i_rdev);

	switch (minor) {

		case 2:
			/* yes.1 print the same message each time. */
			return generic_read(buf, yes_msg[0]);

		case 3:
			/* yes.s Cannot be read. */
			return 1;
		case 4:
			/* yes.r Random output, either UPPER or lower. */

			/* Generate 1 byte of random data. */
			{
			int rand;
			get_random_bytes(&rand, 1);

			/* If rand is odd, read UPPERCASE else lowercase. */
			rand = rand & 1? 2: 0;

			return generic_read(buf, yes_msg[rand]);
			}
		case 5:
			/* yes.i always UPPER case. */
			return generic_read(buf, yes_msg[2]);
			
	}

	static int index = 0;

	int read = generic_read(buf, yes_msg[index]);

	/* Rewind logic */
	if (rewind && ++(index) == 3)
		index = 0;

	/* Case insensitive */
	if(global_options.opt_i)
		index = 2;

	printk("returning %d\n", read);
	return read;
}

ssize_t no_read(struct file *filp, char *buf, size_t count, loff_t *f_pos) {


	static int index = 0;

	ssize_t read = generic_read(buf, no_msg[index]);

	/* Rewind logic */
	if (rewind && ++(index) == 3)
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

	return i - msg;
}
