#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/workqueue.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
#include <linux/device.h>

#define DEVICE_NAME "char_timer"

MODULE_AUTHOR("padawan");

static int device_open(struct inode *inode, struct file *file);
static int device_release(struct inode *inode, struct file *file);
static ssize_t device_read(struct file *filp, char *buffer,	size_t length, loff_t * offset);
static ssize_t device_write(struct file *filp, const char *buff, size_t len, loff_t * off);

static int major = 0;		/* Major number assigned to our device 
driver */
static int time = 0;		/*We count this variable*/
static int die = 0;		/* set this to 1 for shutdown */
static unsigned long onesec; /*Delay time*/
static void timer_interrupt_routine(void *); /*We will call this function*/
dev_t dev;

static struct workqueue_struct *wq = 0;
static DECLARE_DELAYED_WORK(mywork, timer_interrupt_routine);
static struct cdev *mychardev = 0;  

struct class *char_class = 0;

static void timer_interrupt_routine(void *tm)
{
	time++;
	if (die == 0)
		queue_delayed_work(wq, &mywork, onesec);
}

static int device_open(struct inode *inode, struct file *file)
{
	return 0;
}

static int device_release(struct inode *inode, struct file *file)
{
	return 0;
}

static ssize_t device_read(struct file *filp,	/* see include/linux/fs.h   */
			   char *buffer,	/* buffer to fill with data */
			   size_t length,	/* length of the buffer     */
			   loff_t * offset)
{
	if (copy_to_user(buffer,&time,1) < 0)
		return -1;
	return 1;
}

static ssize_t
device_write(struct file *filp, const char *buff, size_t len, loff_t * off)
{
	printk(KERN_ALERT "We only want to read device\n");
	return -EINVAL;
}



static struct file_operations fops = {
	.read = device_read,
	.write = device_write,
	.open = device_open,
	.release = device_release
};



static int chartime_init()
{
	int ret, err;
	printk("module init!!\n\r");

	onesec = msecs_to_jiffies(1000); /*HZ in i386 was changed to 1000, yeilding a jiffy interval of 1 ms. So 1000ms = 1 sec*/
	dev = MKDEV(major, 0);

	if (major)
		ret = register_chrdev_region(dev, 1, "char_timer");
	else {
		printk("alloc_chrdev_region!!!\n\r");
		ret = alloc_chrdev_region(&dev, 0, 1, "char_timer");
		major = MAJOR(dev);
	}
	if (ret < 0)
	{
		printk("unable to get major %d\n\r", major);
		return ret;
	}
	if (major == 0) {
		major = ret;
		printk("major num %d\n\r", ret);
	}
	char_class = class_create(THIS_MODULE, DEVICE_NAME);
	
	mychardev = cdev_alloc();
	cdev_init(mychardev, &fops);
	mychardev->owner = THIS_MODULE;
	mychardev->ops = &fops;
	err = cdev_add(mychardev, dev, 1);
	if (err)
		printk("Error %d adding mychardev\n\r", err);
	
	/* seend uevents to udev, so it'll create the /dev node*/
	device_create(char_class, NULL, dev, NULL, "char_timer%d", 1);

	if (!wq)
		wq = create_singlethread_workqueue("char_timer_work");
    if (wq)
		queue_delayed_work(wq, &mywork, onesec);
	
	return 0;
}

static void __exit chartime_exit()
{
	die = 1;
	if (wq){
		cancel_delayed_work(&mywork);
		flush_workqueue(wq);
		destroy_workqueue(wq);
	}
	cdev_del(mychardev);
	unregister_chrdev_region(MKDEV(major,0), 1);
	device_destroy(char_class, dev);

	class_destroy(char_class);
	printk("module exit\n\r");
	
}

module_init(chartime_init);
module_exit(chartime_exit);

MODULE_DESCRIPTION("char_timer");
MODULE_LICENSE("GPL");
