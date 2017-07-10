#ifndef __RK3288_GPIO_H_
#define __RK3288_GPIO_H_

#define 	REGISTER_CDEV
#define 	DEVICE_NAME                "rk3288-gpio"

#define     SET_GPIO_TO_INPUT          0
#define     SET_GPIO_TO_OUTPUT         1
#define     REQUEST_GPIO_SUM           11
#define     rk3288_gpio_msg(...)    do {printk("[RK3288-GPIO] "__VA_ARGS__);} while(0)


unsigned int gpio_info_arr[REQUEST_GPIO_SUM];
#ifdef  REGISTER_CDEV
struct rk3288_gpio_dev {
    struct cdev         cdev;
    dev_t               devno;
    struct class*       class;
};

static struct rk3288_gpio_dev* rk3288_gpio_devp = NULL;

#endif

static int rk3288_gpio_set_io(unsigned gpio, bool io);
static void rk3288_gpio_free_io(unsigned gpio);
static int rk3288_gpio_set_val(unsigned gpio, unsigned int val);
static int rk3288_gpio_get_val(unsigned gpio);
static int rk3288_gpio_cfg_hw(struct platform_device *pdev);
static int rk3288_gpio_open (struct inode *inode, struct file *filp);
static ssize_t rk3288_gpio_read (struct file *filp, const char __user *buf, 
        size_t size, loff_t *ppos);
static ssize_t rk3288_gpio_write (struct file *filp, const char __user *buf,
        size_t size, loff_t *ppos);
#endif
