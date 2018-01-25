CFLAGS += -I.

all: chaosd server listen tstclient tsttime

chaosd: chaosd.o transport.o node.o log.o signal.o misc.o
	$(CC) $(CFLAGS) -o $@ $^

listen: listen.o misc.o
	$(CC) $(CFLAGS) -o $@ $^

CHAOSNET_OBJS = ../chunix/challoc.o ../chunix/chconf.o ../chunix/chtime.o \
	../chunix/chaos.o \
	../chncp/chstream.o ../chncp/chclock.o ../chncp/chrcv.o \
	../chncp/chsend.o ../chncp/chuser.o ../chncp/chutil.o

server: server.o testpackets.o log.o signal.o misc.o $(CHAOSNET_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

tstclient: tstclient.o misc.o
	$(CC) $(CFLAGS) -o $@ $^

tsttime: tsttime.c misc.o
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f *.o chaosd listen server tstclient tsttime
