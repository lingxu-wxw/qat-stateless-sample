
#KBUILD_EXTRA_SYMBOLS += $(ICP_ROOT)/quickassist/lookaside/access_layer/src/Module.symvers
#INCLUDES = -I$(src)/base -I$(src)/kernel -I$(src)/../uds -I$(ICP_ROOT)/quickassist/include

zram-y	:=	zcomp.o zram_drv.o 
#	qat/qatCompress.o qat/qat.o

obj-$(CONFIG_ZRAM)	+=	zram.o 

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
