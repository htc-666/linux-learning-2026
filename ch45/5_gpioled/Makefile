KERNELDIR := /home/book/linux/alientek_kernel
CURRENT_PATH := $(shell pwd)

obj-m := gpioled.o

build:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) modules

clean:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) clean