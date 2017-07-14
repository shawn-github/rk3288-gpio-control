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
#include "rk3288_keys.h" 


#ifdef REGISTER_INPUT_DEV
static struct input_dev *gamekbd_dev;
static unsigned char gamekey_val[TOUCH_KEY_MAX_CNT] ={
    225,220,221,222,223,224,227,226,
    228,229,230,231,232,233,234,235
};
#endif

#ifdef USE_TIMER_POLL
struct timer_list gamekbd_timer;	
#endif


static void rk3288_keys_report_val(int gpio, int val)
{
    input_report_key(gamekbd_dev, gpio, val);
    input_sync(gamekbd_dev);

    return;
}

#ifdef USE_TIMER_POLL
static void rk3288_keys_timer_func(void)
{
    int val,i,comp_key1,comp_key2;

    comp_key1 = rk3288_keys_get_val(keys_info_arr[COMP_KEY_1]);
    comp_key2 = rk3288_keys_get_val(keys_info_arr[COMP_KEY_2]);
    if(comp_key1 < 0 || comp_key2 < 0)
        rk3288_keys_msg("get complite key's value fail!\n"); 

    if(comp_key1 == comp_key2){
        if(comp_key1 != keys_save_arr[COMP_KEY_1] || comp_key2 != keys_save_arr[COMP_KEY_2]){
            rk3288_keys_msg("complite key changed!\n"); 
            keys_save_arr[COMP_KEY_1] = comp_key1;
            keys_save_arr[COMP_KEY_2] = comp_key2;
            rk3288_keys_report_val(gamekey_val[COMP_KEY_VAL], comp_key1); 
        }
    }

    for(i = 0; i < TOUCH_KEY_MAX_CNT; i++){
        val = rk3288_keys_get_val(keys_info_arr[i]);
        if(val < 0){
            rk3288_keys_msg("get GPIO%d value fail!\n",keys_info_arr[i]); 
            return;
        }else if(val != keys_save_arr[i]){
            keys_save_arr[i] = val;
            rk3288_keys_report_val(gamekey_val[i], val); 
        }
    }
    mod_timer(&gamekbd_timer,jiffies + HZ/5); //set timer HZ
    return;
}
#endif

static const struct of_device_id rk3288_keys_match[] = {
	{ .compatible = "rockchip_rk3288_keys", .data = NULL},
	{},
};
MODULE_DEVICE_TABLE(of, rk3288_keys_match);

static int rk3288_keys_get_val(unsigned gpio)
{
    int res;
    res = __gpio_get_value(gpio);
    if(res < 0){
        rk3288_keys_msg("get GPIO%d value fail!\n",gpio); 
        return res; 
    }

    return res;
}

static int rk3288_keys_cfg_hw(struct platform_device *pdev)
{
    int res; 
    struct device_node *np = pdev->dev.of_node;
    enum of_gpio_flags get_arr[TOUCH_KEY_MAX_CNT]; 

    keys_info_arr[0] = of_get_named_gpio_flags(np, "gpio-in0", 0, &get_arr[0]);
    keys_info_arr[1] = of_get_named_gpio_flags(np, "gpio-in1", 0, &get_arr[1]);
    keys_info_arr[2] = of_get_named_gpio_flags(np, "gpio-in2", 0, &get_arr[2]);
    keys_info_arr[3] = of_get_named_gpio_flags(np, "gpio-in3", 0, &get_arr[3]);
    keys_info_arr[4] = of_get_named_gpio_flags(np, "gpio-in4", 0, &get_arr[4]);
    keys_info_arr[5] = of_get_named_gpio_flags(np, "gpio-in5", 0, &get_arr[5]);
    keys_info_arr[5] = of_get_named_gpio_flags(np, "gpio-in6", 0, &get_arr[6]);
    keys_info_arr[7] = of_get_named_gpio_flags(np, "gpio-in7", 0, &get_arr[7]);
    keys_info_arr[8] = of_get_named_gpio_flags(np, "gpio-in8", 0, &get_arr[8]);
    keys_info_arr[9] = of_get_named_gpio_flags(np, "gpio-in9", 0, &get_arr[9]);
    keys_info_arr[10] = of_get_named_gpio_flags(np, "gpio-in10", 0, &get_arr[10]);
    keys_info_arr[11] = of_get_named_gpio_flags(np, "gpio-in11", 0, &get_arr[11]);
    keys_info_arr[12] = of_get_named_gpio_flags(np, "gpio-in12", 0, &get_arr[12]);
    keys_info_arr[13] = of_get_named_gpio_flags(np, "gpio-in13", 0, &get_arr[13]);
    keys_info_arr[14] = of_get_named_gpio_flags(np, "gpio-in14", 0, &get_arr[14]);

#if 0
    for(res = 0;res < TOUCH_KEY_MAX_CNT; res++){
        rk3288_keys_msg("keys_info_arr[%d] = %d\n",res,keys_info_arr[res]);
    }
#endif

    for(res = 0;res < TOUCH_KEY_MAX_CNT; res++){
        keys_save_arr[res] = rk3288_keys_get_val(keys_info_arr[res]);
        //rk3288_keys_msg("init val:%d\n",keys_save_arr[res]);
    }
    return 0;
}

static int rk3288_keys_probe(struct platform_device *pdev)
{
    int res,error;

#ifdef REGISTER_INPUT_DEV
    gamekbd_dev = input_allocate_device();
    if (!gamekbd_dev) {
        printk(KERN_ERR "alloc memory for input device fail\n");
        return -ENOMEM;
    }
    gamekbd_dev->name = INPUT_DEV_NAME;
    gamekbd_dev->phys = "input/game_key";
    gamekbd_dev->id.bustype = BUS_HOST;
    gamekbd_dev->id.vendor = 0x0002;
    gamekbd_dev->id.product = 0x0002;
    gamekbd_dev->id.version = 0x0200;
    gamekbd_dev->evbit[0] = BIT_MASK(EV_KEY);

    for (res = 0; res < TOUCH_KEY_MAX_CNT; res++)
        set_bit(gamekey_val[res], gamekbd_dev->keybit);

    error = input_register_device(gamekbd_dev);
    if (error) {
        printk("Register %s input device failed", INPUT_DEV_NAME);
        return -ENODEV;
    }
#endif

#ifdef USE_TIMER_POLL
    init_timer(&gamekbd_timer);
    gamekbd_timer.expires = jiffies +2; 
    gamekbd_timer.function = (void*)rk3288_keys_timer_func;
    add_timer(&gamekbd_timer);
#endif


    res = rk3288_keys_cfg_hw(pdev);
    if(res){
        printk(KERN_ERR "RK3288-GPIO: init fail\n");
        return -ENODEV;
    }

    return 0;
}


static int rk3288_keys_remove(struct platform_device *pdev)
{ 
#ifdef USE_TIMER_POLL
	del_timer(&gamekbd_timer);
#endif

    return 0;   
} 


static struct platform_driver rk3288_keys_driver = {
	.probe		= rk3288_keys_probe,
	.remove		= rk3288_keys_remove,
	.driver		= {
		.name	= "rockchip_rk3288_keys",
		.owner	= THIS_MODULE,
		.of_match_table = of_match_ptr(rk3288_keys_match),
	}
};


static int __init rk3288_keys_init(void)
{
    int res;

    res = platform_driver_register(&rk3288_keys_driver);
    if(res){
        rk3288_keys_msg("platform driver register fail\n");
        return -ENODEV;
    }
    return 0;
}

static void __exit rk3288_keys_exit(void)
{
	platform_driver_unregister(&rk3288_keys_driver);
    return;
}

module_init(rk3288_keys_init);
module_exit(rk3288_keys_exit);


MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("linux gpio io-contrl driver");
MODULE_AUTHOR("shawn_cool@hotmail.com");
MODULE_ALIAS("platform:rk3288");
