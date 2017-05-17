#ifndef __RK3288_GPIO_H_
#define __RK3288_GPIO_H_

#define 	AW2013_NAME             "RK3288-GPIO"
#define 	REGISTER_CDEV
#define 	DEVICE_NAME             "rk3288-gpio"

#define     SET_GPIO_TO_INPUT       0
#define     SET_GPIO_TO_OUTPUT      1
#define     REQUEST_GPIO_SUM    11
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
#endif
