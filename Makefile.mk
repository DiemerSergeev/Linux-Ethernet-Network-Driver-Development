MODULE_NAME := MyDriver
PWD := $(shell pwd)
KERNEL := $(shell uname -r)

obj-m := $(MODULE_NAME).o
$(MODULE_NAME)-objs += net_drv.o

all: 
	make -C /lib/modules/$(KERNEL)/build M=$(PWD) modules
	
clean:
	make -C /lib/modules/$(KERNEL)/build M=$(PWD) clean
