# where the system is built
SYSINCLUDES = /usr/src/local/sys/VIXEN
# if the chaos includes are any where special
INCLUDES =
# where the user programs go
DESTUSERS = /usr/local
# where the servers go - this is carried into chserver
DESTSERVERS = /usr/local/lib/chaos
# where the daemons goes
DESTDAEMONS = /etc
# where the mail spooler goes
DESTMAILER = /usr/lib/mailers
# where the host library lives
HOSTLIB = /usr/lib/libhosts.a

SERVERS = MAIL NAME SEND FILE TELNET UPTIME RTAPE SUPDUP
USERS = chsend chstat chup hostat telnet cftp supdup finger chtime chsymtape \
	chtape
DAEMONS = chserver chinit chmailer chmail
PROGS = $(USERS) $(SERVERS) $(DAEMONS) chmkdev
CFLAGS = -O -I$(INCLUDES)

all: $(PROGS)

chsend:		chsend.o
	$(CC) -o $@ $@.o $(HOSTLIB)

chmkdev:	chmkdev.o
	$(CC) -o $@ $@.o $(HOSTLIB)

chstat:		chstat.o
	$(CC) -o $@ $@.o $(HOSTLIB)

telnet:		telnet.o
	$(CC) -o $@ $@.o $(HOSTLIB)

cftp:		cftp.o
	$(CC) -o $@ $@.o $(HOSTLIB)

supdup:		supdup.o /usr/lib/libtermlib.a
	$(CC) -o $@ $@.o $(HOSTLIB) -ltermlib

finger:		finger.o
	$(CC) -o $@ $@.o $(HOSTLIB)

hostat:		hostat.o
	$(CC) -o $@ $@.o $(HOSTLIB)

chup:		chup.o
	$(CC) -o $@ $@.o $(HOSTLIB)

chtime:		chtime.o
	$(CC) -o $@ $@.o $(HOSTLIB)

chsymtape:	chsymtape.o
	$(CC) -o $@ $@.o $(HOSTLIB)

chtape:	chtape.o record.o
	$(CC) -o $@ $@.o record.o $(HOSTLIB)

chtape.o:	chtape.c record.h RTAPE.h

MAIL:		MAIL.o $(HOSTLIB)
	$(CC) -o $@ $@.o $(HOSTLIB)

NAME:		NAME.o
	$(CC) -o $@ $@.o

SEND:		SEND.o $(HOSTLIB)
	$(CC) -o $@ $@.o $(HOSTLIB)

TELNET:		TELNET.o
	$(CC) -o $@ $@.o

UPTIME:		UPTIME.o
	$(CC) -o $@ $@.o

SUPDUP:		SUPDUP.o
	$(CC) -o $@ $@.o

RTAPE:		RTAPE.o record.o
	$(CC) -o $@ $@.o record.o

RTAPE.o record.o: record.h


$(USERS):	$(HOSTLIB)

chstat.o: chstat.c
	$(CC) -c $(CFLAGS) -I$(SYSINCLUDES) -O chstat.c

FILE: FILE.o glob.o
	$(CC) -o $@ $@.o glob.o

chserver: chserver.o
	$(CC) -o chserver chserver.o -ljobs

chinit: chinit.o
	$(CC) -o chinit chinit.o $(HOSTLIB)

chmailer: chmailer.o
	$(CC) -o chmailer chmailer.o $(HOSTLIB)

chmail: chmail.o
	$(CC) -o chmail chmail.o $(HOSTLIB)

install:	$(PROGS)
	cp $(SERVERS) $(DESTSERVERS)
	cp $(USERS) $(DESTUSERS)
	-rm $(DESTUSERS)/f
	ln $(DESTUSERS)/finger $(DESTUSERS)/f
	-rm $(DESTUSERS)/tn
	ln $(DESTUSERS)/telnet $(DESTUSERS)/tn
	-cd $(DESTDAEMONS); rm -f $(DAEMONS)
	cp $(DAEMONS) $(DESTDAEMONS)
	cp chmail $(DESTMAILER)
	cd $(DESTMAILER); strip chmail; chown network chmail;chmod 4755 chmail
	cd $(DESTDAEMONS); chmod 700 $(DAEMONS); strip $(DAEMONS)
	cd $(DESTDAEMONS); chown network chmailer; chmod 4755 chmailer
	cd $(DESTSERVERS); chmod 700 $(SERVERS); strip $(SERVERS)
	cd $(DESTUSERS); chmod 755 $(USERS); strip $(USERS)
		
clean:
	rm -f $(PROGS) *.o

special: chmkdev
	chmkdev | sh
	chmod 666 /dev/ch*
