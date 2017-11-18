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

CHAOSD_OBJ = chaosd.o transport.o node.o log.o signal.o

SERVER_OBJ = server.o testpackets.o log.o signal.o

CFLAGS = -O2 -fno-builtin-log

all: chaosd listen server

chaosd: $(CHAOSD_OBJ)
	$(CC) $(CFLAGS) -o chaosd $(CHAOSD_OBJ)

listen: listen.c
	$(CC) $(CFLAGS) -o listen listen.c

server: $(SERVER_OBJ)
	$(CC) $(CFLAGS) -o server $(SERVER_OBJ)

clean:
	rm -f *.o chaosd listen server
