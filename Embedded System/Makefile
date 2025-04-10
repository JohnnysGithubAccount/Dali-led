KERN_SRC:= /home/johnny/kernel/linux
PWD   := $(shell pwd)
obj-m := my_driver.o
all:
	make -C $(KERN_SRC) ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- M=$(PWD) modules
clean:
	make -C $(KERN_SRC) ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- M=$(PWD) clean

