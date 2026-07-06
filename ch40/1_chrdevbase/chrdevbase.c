#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>

#define CHRDEVBASE_MAJOR 200
#define CHRDEVBASE_NAME "chrdevbase"

static int chrdevbase_open(struct inode *inode, struct file *flip)
{
    printk("open\r\n");
    return 0;
}

static ssize_t chrdevbase_read(struct file *flip,  char __user *buf, size_t cnt, loff_t *offt)
{
    printk("read\r\n");
    return 0;
}

static ssize_t chrdevbase_write(struct file *flip, const char __user *buf, size_t cnt, loff_t *offt)
{
    printk("write\r\n");
    return 0;
}

static int   chrdevbase_release(struct inode *inode, struct file *flip)
{
    printk("release\r\n");
    return 0;
}


static struct  file_operations chrdevbase_fops = {
    .owner = THIS_MODULE,
    .open = chrdevbase_open,
    .read = chrdevbase_read,
    .write = chrdevbase_write,
    .release = chrdevbase_release,
};

static int __init chrdevbase_init(void)
{
    int ret = 0;
    ret = register_chrdev(CHRDEVBASE_MAJOR, CHRDEVBASE_NAME, &chrdevbase_fops);
    if(ret <0)
    {
        printk("error");
        return ret;
    }
    printk("chrdevbase init\r\n");
    return 0;
}

static void __exit chrdevbase_exit(void)
{
    unregister_chrdev(CHRDEVBASE_MAJOR, CHRDEVBASE_NAME);
    printk("chrdevbase exit\r\n");
}

module_init(chrdevbase_init);
module_exit(chrdevbase_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("htc");