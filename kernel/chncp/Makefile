# Makefile for the Linux chaosnet driver
#

.PHONY: dummy

ifndef TOPDIR
TOPDIR=/usr/src/linux
endif

OBJS = chuser.o chstream.o chrcv.o chsend.o chclock.o chutil.o 

all:	$(OBJS)

ifndef HOSTCC
#
#	Running outside the kernel build.
#
CC	= gcc
HOSTCC	= gcc

#CINCLUDE = -I../h -I/usr/include/linux
#GCCFLAGS = -Wall -Wstrict-prototypes -fomit-frame-pointer -pipe -m486

CINCLUDE = -I../h -I/usr/src/linux/include
OPTFLAGS = -O2 -g
GCCFLAGS = -fomit-frame-pointer -pipe -m486

CFLAGS	= $(OPTFLAGS) -D__KERNEL__ -DMODULE $(CINCLUDE) $(GCCFLAGS)

else
include $(TOPDIR)/Rules.make
endif

clean:
	rm -f core core.* *.o *.a

dep:
	$(CPP) -M *.c > .depend

#
# include a dependency file if one exists
#
ifeq (.depend,$(wildcard .depend))
include .depend
endif
