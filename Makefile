zram-y	:=	zcomp.o zram_drv.o 

obj-$(CONFIG_ZRAM)	+=	zram.o 

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean