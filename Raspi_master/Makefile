# Makefile
obj-m += master_i2c.o

KDIR := /lib/modules/$(shell uname -r)/build

all: module device_tree

module:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean  
	rm -rf *.dtbo 

device_tree:
	dtc -@ -I dts -O dtb -o master_i2c.dtbo dts/i2c_dt_overlay.dts

load:
	@echo "Loading Device Tree overlay..."
	sudo dtoverlay -v  ./master_i2c.dtbo
	@echo "Loading kernel module..."
	sudo insmod master_i2c.ko

unload:
	@echo "Unloading kernel module..."
	sudo rmmod master_i2c

	@echo "Removing Device Tree overlay..."
	sudo dtoverlay -r master_i2c