LINUX_DIR ?= /usr/local/src/linux

ccflags-y := -I$(src)/h -I$(src) -I$(src)/chlinux
ccflags-y += -DDEBUG_CHAOS=1 -DNCHETHER=2
ccflags-y += -Wno-error=implicit-function-declaration -Wno-error=strict-prototypes -Wno-error=implicit-int
# -Wno-error=incompatible-pointer-types

obj-m := chaosnet.o
chaosnet-objs = \
	chlinux/chlinux.o chunix/chether.o \
	chunix/challoc.o chunix/chconf.o chunix/chtime.o \
	chunix/chaos.o \
	chncp/chstream.o chncp/chclock.o chncp/chrcv.o \
	chncp/chsend.o chncp/chuser.o chncp/chutil.o
