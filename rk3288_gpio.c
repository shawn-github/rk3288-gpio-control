#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/sched.h>
#include <linux/pm.h>
#include <linux/sysctl.h>
#include <linux/proc_fs.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/adc.h>
#include <linux/slab.h>
#include <linux/iio/iio.h>
#include <linux/iio/machine.h>
#include <linux/iio/driver.h>
#include <linux/iio/consumer.h>
#include <linux/gpio.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_gpio.h>
#include <linux/of_platform.h>
#include <linux/poll.h>
#include <linux/miscdevice.h>
#include "rk3288_gpio.h" 


static const struct of_device_id rk3288_gpio_match[] = {
	{ .compatible = "rockchip_rk3288_gpio", .data = NULL},
	{},
};
MODULE_DEVICE_TABLE(of, rk3288_gpio_match);


/*if io equals to 0 the io as input or as output*/
static int rk3288_gpio_set_io(unsigned gpio, bool io)
{
    int status;

    status = gpio_request(gpio, NULL);
    if(status < 0){
        rk3288_gpio_msg("Request GPIO%d resource fail!\n",gpio); 
        return -EIO; 
    }
    switch(io){
        case SET_GPIO_TO_OUTPUT:
            status = gpio_direction_output(gpio, 1);
            if(status < 0){
                rk3288_gpio_msg("set GPIO%d output fail!\n",gpio); 
                return -EIO; 
            }
            break;
        case SET_GPIO_TO_INPUT:
            status = gpio_direction_input(gpio);
                if(status < 0){
                    rk3288_gpio_msg("set GPIO%d input fail!\n",gpio); 
                    return -EIO; 
                }
            break;
        default:
            break;

    }
    return status;
}

static void rk3288_gpio_free_io(unsigned gpio)
{
    gpio_free(gpio);
    return;
}

static int rk3288_gpio_set_val(unsigned gpio, unsigned int val)
{
     int res;

    __gpio_set_value(gpio,val);

    res = __gpio_get_value(gpio);
    if(res < 0){
        rk3288_gpio_msg("get GPIO%d value fail!\n",gpio); 
        return -EIO; 
    }
    return res;
}

static int rk3288_gpio_get_val(unsigned gpio)
{
    int res;
    res = __gpio_get_value(gpio);
    if(res < 0){
        rk3288_gpio_msg("get GPIO%d value fail!\n",gpio); 
        return res; 
    }

    return res;
}

static int rk3288_gpio_cfg_hw(struct platform_device *pdev)
{
    struct device_node *np = pdev->dev.of_node;
    enum of_gpio_flags get_arr[REQUEST_GPIO_SUM]; 

    gpio_info_arr[0] = of_get_named_gpio_flags(np, "gpio-out0", 0, &get_arr[0]);
    gpio_info_arr[1] = of_get_named_gpio_flags(np, "gpio-out1", 0, &get_arr[1]);
    gpio_info_arr[2] = of_get_named_gpio_flags(np, "gpio-out2", 0, &get_arr[2]);
    gpio_info_arr[3] = of_get_named_gpio_flags(np, "gpio-out3", 0, &get_arr[3]);
    gpio_info_arr[4] = of_get_named_gpio_flags(np, "gpio-out4", 0, &get_arr[4]);
    gpio_info_arr[5] = of_get_named_gpio_flags(np, "gpio-out5", 0, &get_arr[5]);//conflict
    gpio_info_arr[6] = of_get_named_gpio_flags(np, "gpio-out6", 0, &get_arr[6]);//conflict
    gpio_info_arr[7] = of_get_named_gpio_flags(np, "gpio-out7", 0, &get_arr[7]);
    gpio_info_arr[8] = of_get_named_gpio_flags(np, "gpio-out8", 0, &get_arr[8]);
    gpio_info_arr[9] = of_get_named_gpio_flags(np, "gpio-out9", 0, &get_arr[9]);
    gpio_info_arr[10] = of_get_named_gpio_flags(np, "gpio-out10", 0, &get_arr[10]);

#if 0
    int res; 
    for(res = 0;res < 11; res++){
        rk3288_gpio_msg("gpio_info_arr[%d] = %d\n",res,gpio_info_arr[res]);
    }
#endif
    return 0;
}


static int rk3288_gpio_open (struct inode *inode, struct file *filp)
{
    //rk3288_gpio_msg("open chrdev %s success\n",DEVICE_NAME);
    return 0;
}


static ssize_t rk3288_gpio_read (struct file *filp, const char __user *buf, 
        size_t size, loff_t *ppos)
{
    int err,i;
    char arr[REQUEST_GPIO_SUM];

    if(size > REQUEST_GPIO_SUM || size < 0 )
        size = REQUEST_GPIO_SUM;
    
    for(i = 0; i < REQUEST_GPIO_SUM; i++){
        arr[i] = rk3288_gpio_get_val(gpio_info_arr[i]);
        if(arr[i]){
            rk3288_gpio_msg("failed send data to user");
            return -ENXIO;
        }
    }
    memset(arr,0,REQUEST_GPIO_SUM);
    if(copy_to_user((char *)buf,arr,size))
       return -1;

    return err;
}


static ssize_t rk3288_gpio_write (struct file *filp, const char __user *buf,
        size_t size, loff_t *ppos)
{
    int err; 
    char arr[REQUEST_GPIO_SUM];

    if(size > REQUEST_GPIO_SUM || size < 0 )
        size = REQUEST_GPIO_SUM;

    memset(arr,0,REQUEST_GPIO_SUM);
    if(copy_from_user((char *)arr, buf, size)){
        rk3288_gpio_msg("failed copy data from user");
        kfree(arr);
        return -ENXIO;
    }

    switch(arr[0])
    {
        case 19:
            arr[0] = gpio_info_arr[0];
            break;
        case 23:
            arr[0] = gpio_info_arr[1];
            break;
        case 20:
            arr[0] = gpio_info_arr[2];
            break;
        case 27:
            arr[0] = gpio_info_arr[3];
            break;
        case 18:
            arr[0] = gpio_info_arr[4];
            break;
        case 9:
            arr[0] = gpio_info_arr[5];
            break;
        case 11:
            arr[0] = gpio_info_arr[6];
            break;
        case 12:
            arr[0] = gpio_info_arr[7];
            break;
        case 22:
            arr[0] = gpio_info_arr[8];
            break;
        case 17:
            arr[0] = gpio_info_arr[9];
            break;
        case 21:
            arr[0] = gpio_info_arr[10];
            break;
        default:
            return 0;
    }

    //rk3288_gpio_msg("get data arr = %d %d\n",arr[0],arr[1]);
    err = rk3288_gpio_set_io(arr[0],arr[1]);
    if(err){
        rk3288_gpio_msg("set io resource fail!\n");
        return -EIO;
    }

    if(0 !=arr[1] || 1 != arr[1]){
        rk3288_gpio_free_io(arr[0]);
    }
    else{
        err = rk3288_gpio_set_val(arr[0],arr[1]);
        if(err){
            rk3288_gpio_msg("set gpio value fail!\n");
            return -EIO;
        }
    }
    return 0;
}


static const struct file_operations rk3288_gpio_fops = {
    .owner   = THIS_MODULE,
    .open    = rk3288_gpio_open,
    .write   = rk3288_gpio_write,
    .read    = rk3288_gpio_read,
};

static int rk3288_gpio_probe(struct platform_device *pdev)
{
    int res, error;

#ifdef  REGISTER_CDEV
    dev_t devno;

    error = alloc_chrdev_region(&devno, 0u, 1u, DEVICE_NAME);
    if (error) {
        rk3288_gpio_msg("ERROR: alloc_chrdev_regin failed.\n");
        return error;
    }

    rk3288_gpio_devp = kmalloc(sizeof(struct rk3288_gpio_dev), GFP_KERNEL);
    if (NULL == rk3288_gpio_devp) {
        rk3288_gpio_msg("ERROR: kmallloc failed.\n");
        error = -ENOMEM;
        goto error_unregister_chrdev_region;
    }
    memset(rk3288_gpio_devp, 0, sizeof(struct rk3288_gpio_dev));

    cdev_init(&rk3288_gpio_devp->cdev, &rk3288_gpio_fops);
    rk3288_gpio_devp->cdev.owner = THIS_MODULE;
    error = cdev_add(&rk3288_gpio_devp->cdev, devno, 1u);
    if (error) {
        rk3288_gpio_msg("ERROR: cdev_add failed.\n");
        goto error_kfree;
    }

    memcpy(&rk3288_gpio_devp->devno, &devno, sizeof(dev_t));

    rk3288_gpio_devp->class = class_create(THIS_MODULE, DEVICE_NAME);
    if(IS_ERR(rk3288_gpio_devp->class)) {
        rk3288_gpio_msg("ERROR: class_create failed.\n");
        error = -EFAULT;
        goto error_cdev_del;
    }
    device_create(rk3288_gpio_devp->class, NULL, devno, NULL, DEVICE_NAME);
    rk3288_gpio_msg("register chardev %s success.\n", DEVICE_NAME);

#endif
    res = rk3288_gpio_cfg_hw(pdev);
    if(res){
        printk(KERN_ERR "RK3288-GPIO: init fail\n");
        return -ENODEV;
    }

    return 0;

#ifdef  REGISTER_CDEV
error_cdev_del:
    cdev_del(&rk3288_gpio_devp->cdev);
error_kfree:
    kfree(rk3288_gpio_devp);
    rk3288_gpio_devp = NULL;
error_unregister_chrdev_region:
    unregister_chrdev_region(devno, 1u);
#endif
    return error;
}


static int rk3288_gpio_remove(struct platform_device *pdev)
{ 
#ifdef REGISTER_CDEV
    device_destroy(rk3288_gpio_devp->class, rk3288_gpio_devp->devno);
    class_destroy (rk3288_gpio_devp->class);
    cdev_del(&rk3288_gpio_devp->cdev);
    unregister_chrdev_region(rk3288_gpio_devp->devno, 1u);
    kfree(rk3288_gpio_devp);
    rk3288_gpio_devp = NULL;
#endif
    rk3288_gpio_msg("rk3288_gpio_remove\n");
    return 0;   
} 


static struct platform_driver rk3288_gpio_driver = {
	.probe		= rk3288_gpio_probe,
	.remove		= rk3288_gpio_remove,
	.driver		= {
		.name	= "rockchip_rk3288_gpio",
		.owner	= THIS_MODULE,
		.of_match_table = of_match_ptr(rk3288_gpio_match),
	}
};


static int __init rk3288_gpio_init(void)
{
    int res;

    rk3288_gpio_msg("rk3288_gpio_init\n");

    res = platform_driver_register(&rk3288_gpio_driver);
    printk("res = %d\n",res);
    if(res){
        rk3288_gpio_msg("platform driver register fail\n");
        return -ENODEV;
    }
    return 0;
}

static void __exit rk3288_gpio_exit(void)
{
    rk3288_gpio_msg("rk3288_gpio_exit\n");

	platform_driver_unregister(&rk3288_gpio_driver);
    return;
}

module_init(rk3288_gpio_init);
module_exit(rk3288_gpio_exit);


MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("linux gpio io-contrl driver");
MODULE_AUTHOR("shawn_cool@hotmail.com");
MODULE_ALIAS("platform:rk3288");
