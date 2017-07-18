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
static unsigned char gamekey_val[TOUCH_KEY_MAX_CNT+1] ={
    225,220,221,222,223,224,227,226,
    228,229,230,231,232,233,234,235
};
#endif

#ifdef USE_TIMER_POLL
struct timer_list gamekbd_timer;	
#endif


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

static void rk3288_keys_report_val(int gpio, int val)
{
    input_report_key(gamekbd_dev, gpio, val);
    input_sync(gamekbd_dev);

    return;
}

#ifdef USE_TIMER_POLL
static void rk3288_keys_timer_func(void)
{
    static int flagrightaddlefttoenter = 0;
    static int flagfromright = 0;          
    static int flagfromleft = 0;           
    static int flagfromup = 0 ;
    static int flagfromdown = 0 ;
    static int flagbothupanddown = 0 ;

    unsigned int gpio_key_new_value0 = -1, gpio_key_new_value1=-1,gpio_key_new_value2 =-1;
    unsigned int gpio_key_new_value3 =-1, gpio_key_new_value4=-1,gpio_key_new_value5 =-1;
    unsigned int gpio_key_new_value6 =-1, gpio_key_new_value7=-1,gpio_key_new_value9 =-1;
    unsigned int gpio_key_new_value10=-1,gpio_key_new_value11=-1,gpio_key_new_value12=-1;
    unsigned int gpio_key_new_value13=-1,gpio_key_new_value14=-1,gpio_key_new_value15=-1;

    gpio_key_new_value0 =rk3288_keys_get_val(keys_info_arr[0]);
    gpio_key_new_value1 =rk3288_keys_get_val(keys_info_arr[1]);
    gpio_key_new_value2 =rk3288_keys_get_val(keys_info_arr[2]);
    gpio_key_new_value3 =rk3288_keys_get_val(keys_info_arr[3]);
    gpio_key_new_value4 =rk3288_keys_get_val(keys_info_arr[4]);
    gpio_key_new_value5 =rk3288_keys_get_val(keys_info_arr[5]);
    gpio_key_new_value6 =rk3288_keys_get_val(keys_info_arr[6]);
    gpio_key_new_value7 =rk3288_keys_get_val(keys_info_arr[7]);
    gpio_key_new_value9 =rk3288_keys_get_val(keys_info_arr[8]);
    gpio_key_new_value10=rk3288_keys_get_val(keys_info_arr[9]);
    gpio_key_new_value11=rk3288_keys_get_val(keys_info_arr[10]);
    gpio_key_new_value12=rk3288_keys_get_val(keys_info_arr[11]);
    gpio_key_new_value13=rk3288_keys_get_val(keys_info_arr[12]);
    gpio_key_new_value14=rk3288_keys_get_val(keys_info_arr[13]);
    gpio_key_new_value15=rk3288_keys_get_val(keys_info_arr[14]);

    if(gpio_key_new_value0 == 0){
        rk3288_keys_report_val(gamekey_val[0], 1);
    }else{
        rk3288_keys_report_val(gamekey_val[0], 0);
    }

    if((gpio_key_new_value1 == 0) && (gpio_key_new_value2 == 0)){
        if( 1 == flagfromright ){
            flagfromright = 0 ;            
            rk3288_keys_report_val(gamekey_val[2], 0);
        }
        if( 1 == flagfromleft ){
            flagfromleft = 0 ;
            rk3288_keys_report_val(gamekey_val[1], 0);
        }
        rk3288_keys_report_val(gamekey_val[5], 1);
        flagrightaddlefttoenter = 1 ;  
    }else if((gpio_key_new_value1==0) && (gpio_key_new_value2==1)){  
        if( 1 == flagrightaddlefttoenter){
            flagrightaddlefttoenter = 0;  
            rk3288_keys_report_val(gamekey_val[5], 0);
        }
        flagfromleft = 1 ;             
        rk3288_keys_report_val(gamekey_val[1], 1);
        flagfromright = 0 ;   
        rk3288_keys_report_val(gamekey_val[2], 0);
    }else if( (gpio_key_new_value1==1) && (gpio_key_new_value2==1)){  
        if( 1 == flagrightaddlefttoenter){
            flagrightaddlefttoenter = 0 ;  
            rk3288_keys_report_val(gamekey_val[5], 0);
        }else{
            flagfromleft = 0 ;
            flagfromright = 0 ;            
            rk3288_keys_report_val(gamekey_val[1], 0);
            rk3288_keys_report_val(gamekey_val[2], 0);
        }
    }else if((gpio_key_new_value1==1) && (gpio_key_new_value2==0)){
        if(1 == flagrightaddlefttoenter){
            flagrightaddlefttoenter = 0 ;
            rk3288_keys_report_val(gamekey_val[5], 0);
        }
        flagfromright = 1 ;
        rk3288_keys_report_val(gamekey_val[2], 1);

        flagfromleft = 0 ;
        rk3288_keys_report_val(gamekey_val[1], 0);
    }
    if((gpio_key_new_value3==0) && (gpio_key_new_value4==0)){
        rk3288_keys_report_val(gamekey_val[8], 1);
        flagbothupanddown = 1 ;
    }else if( (gpio_key_new_value3==1) && (gpio_key_new_value4==1) ){
        if(1== flagbothupanddown ){
            rk3288_keys_report_val(gamekey_val[8], 0);
            flagbothupanddown = 0 ;  
        }else{
            flagfromup = 0 ;
            flagfromdown = 0 ;
            rk3288_keys_report_val(gamekey_val[3], 0);

            rk3288_keys_report_val(gamekey_val[4], 0);
        }
    }else if((gpio_key_new_value3==0) && (gpio_key_new_value4==1)){
        if(1 == flagbothupanddown ){
            rk3288_keys_report_val(gamekey_val[8], 0);
            flagbothupanddown = 0 ;
        }
        flagfromup = 1 ;
        rk3288_keys_report_val(gamekey_val[3], 1);

        flagfromdown = 0 ;
        rk3288_keys_report_val(gamekey_val[4], 0);
    }else if( (gpio_key_new_value3==1) && (gpio_key_new_value4==0)){
        if(1 == flagbothupanddown ){
            rk3288_keys_report_val(gamekey_val[8], 0);
            flagbothupanddown = 0 ;
        }
        flagfromup = 0 ;
        rk3288_keys_report_val(gamekey_val[3], 0);
        flagfromdown = 1 ;
        rk3288_keys_report_val(gamekey_val[4], 1);
    }
    if(gpio_key_new_value5==0){
        rk3288_keys_report_val(gamekey_val[5], 1);
    }else{
        rk3288_keys_report_val(gamekey_val[5], 0);
    }
    if(gpio_key_new_value6==0){
        rk3288_keys_report_val(gamekey_val[6], 1);
    }else{
        rk3288_keys_report_val(gamekey_val[6], 0);
    }
    if(gpio_key_new_value7==0){
        rk3288_keys_report_val(gamekey_val[7], 1);
    }
    else{
        rk3288_keys_report_val(gamekey_val[7], 0);
    }
    if(gpio_key_new_value9==0){
        rk3288_keys_report_val(gamekey_val[15], 1);
    }
    else{
        rk3288_keys_report_val(gamekey_val[15], 0);
    }
    if(gpio_key_new_value10==0){
        rk3288_keys_report_val(gamekey_val[9], 1);
    }
    else{
        rk3288_keys_report_val(gamekey_val[9], 0);
    }
    if(gpio_key_new_value11==0){
        rk3288_keys_report_val(gamekey_val[10], 1);
    }
    else{
        rk3288_keys_report_val(gamekey_val[10], 0);
    }
    if(gpio_key_new_value12==0){
        rk3288_keys_report_val(gamekey_val[11], 1);
    }
    else{
        rk3288_keys_report_val(gamekey_val[11], 0);
    }
    if(gpio_key_new_value13==0){
        rk3288_keys_report_val(gamekey_val[12], 1);
    }
    else{
        rk3288_keys_report_val(gamekey_val[12], 0);
    }
    if(gpio_key_new_value14==0){
        rk3288_keys_report_val(gamekey_val[13], 1);
    }else{
        rk3288_keys_report_val(gamekey_val[13], 0);
    }
    if(gpio_key_new_value15==0){
        rk3288_keys_report_val(gamekey_val[14], 1);
    }else{
        rk3288_keys_report_val(gamekey_val[14], 0);
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
    keys_info_arr[6] = of_get_named_gpio_flags(np, "gpio-in6", 0, &get_arr[6]);
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
        rk3288_keys_msg("init val:%d\n",keys_save_arr[res]);
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
