#
# Makefile for the Linux chaosnet driver
#

.PHONY: dummy

KDIR=/lib/modules/4.9.0-4-amd64/build/

all:
	make -C $(KDIR) M=$(PWD) modules 

ccflags-y := -I$(PWD)/h -I$(PWD)/chncp -I$(PWD)/chunix

obj-m := chaosnet.o
chaosnet-objs = \
	chunix/challoc.o chunix/chconf.o chunix/chtime.o \
	chunix/chaos.o chunix/chlinux.o chunix/chether.o \
	chncp/chstream.o chncp/chclock.o chncp/chrcv.o \
	chncp/chsend.o chncp/chuser.o chncp/chutil.o

#	chunix/chtty.o
#	chncp/chch11.o chncp/chil.o chncp/chstream.o chncp/if_chip.o
#	chncp/chclock.o chncp/chrcv.o chncp/chuser.o
#	chncp/chdr11c.o chncp/chsend.o chncp/chutil.o

clean:
	make -C $(KDIR) M=$(PWD) modules 
