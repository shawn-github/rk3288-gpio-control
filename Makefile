KERN_DIR = /home/shawn/Work/RK3288_KitKat/kernel

all:
	make -j32 -C $(KERN_DIR) M=`pwd` modules 

clean:
	make -C $(KERN_DIR) M=`pwd` modules clean



obj-$(CONFIG_RK3288_GPIO)		+= rk3288_gpio.o
#obj-$(CONFIG_RK3288_GPIO)		+= rk3288_keys.o

#obj-$(CONFIG_RK3288_GPIO)		+= game_gpio.o
#game_gpio-objs = rk3288_gpio.o rk3288_keys.o

