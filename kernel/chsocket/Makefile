#---------- figure out what we are runnign on ---------

OS_NAME = $(shell uname)
MACH_NAME = $(shell uname -m)

ifeq ($(OS_NAME), Darwin)
OS = OSX
LIBS = 
endif

ifeq ($(OS_NAME), Linux)
OS = LINUX
LIBS =
endif

ifeq ($(OS_NAME), NetBSD)
OS = BSD
LIBS= -lcompat -lcrypt
endif

#----------- code ------------

CHAOSD_OBJ = chaosd.o transport.o node.o log.o signal.o misc.o

CHAOS_OBJS = ../chunix/challoc.o ../chunix/chconf.o ../chunix/chtime.o \
	../chunix/chaos.o \
	../chncp/chstream.o ../chncp/chclock.o ../chncp/chrcv.o \
	../chncp/chsend.o ../chncp/chuser.o ../chncp/chutil.o

SERVER_OBJ = server.o testpackets.o log.o signal.o misc.o $(CHAOS_OBJS)

CFLAGS = -O2 -fno-builtin-log -I.

all: chaosd server listen tstclient tsttime

chaosd: $(CHAOSD_OBJ)
	$(CC) $(CFLAGS) -o chaosd $(CHAOSD_OBJ)

listen: listen.c
	$(CC) $(CFLAGS) -o listen listen.c

server: $(SERVER_OBJ)
	$(CC) $(CFLAGS) -o server $(SERVER_OBJ)

tstclient: tstclient.c
	$(CC) $(CFLAGS) -o tstclient tstclient.c misc.c

tsttime: tsttime.c
	$(CC) $(CFLAGS) -o tsttime tsttime.c misc.c

clean:
	rm -f *.o chaosd listen server
