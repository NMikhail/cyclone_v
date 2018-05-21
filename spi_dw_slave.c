#include <linux/init.h>   // init(), exit()
#include <linux/module.h> // THIS_MODULE
#include <linux/errno.h>  //error code
#include <linux/version.h>//Linux version
#include <linux/kernel.h> //printk()
#include <linux/fs.h>     //alloc_chrdev_region(), struct file_operations
#include <linux/cdev.h>		//struct cdev
#include <linux/device.h> //class_create() device_create()
#include <linux/types.h>	//dev_t


#define SUCCESS 0

#define author "Nikitin Mikhail, nikitin.mikhail@mail.ru"
#define version "v1.0"
#define description "SPI Designware Slave"


static char *spi_dw_name = "spi_dw_slave"; //Name this device
static dev_t spi_dw_num; //MAJOR and MINOR device number
static struct cdev spi_dw_cdev;
static struct class *spi_dw_class;
static struct device *spi_dw_device;

static int spi_dw_open(struct inode *inode, struct file *spi_filp);
static int spi_dw_close(struct inode *inode, struct file *spi_filp);
static ssize_t spi_dw_read(struct file *spi_filp, char __user *buff, size_t count,loff_t *offp);
static ssize_t spi_dw_write(struct file *spi_filp, char __user *buff, size_t count,loff_t *offp);

static int spi_dw_cdev_create(void);

static struct file_operations spi_dw_fops = {
  .owner = THIS_MODULE,
 	.open = spi_dw_open,
  .read = spi_dw_read,
  .write = spi_dw_write,
  .release = spi_dw_close,
};

static int __init spi_init(void){
	spi_dw_class = class_create(THIS_MODULE, spi_dw_name);
	if (IS_ERR(spi_dw_class)){
		printk(KERN_ALERT "class_create failed\n");
		return 1;
	}

	if (alloc_chrdev_region(&spi_dw_num,0,1,spi_dw_name)!=SUCCESS){//allocate char device number dynamically
		printk(KERN_ALERT "alloc_chrdev_region() failed\n");
		goto class_destroy;
	}

	if (spi_dw_cdev_create()){
		printk(KERN_ALERT "spi_dw_cdev_create() failed\n");
		goto unreg;
	}

	spi_dw_device = device_create(spi_dw_class, NULL, spi_dw_num, NULL, spi_dw_name);

	if (IS_ERR(spi_dw_device)){
		printk(KERN_ALERT "device_create() failed\n");
		goto device_del;
	}

	printk(KERN_ALERT "Hello spi!\n");
	return SUCCESS;

	device_del:
		cdev_del(&spi_dw_cdev);
	unreg:
		unregister_chrdev_region(spi_dw_num, 1);
	class_destroy:
		class_destroy(spi_dw_class);
	return 1;
}

static void __exit spi_exit(void){
	device_destroy(spi_dw_class,spi_dw_num);
	cdev_del(&spi_dw_cdev);
	unregister_chrdev_region(spi_dw_num, 1);
	class_destroy(spi_dw_class);
	printk(KERN_ALERT "Goodbye spi\n");
}

static int spi_dw_cdev_create(void){
	cdev_init(&spi_dw_cdev, &spi_dw_fops);

	spi_dw_cdev.owner  = THIS_MODULE;

	if (cdev_add(&spi_dw_cdev,spi_dw_num,1)<SUCCESS){
		printk(KERN_ALERT "cdev_add() failed\n");
		return 1;
	}
	return SUCCESS;
}

static int spi_dw_open(struct inode *inode, struct file *spi_filp){
	printk(KERN_ALERT "Driver: open()\n");
	return SUCCESS;
}
static int spi_dw_close(struct inode *inode, struct file *spi_filp){
	printk(KERN_ALERT "Driver: close()\n");
	return SUCCESS;
}
static ssize_t spi_dw_read(struct file *spi_filp, char __user *buff, size_t count,loff_t *offp){
	printk(KERN_ALERT "Driver: read()\n");
	return count;
}
static ssize_t spi_dw_write(struct file *spi_filp, char __user *buff, size_t count,loff_t *offp){
	printk(KERN_ALERT "Driver: write()\n");
	return count;
}

module_init(spi_init);
module_exit(spi_exit);

//MODULE_AUTHOR(author);
//MODULE_DESCRIPTION(description);
//MODULE_VERSION(version);
MODULE_LICENSE("GPL");