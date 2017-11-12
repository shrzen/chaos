#
# Makefile for the Linux chaosnet driver
#

.PHONY: dummy

TARGET_OS	= linux
USRINCDIR	= /usr/include
MODULEDIR	= /usr/local/chaos/sbin
#KERNELINCLUDE	= /usr/include/linux
#KERNELINCLUDE	= /work2/i386/linux-2.4.2/include
KERNELINCLUDE	= /work2/i386/linux-2.2.18/include

ifndef TOPDIR
TOPDIR=/usr/src/linux
endif

build:
	@echo Compiling modularized chaosnet driver
	@make chaosnet.o
	@echo chaosnet module compiled.

install: chaosnet.o
	cp chaosnet.o $(MODULEDIR)

all:	chaosnet.o

OBJS = \
	chunix/challoc.o chunix/chconf.o chunix/chtime.o \
	chunix/chaos.o chunix/chlinux.o chunix/chether.o \
	chncp/chstream.o chncp/chclock.o chncp/chrcv.o \
	chncp/chsend.o chncp/chuser.o chncp/chutil.o

#	chunix/chtty.o
#	chncp/chch11.o chncp/chil.o chncp/chstream.o chncp/if_chip.o
#	chncp/chclock.o chncp/chrcv.o chncp/chuser.o
#	chncp/chdr11c.o chncp/chsend.o chncp/chutil.o

chaosnet.o: $(OBJS)
	-rm -f chaosnet.o
	$(LD) -r -o chaosnet.o $(OBJS)

ifndef HOSTCC
#
#	Running outside the kernel build.
#
CC	= gcc
HOSTCC	= gcc
CINCLUDE = -I./h -I$(KERNELINCLUDE)
GCCFLAGS = -Wall -Wstrict-prototypes -fomit-frame-pointer -pipe -m486
OPTFLAGS = -O2 -g
CFLAGS	= $(OPTFLAGS) -D__KERNEL__ -DMODULE $(CINCLUDE) $(GCCFLAGS)
else
include $(TOPDIR)/Rules.make
endif

clean:
	rm -f core core.* *.o *.a
	rm -f chncp/*.o chunix/*.o

dep:
	$(CPP) -M *.c > .depend

#
# include a dependency file if one exists
#
ifeq (.depend,$(wildcard .depend))
include .depend
endif
