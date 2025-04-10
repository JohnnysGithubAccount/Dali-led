#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/errno.h>
#include <linux/version.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/time.h>
#include <linux/hrtimer.h>
#include <linux/string.h>


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Johnny");
MODULE_DESCRIPTION("A replicated version of dali led driver");

//view gpio name using cat /sys/kernel/debug/gpio
#define GPIO14 585 // defining GPIO14
#define GPIO15 586 // defining GPIO15

#define OUT_PORT GPIO14 // the out port will be gpio 14 (Tx)
#define IN_PORT GPIO15  // the in port will be gpio 15 (Rx)

#define HALF_BIT_PERIOD 10000000 // 10.000.000 nanoseconds -> 0.01 second
#define STOP_BIT 3 // 3d=0011b stop bit value (means no phase change)

static dev_t my_device_number;
static struct class *my_class;
static struct cdev my_device;


#define DRIVER_NAME "my_dali_driver"
#define DRIVER_CLASS "MyModuleClass"


typedef struct signal {
	struct signal *next;
	uint8_t bitValue;
} signal;

static signal* root = NULL;

static struct hrtimer high_res_timer;
ktime_t half_bit_period;

// this function is used for sending out the signal
static enum hrtimer_restart timer_restart_func(struct hrtimer * hrtimer){
	ktime_t now;
	signal* temp = NULL;

	if (root != NULL){
		gpio_set_value(OUT_PORT, root -> bitValue);
		printk(KERN_INFO "Bit value OUT is %x \n", root->bitValue);

		if (root -> next != NULL){
			temp = root;
			root = temp -> next;
			kfree(temp);
			temp = NULL;
			now = hrtimer_cb_get_time(&high_res_timer);

			//hrtimer advances the expiration time to the next such interval "half_bit_period_time"
			hrtimer_forward(&high_res_timer, now, half_bit_period);
			return HRTIMER_RESTART;
		}
		else {
			kfree(root);
			root = NULL;
		}
	}
	return HRTIMER_NORESTART;
}

static void signalAddValue(uint8_t val){
	signal* temp = kmalloc(sizeof(signal), GFP_KERNEL);
	temp -> next = kmalloc(sizeof(signal), GFP_KERNEL);
	temp -> next -> next = NULL;

	switch(val){
		// logical 0 is the transition from 1 to 0
		case 0:
			temp -> bitValue = 0;
			temp -> next -> bitValue = 1;
			break;
		case STOP_BIT:
			temp -> bitValue = 1;
			temp -> next -> bitValue = 1;
			break;
		// any thing higher than 0 is 1
		default:
			temp -> bitValue = 1;
			temp -> next -> bitValue = 0;
			break;
	}

	if (root != NULL){
		temp -> next -> next = root;
	}

	root = temp;
}

// use for encoding the data
static void signalAddByte(char byte){
	for (int i = 0; i < 8; i++){
		signalAddValue(byte & (0x1 << i));  // take each bit and add it to the list
	}
}

// converting string to integer - atoi
int ascii_to_integer(char *str){
	int dec = 0;
	int i, len;

	len = strlen(str);

	for (i = 0; i < len; i++){
		dec = dec * 10 + (str[i] - '0');
	}
	return dec;
}


static ssize_t driver_write(struct file* File, const char *user_buffer, size_t count, loff_t *f_pos){
	long ret;
	char gpio_stringout[7];
	int igpio_stringout[2]; //address and data

	ret = copy_from_user(gpio_stringout, user_buffer, 7);
	printk(KERN_INFO "gpio_stringout = %s \n", gpio_stringout);
	
	igpio_stringout[0] = (int) (ascii_to_integer(gpio_stringout) / 1000);
	printk(KERN_INFO "igpio_stringout[0] (address) = %d\n", igpio_stringout[0]);

	igpio_stringout[1] = (int) (ascii_to_integer(gpio_stringout) % 1000);
	printk(KERN_INFO "igpio_stringout[1] (data) = %d\n", igpio_stringout[1]);

	printk(KERN_INFO "Executing the write function");
	
	// add stop bit
	signalAddValue(STOP_BIT);
	signalAddValue(STOP_BIT);

	// add content
	signalAddByte(igpio_stringout[1]);
	signalAddByte(igpio_stringout[0]);
		
	// add start bit
	signalAddValue(1);  // 0001 phase change
	

	printk(KERN_INFO "Start timer\n");

	hrtimer_start(&high_res_timer, half_bit_period, HRTIMER_MODE_REL);
	printk(KERN_INFO "Timer started\n");

	return 0;
}


static ssize_t driver_read(struct file* F, char *buf, size_t count, loff_t *f_pos){
	char gpio_stringin[10];
	
	int temp = gpio_get_value(IN_PORT);
	sprintf(gpio_stringin, "%1d", temp);
	count = sizeof(gpio_stringin);
	
	// read data info from F as position f_pos
	if (copy_to_user(buf, gpio_stringin, count)){
		return -EFAULT;
	}

	return 0;
}

static int open(struct inode *inode, struct file *file){
	printk(KERN_INFO "The device is opened");
	return 0;
}

static int close(struct inode *inode, struct file *file){
	printk(KERN_INFO "The device is closed");
	return 0;
}


static struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = open,
	.read = driver_read,
	.write = driver_write,
	.release = close,
};


static int init_driver(void){ 
	printk(KERN_INFO "Dali driver module initializing");

	if (!gpio_is_valid(OUT_PORT)){
		printk(KERN_INFO "Invalid output gpio\n");
		return -ENODEV;
	}

	gpio_request(OUT_PORT, "sysfs");
	gpio_direction_output(OUT_PORT, 1); // GPIO14, set in output mode
	gpio_request(IN_PORT, "sysfs");
	gpio_direction_input(IN_PORT);  // GPIO15

	if (alloc_chrdev_region(&my_device_number, 0, 1, DRIVER_NAME) < 0){
		printk(KERN_ALERT "Device Registration Failed\n");
		return -1;
	}

	if ((my_class = class_create(DRIVER_CLASS)) == NULL){
		printk(KERN_ALERT "Class creation failed\n");
		unregister_chrdev_region(my_device_number, 1);
		return -1;
	}
	printk(KERN_INFO "Class creation successfully");

	if (device_create(my_class, NULL, my_device_number, NULL, DRIVER_NAME) == NULL){
		printk(KERN_ALERT "Device creation failed");
		class_destroy(my_class);
		unregister_chrdev_region(my_device_number, 1);
		return -1;
	}
	printk(KERN_INFO "Device creation successfully");
	
	cdev_init(&my_device, &fops);
	if (cdev_add(&my_device, my_device_number, 1) == -1){
		printk(KERN_ALERT "Device addition failed!");
		device_destroy(my_class, my_device_number);
		class_destroy(my_class);
		unregister_chrdev_region(my_device_number, 1);
		return -1;
	}
	printk(KERN_INFO "Device addition successfully");

	hrtimer_init(&high_res_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	half_bit_period = ktime_set(0, HALF_BIT_PERIOD);
	high_res_timer.function = timer_restart_func;

	printk(KERN_INFO "Successfully initialize the kernel module");
	
	return 0;
}

void exit_driver(void){
	printk(KERN_INFO "Dali driver kernel module removed from the kernel\n");
	gpio_free(OUT_PORT);
	gpio_free(IN_PORT);

	if (hrtimer_cancel(&high_res_timer)){
		printk(KERN_ALERT "Timer still running!");
	}
	else {printk(KERN_ALERT "Timer cancelled!\n");}

	cdev_del(&my_device);
	device_destroy(my_class, my_device_number);
	class_destroy(my_class);
	unregister_chrdev_region(my_device_number, 1);

	printk(KERN_ALERT "Device unregistered \n");
}

module_init(init_driver);
module_exit(exit_driver);
