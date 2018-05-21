KDIR = /home/mikhail/SRZ/Altera_SoC_ADC/buildroot-2017.02.10/output/build/linux-rel_socfpga-4.15_18.05.01_pr
PWD = $(shell pwd)
ARCH = arm
PATH := $(PATH):/home/mikhail/intelFPGA/17.1/embedded/ds-5/sw/gcc/bin
CROSS_COMPILE = arm-linux-gnueabihf-

TARGET = spi-srz
obj-m := $(TARGET).o

all:
	$(MAKE) -C $(KDIR) M=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) modules

	

clean:
	@rm -f *.o .*.cmd .*.flags *.mod.c *.order
	@rm -f .*.*.cmd *.symvers *~ *.*~ TODO.*
	@rm -fR .tmp*
	@rm -rf .tmp_versions
