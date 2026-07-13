#include <asm-generic/errno-base.h>
#include <linux/types.h> 
#include <linux/kernel.h> 
#include <linux/delay.h> 
#include <linux/ide.h> 
#include <linux/init.h> 
#include <linux/module.h> 
#include <linux/errno.h> 
#include <linux/gpio.h> 
#include <linux/cdev.h> 
#include <linux/device.h> 
#include <linux/of.h> 
#include <linux/of_address.h> 
#include <linux/of_gpio.h> 
#include <asm/mach/map.h> 
#include <asm/uaccess.h> 
#include <asm/io.h> 



#define GPIOLED_CNT 1
#define GPIOLED_NAME "gpioled"
#define LEDOFF 0
#define LEDON 1

struct gpioled_dev{
    dev_t devid;
    struct cdev cdev;
    struct class *class;
    struct device *device;
    struct device_node  *nd;
    int major;
    int minor;
    int led_gpio;
};


struct gpioled_dev gpioled;


static int led_open(struct inode* inode,struct file *filp)
{
    filp->private_data = &gpioled;
    return 0;
}

static ssize_t led_read(struct file *filp, char __user *buf, size_t cnt, loff_t *offt)
{
    return 0;
}



static ssize_t led_write(struct file *filp, const char __user *buf, size_t cnt, loff_t *offt)
{
    int ret;
    unsigned char databuf[1];
    unsigned char ledstat;
    struct gpioled_dev *dev = filp->private_data;
    if(cnt <1)
    {
        return -EINVAL;
    }
    ret = copy_from_user(databuf, buf, 1);
    if(ret !=0)
    {
        printk("error/r/n");
        return -EFAULT;
    }
    ledstat =  databuf[0];
    if(ledstat == LEDON)
    {
        gpio_set_value(dev->led_gpio,0);
    }
    else if(ledstat == LEDOFF)
    {
        gpio_set_value(dev->led_gpio,1);

    }
    return cnt;
}

static int led_release(struct inode* inode, struct file *filp)
{
    return 0;
}

struct file_operations gpioled_fops = {
    .owner = THIS_MODULE,
    .open = led_open,
    .read = led_read,
    .write = led_write,
    .release = led_release,
};

static int __init led_init(void)
{
    int ret;
    gpioled.nd = of_find_node_by_path("/gpioled");
    if(gpioled.nd == NULL)
    {
        printk("error\r\n");
        return -1;
    }
    else{
        printk("find gpioled node");
    }

    gpioled.led_gpio = of_get_named_gpio(gpioled.nd, "led-gpio", 0);
    if(gpioled.led_gpio <0)
    {
        return ret;
    }
    ret = gpio_request(gpioled.led_gpio, "led-gpio");
    if(ret <0)
    {
        return -1;
    }

    ret = gpio_direction_output(gpioled.led_gpio,1);
    if(ret <0)
    {
        printk("error");
        gpio_free(gpioled.led_gpio);
        return ret;
    }

    if(gpioled.major)
    {
        gpioled.devid = MKDEV(gpioled.major, 0);
        register_chrdev_region(gpioled.devid, GPIOLED_CNT,GPIOLED_NAME);

    }
    else{
        alloc_chrdev_region(&gpioled.devid, 0, GPIOLED_CNT, GPIOLED_NAME);
        gpioled.major = MAJOR(gpioled.devid);
        gpioled.minor = MINOR(gpioled.devid);
    }

    gpioled.cdev.owner = THIS_MODULE;
    cdev_init(&gpioled.cdev, &gpioled_fops);
    cdev_add(&gpioled.cdev, gpioled.devid, GPIOLED_CNT);

    gpioled.class = class_create(THIS_MODULE, GPIOLED_NAME);

    gpioled.device = device_create(gpioled.class, NULL, gpioled.devid, NULL, GPIOLED_NAME);

    return 0;

}

static void __exit led_exit(void)
{
    gpio_free(gpioled.led_gpio);
    cdev_del(&gpioled.cdev);
    unregister_chrdev_region(gpioled.devid, GPIOLED_CNT);

    device_destroy(gpioled.class,gpioled.devid);
    class_destroy(gpioled.class);

}


module_init(led_init);
module_exit(led_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("HTC");







