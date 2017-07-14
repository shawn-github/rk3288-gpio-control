#ifndef __RK3288_KEYS_H_
#define __RK3288_KEYS_H_

#define     rk3288_keys_msg(...)    do {printk("[RK3288-KEYS] "__VA_ARGS__);} while(0)

#define     INPUT_DEV_NAME             "rk3288-kbd"
#define     TOUCH_KEY_MAX_CNT          16
#define     COMP_KEY_1                 3
#define     COMP_KEY_2                 4 
#define     COMP_KEY_VAL               8 
#define     REGISTER_INPUT_DEV  
#define     USE_TIMER_POLL      

unsigned int keys_info_arr[TOUCH_KEY_MAX_CNT];
unsigned int keys_save_arr[TOUCH_KEY_MAX_CNT];

static void rk3288_keys_report_val(int gpio, int val);
static void rk3288_keys_timer_func(void);
static int rk3288_keys_get_val(unsigned gpio);
static int rk3288_keys_cfg_hw(struct platform_device *pdev);
#endif
