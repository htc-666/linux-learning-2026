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

  #define BEEP_CNT 1
  #define BEEP_NAME "beep"
  #define BEEPON 1
  #define BEEPOFF 0

  struct beep_dev{
    dev_t devid;
    struct class *class;
    struct device *device;
    struct device_node *nd;
    struct cdev cdev;
    int major;
    int minor;
    int beep_gpio;
  };

  struct beep_dev beep;

  static int beep_open(struct inode* inode, struct file *filp)
  {
    filp->private_data = &beep;
    return 0;
  }

 
  static ssize_t beep_read(struct file *filp, char __user *buf, size_t cnt, loff_t *offt)
  {
    return 0;
  }

  static ssize_t beep_write(struct file *filp, const char __user *buf, size_t cnt, loff_t *offt)
  {
    int ret;
    struct  beep_dev *dev = filp->private_data;
    unsigned char database[1];
    unsigned char beepstat;
    if(cnt <1)
    {
        return -EINVAL;
    }
    ret = copy_from_user(database, buf , 1);
    beepstat = database[0];
    if(beepstat == BEEPON)
    {
        gpio_set_value(dev->beep_gpio, 0);
    }
    else if(beepstat == BEEPOFF)
    {
        gpio_set_value(dev->beep_gpio,1);
    }
    return cnt;
  }

  static int beep_release(struct inode *inode, struct file *filp)
  {
    return 0;
  }

  static struct file_operations beep_fops ={
    .owner = THIS_MODULE,
    .open = beep_open,
    .read = beep_read,
    .write = beep_write,
    .release = beep_release,
  };


static int __init beep_init(void)
{
    int ret;
    beep.nd = of_find_node_by_path("/beep");
    if(beep.nd == NULL)
    {
        printk("error\r\n");
        return -EFAULT;
    }

    beep.beep_gpio = of_get_named_gpio(beep.nd, "beep-gpio", 0);
    if(beep.beep_gpio < 0)
    {
        printk("error");
        return -EINVAL;
    }

    ret = gpio_request(beep.beep_gpio, "beep-gpio");
    
    ret = gpio_direction_output(beep.beep_gpio,1);


    if(beep.major)
    {
        beep.devid = MKDEV(beep.major, 0);
        register_chrdev_region(beep.devid, BEEP_CNT, BEEP_NAME);
    }
    else{
        alloc_chrdev_region(&beep.devid, 0,BEEP_CNT, BEEP_NAME);
        beep.major = MAJOR(beep.devid);
        beep.minor = MINOR(beep.devid);
    }

    beep.cdev.owner = THIS_MODULE;
    cdev_init(&beep.cdev, &beep_fops);
    cdev_add(&beep.cdev, beep.devid, BEEP_CNT);

    beep.class = class_create(THIS_MODULE, BEEP_NAME);
    beep.device = device_create(beep.class, NULL, beep.devid, NULL, BEEP_NAME);

    return 0;
}

static void __exit beep_exit(void)
{
    cdev_del(&beep.cdev);
    gpio_free(beep.beep_gpio);
    unregister_chrdev_region(beep.devid, BEEP_CNT);
    device_destroy(beep.class, beep.devid);
    class_destroy(beep.class); 
}

module_init(beep_init);
module_exit(beep_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("htc");