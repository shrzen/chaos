KDIR=/lib/modules/$(shell uname -r)/build/

ccflags-y := -I$(src)/h -I$(src)/chncp -I$(src)/chunix -I$(src)/chlinux -I$(src)

obj-m := chaosnet.o
chaosnet-objs = \
	chlinux/chlinux.o chunix/chether.o \
	chunix/challoc.o chunix/chconf.o chunix/chtime.o \
	chunix/chaos.o \
	chncp/chstream.o chncp/chclock.o chncp/chrcv.o \
	chncp/chsend.o chncp/chuser.o chncp/chutil.o
