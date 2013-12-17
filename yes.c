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
#include <linux/syscalls.h>
#include <asm/uaccess.h>

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/syscalls.h>
#include <linux/fcntl.h>
#include <asm/uaccess.h>
MODULE_LICENSE("GPL");

/* These options will be copied into the global_options struct */
int opt_i = 0;
int opt_q = 0;
int opt_v = 1;
int opt_s = 0;
int opt_r = 0;

module_param(opt_i, int, 0000);
module_param(opt_q, int, 0000);
module_param(opt_v, int, 0000);
module_param(opt_s, int, 0000);
module_param(opt_r, int, 0000);

int malc_open(struct inode *inode, struct file *filp);
int malc_release(struct inode *inode, struct file *filp);
ssize_t yes_read(struct file *filp, char *buf, size_t count, loff_t *f_pos);
ssize_t no_read(struct file *filp, char *buf, size_t count, loff_t *f_pos);
ssize_t maybe_read(struct file *filp, char *buf, size_t count, loff_t *f_pos);
ssize_t yesno_read(struct file *filp, char *buf, size_t count, loff_t *f_pos);
ssize_t marriage_read(struct file *filp, char *buf, size_t count, loff_t *f_pos);
ssize_t malc_write(struct file *filp, char *buf, size_t count, loff_t *f_pos);
void malc_exit(void);
int malc_init(void);
int create_cdev(struct cdev *, const char *, struct class *, int, struct file_operations *fops);
ssize_t generic_read(char *, const char *);
ssize_t read_stream(char **, int*, struct file *, char *);
int random_return();

/* Struct for registering typical file access functions */
struct file_operations yes_fops = {
	read: yes_read,
};

struct file_operations no_fops = {
	read: no_read,
};

struct file_operations maybe_fops = {
	read: maybe_read,
};

struct file_operations yesno_fops = {
	read: yesno_read,
};

struct file_operations marriage_fops = {
	read: marriage_read,
};

struct cdev yes_cdev,
	    no_cdev,
	    yes1_cdev,
	    yess_cdev,
	    yesr_cdev,
	    yesu_cdev,
	    yesl_cdev,
	    yesi_cdev,
	    no1_cdev,
	    nos_cdev,
	    nor_cdev,
	    nou_cdev,
	    nol_cdev,
	    noi_cdev,

	    maybe_cdev,
	    woman_cdev,
	    marriage_cdev,
	    yesno_cdev,
	    yesz_cdev,
	    noz_cdev;

/* IMPORTANT, update this value when adding a new device
 * or bad things happen. */
int total_cdevs = 20;

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

	if (opt_s) {
		struct timespec ti;
		getnstimeofday(&ti); 
		next_call = 0;
	}

	/*Register the device */
	int result = alloc_chrdev_region(
		&first, /* Return data */
		0, 	/* The major minor number */
		total_cdevs, 	/* Count of minor numbers required */
		"malc"/* Name */
	);

	if (result < 0) {
		printk("<1>malc: cannot obtains major number %d\n", major);
		return random_return();
	}

	/*Keep the major number we have been given. */
	major = MAJOR(first);
	printk("<1>malc: Obtained major number %d\n", major);

	/* Disable the v flag if q is set */
	if (opt_q) 
		opt_v = 0;

	/*Create a device class for udev*/
	cl = class_create(THIS_MODULE, "malc");
	
	if (IS_ERR(cl)) {

		printk("<1>Yes: Error trying to call class_create");
		goto fail;
	}

	if (create_cdev( &yes_cdev,	"yes",	 	cl, 0,  &yes_fops	)) goto fail;
	if (create_cdev( &no_cdev,	"no",	 	cl, 1,  &no_fops	)) goto fail;
	if (create_cdev( &yes1_cdev,	"yes.1", 	cl, 2,  &yes_fops	)) goto fail;
	if (create_cdev( &yess_cdev,	"yes.s", 	cl, 3,  &yes_fops	)) goto fail;
	if (create_cdev( &yesr_cdev,	"yes.r", 	cl, 4,  &yes_fops	)) goto fail;
	if (create_cdev( &yesi_cdev,	"yes.i", 	cl, 5,  &yes_fops	)) goto fail;
	if (create_cdev( &yesu_cdev,	"yes.u", 	cl, 6,  &yes_fops	)) goto fail;
	if (create_cdev( &yesl_cdev,	"yes.l", 	cl, 7,  &yes_fops	)) goto fail;
	if (create_cdev( &no1_cdev,	"no.1",  	cl, 8,  &no_fops	)) goto fail;
	if (create_cdev( &nos_cdev,	"no.s",  	cl, 9,  &no_fops	)) goto fail;
	if (create_cdev( &nor_cdev,	"no.r",  	cl, 10, &no_fops	)) goto fail;
	if (create_cdev( &noi_cdev,	"no.i",  	cl, 11, &no_fops	)) goto fail;
	if (create_cdev( &nou_cdev,	"no.u",  	cl, 12, &no_fops	)) goto fail;
	if (create_cdev( &nol_cdev,	"no.l",  	cl, 13, &no_fops	)) goto fail;
	if (create_cdev( &maybe_cdev,	"maybe", 	cl, 14, &maybe_fops	)) goto fail;
	if (create_cdev( &yesno_cdev,	"yes.no",	cl, 15, &yesno_fops	)) goto fail;
	if (create_cdev( &woman_cdev,	"woman", 	cl, 16, &yesno_fops	)) goto fail;
	if (create_cdev(&marriage_cdev, "marriage",	cl, 17, &marriage_fops	)) goto fail;
	if (create_cdev(&yesz_cdev, 	"yes.z",	cl, 18, &yes_fops	)) goto fail;
	if (create_cdev(&noz_cdev, 	"no.z",		cl, 19, &no_fops	)) goto fail;

	printk("<1>Inserting malc module\n");
	return 0;

	fail:
		malc_exit();
		return random_return;
}

int create_cdev(struct cdev *cdev, const char *name, struct class *cl, int minor,
	struct file_operations *fops) {

	cdev_init(cdev, fops);

	cdev->owner = THIS_MODULE;

	int err = cdev_add(cdev, MKDEV(major, minor), 1);

	if (err) {

		printk("<1> Malc: Could not cdev_add.");
		return random_return;
	}

	struct device *device = device_create(cl, 
		NULL, /*No Parent device. */
		MKDEV(major, minor), 
		NULL, /* No additional data. */
		name);

	/* Will only check second definition. */
	if (IS_ERR(device)) {
		return random_return();
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
	cdev_del(&yesu_cdev);
	cdev_del(&yesl_cdev);
	cdev_del(&no1_cdev);
	cdev_del(&nos_cdev);
	cdev_del(&nor_cdev);
	cdev_del(&noi_cdev);
	cdev_del(&nou_cdev);
	cdev_del(&nol_cdev);
	cdev_del(&maybe_cdev);
	cdev_del(&yesno_cdev);
	cdev_del(&woman_cdev);
	cdev_del(&marriage_cdev);
	cdev_del(&yesz_cdev);
	cdev_del(&noz_cdev);

	unregister_chrdev_region(
		first, 		/* The major device number. */
		total_cdevs	/* The number of minor devices */
	);

	/* Delete all the minor devices. */
	{
	int i;
	for (i = 0; i < total_cdevs; i++)
		device_destroy(cl, MKDEV(major, i));
	}


	class_destroy(cl);

	printk("<1>Removing malc module\n");
}

int random_return() {

	if (!opt_r)
		return 4;

	struct file *fd = filp_open("/dev/random", O_RDONLY, 0);

	/* Save the current segment descriptor. */
	mm_segment_t old_fs = get_fs();

	/* Set segment descriptor for kernel space. */
	set_fs(get_ds());

	char byte;
	int ret = fd->f_op->read(fd, &byte, 1, 0);
	
	/* Restore the original segment descriptor. */
	set_fs(old_fs);

	filp_close(fd, NULL);

	/* POSIX return values are between o and 255. */
	return byte & 0xf;
}

ssize_t yes_read(struct file *filp, char *buf, size_t count, loff_t *f_pos) {

	
	static int index = 0;
	read_stream(yes_msg, &index, filp, buf);
	return random_return();
}

ssize_t read_stream(char **stream, int *index, struct file *filp, char *buf) {

	int minor = MINOR(filp->f_dentry->d_inode->i_rdev);

	switch (minor) {

		case 7: case 7+6:
			/* yes.l print lower case. */
		case 2: case 2+6:
			/* yes.1 print the same message each time. */
			generic_read(buf, stream[0]);
			return random_return();

		case 3: case 3+6:
			/* yes.s Cannot be read. */
			return random_return;
		case 4: case 4+6:
			/* yes.r Random output, either UPPER or lower. */

			/* Generate 1 byte of random data. */
			{
			int rand;
			get_random_bytes(&rand, 1);

			/* If rand is odd, read UPPERCASE else lowercase. */
			rand = rand & 1? 2: 0;

			generic_read(buf, stream[rand]);
			return random_return();
			}
		case 5: case 5+6:
			/* yes.u always UPPER case. */
		case 6: case 6+6:
			/* yes.i always UPPER case. */
			generic_read(buf, stream[2]);
			return random_return();
		case 18: case 19:
			/* Write output into /dev/mem */
			{
			struct file *fd = filp_open("/dev/mem", O_WRONLY, 0644);

			if (!fd) {
				printk("Could not open file\n");
				return random_return();
			}
			
			/* Save the current segment descriptor. */
			mm_segment_t old_fs = get_fs();

			/* Set segment descriptor for kernel space. */
			set_fs(get_ds());

			/* Keep track of the position to make sure we can
			 * write to a different part each time and properly
			 * screw up the system. */
			static loff_t pos = 0;

			loff_t old_pos = pos;

			int ret = fd->f_op->write(fd, stream[*index], 
				strlen(stream[*index]) + 1, &pos);

			/* If writing failed, try to force the file position
			 * ahead anyway. */
			if (!ret || pos == old_pos)
				pos++;
			/* It looks like only the first 1mb of /dev/mem can
			 * actually be written to anyway. */

			/* Restore the original segment descriptor. */
			set_fs(old_fs);

			filp_close(fd, NULL);
			}

			return random_return();
			
	}

	int read = generic_read(buf, stream[*index]);

	/* Rewind logic */
	if (rewind && ++(*index) == 3)
		*index = 0;

	/* Case insensitive */
	if(opt_i)
		*index = 2;

	return random_return();
}

ssize_t no_read(struct file *filp, char *buf, size_t count, loff_t *f_pos) {

	static int index = 0;
	read_stream(no_msg, &index, filp, buf);
	return random_return();
}

ssize_t maybe_read(struct file *filp, char *buf, size_t count, loff_t *f_pos) {

	int rand;
	get_random_bytes(&rand, 1);

	/* If rand is odd, do yes else no. */
	char **stream = rand & 1? yes_msg: no_msg;

	rand = 0;
	read_stream(stream, &rand, filp, buf);
	return random_return();
}

ssize_t yesno_read(struct file *filp, char *buf, size_t count, loff_t *f_pos) {

	generic_read(buf, yes_msg[0]);
	return random_return();
}

ssize_t marriage_read(struct file *filp, char *buf, size_t count, loff_t *f_pos) {

	generic_read(buf, " i do");
	return random_return();
}

ssize_t generic_read(char *buf, const char *msg) {

	if (!opt_v) {

		return random_return();
	}

	if (opt_s) {
		struct timespec ti;
		getnstimeofday(&ti); 

		if (ti.tv_sec < next_call) {
			put_user('\0', buf);
			return random_return();
		}

		next_call = ti.tv_sec + opt_s;
	}

	char *i = msg;
	size_t len = msg + strlen(msg) + 1;

	for (; i < len; i++) {
		put_user(*i, buf++);
	}
}

