# Global defines

# For running under Linux
JOBS =
DBM =
DEFINES = -DSYSLOG  -DCHAOS -DZEPHYR -DTERMIOS
MAILDAEMON =
MAILER = chaosmail

# For running under 4.2 BSD:
#JOBS =
#DBM =
# no more XWIND - gdt 6/10/89
#DEFINES = -DBSD42 -DSYSLOG  -DCHAOS -DZEPHYR
#DEFINES = -DBSD42 -DCHAOS 
MAILDAEMON =
MAILER = chaosmail

# For running under 4.1 BSD:
# JOBS = -ljobs
# DBM = -ldbm
# DEFINES =
# MAILDAEMON = chmailer
# MAILER = chmail

ROOT =
# where the system is built
#SYSINCLUDES = /sys/ALLSPICE
SYSINCLUDES = .

# if the chaos includes are any where special
#INCLUDES = /usr/include/chaos
INCLUDES = ../h

#
DESTBASE = /usr/local/chaos

# where the user programs go
DESTUSERS = $(ROOT)$(DESTBASE)/bin

# where the servers go - this is carried into chserver
DESTSERVERS = $(ROOT)$(DESTBASE)/sbin

# where the daemons goes
DESTDAEMONS = $(ROOT)$(DESTBASE)/sbin

# where the mail spooler goes
DESTMAILER = $(ROOT)$(DESTBASE)/sbin

# where the host library lives
#HOSTLIB = /usr/lib/libhosts.a
HOSTLIB = ../libhosts/libhosts.a
HOSTINC = ../libhosts

# where the file server login file lives
FILEUTMP = /etc/chfile.utmp

# Major device number of CHAOS device driver what a bunch of crap
CHRMAJOR = 80

#SERVERS = NAME SEND FILE TELNET RTAPE SUPDUP SPICE TTYLINK RAMTEK
SERVERS = NAME SEND FILE RTAPE SPICE RAMTEK \
	  LGP MAIL SMTP NETLOAD UU MINI

# Servers that want to only read from the network
RSERVERS = SEND RAMTEK LGP

# Servers that want to only write to the network
WSERVERS = NAME NETLOAD

# USERS = chread chosend chstat chup hostat chtelnet cftp chsupdup chfinger chtime \
# 	chtape prtable conn chaccept chwrite \
# 	chsend reply what newsends chrut
USERS = chread chosend chup hostat chtelnet cftp chsupdup chfinger chtime \
	chtape conn chaccept chwrite \
	chsend reply what newsends chrut

DAEMONS = chserver chinit cheaddr $(MAILDAEMON) chconnect

PROGS = $(USERS) $(SERVERS) $(DAEMONS) $(MAILER) chmkdev
#OPT = -O
OPT = -g
CFLAGS = $(OPT) -I$(INCLUDES) -I$(HOSTINC) -DDESTSERVERS=\"$(DESTSERVERS)\" \
	 -DFILEUTMP=\"$(FILEUTMP)\" -DDESTUSERS=\"$(DESTUSERS)\" \
	 -DCHRMAJOR=$(CHRMAJOR) $(DEFINES)

all: $(PROGS)

# chreject is a link to chaccept
chaccept: chaccept.o
	$(CC) -o $@ $@.o chlib.o

chread: chread.o
	$(CC) -o $@ $@.o chlib.o $(HOSTLIB)

chosend: chosend.o
	$(CC) -o $@ $@.o chlib.o $(HOSTLIB)

chmkdev:	chmkdev.o
	$(CC) -o $@ $@.o chlib.o $(HOSTLIB)

chstat:		chstat.o
	$(CC) -o $@ $@.o chlib.o $(HOSTLIB)

chwrite: chwrite.o
	$(CC) -o $@ $@.o chlib.o $(HOSTLIB)

prtable:	prtable.o
	$(CC) -o $@ $@.o chlib.o $(HOSTLIB)

conn:		conn.o
	$(CC) -o $@ $@.o chlib.o $(HOSTLIB)

chtelnet:	chtelnet.o
	$(CC) -o $@ $@.o chlib.o $(HOSTLIB)

cftp:		cftp.o
	$(CC) -o $@ $@.o chlib.o $(HOSTLIB)

chsupdup:	chsupdup.o
	$(CC) -o $@ $@.o chlib.o $(HOSTLIB) -lcurses -ltinfo

chfinger:	chfinger.o
	$(CC) -o $@ $@.o chlib.o $(HOSTLIB)

hostat:		hostat.o
	$(CC) -o $@ $@.o chlib.o $(HOSTLIB)

chup:		chup.o
	$(CC) -o $@ $@.o chlib.o $(HOSTLIB)

chtime:		chtime.o
	$(CC) -o $@ $@.o chlib.o $(HOSTLIB)

chrut:		chrut.o
	$(CC) -o $@ $@.o chlib.o $(HOSTLIB)

chtape:	chtape.o record.o
	$(CC) -o $@ $@.o record.o chlib.o $(HOSTLIB)

chtape.o:	chtape.c record.h RTAPE.h

MAIL:		MAIL.o
	$(CC) -o $@ $@.o chlib.o $(HOSTLIB) $(DBM)
#	$(CC) -o $@ $@.o chlib.o $(HOSTLIB) $(DBM) TCP/netsubrs.o

NAME:		NAME.o
	$(CC) -o $@ $@.o chlib.o

NETLOAD:	NETLOAD.o
	$(CC) -o $@ $@.o chlib.o $(HOSTLIB)

SEND:		SEND.o $(HOSTLIB)
	$(CC) -o $@ $@.o chlib.o $(HOSTLIB)
#	$(CC) -o $@ $@.o chlib.o $(HOSTLIB) -lzephyr -lkrb -lcom_err

SMTP:		SMTP.o $(HOSTLIB)
	$(CC) -o $@ $@.o chlib.o $(HOSTLIB)

TELNET:		TELNET.o
	$(CC) -o $@ $@.o chlib.o $(HOSTLIB)

SUPDUP:		SUPDUP.o
	$(CC) -o $@ $@.o chlib.o $(HOSTLIB)

TTYLINK:	TTYLINK.o
	$(CC) -o $@ $@.o chlib.o $(HOSTLIB)

RTAPE:		RTAPE.o record.o
	$(CC) -o $@ $@.o record.o chlib.o

RTAPE.o record.o: record.h

SPICE:	SPICE.o
	$(CC) -o $@ $@.o chlib.o

RAMTEK:	RAMTEK.o
	$(CC) -o $@ $@.o chlib.o

LGP:	LGP.o
	$(CC) -o $@ $@.o chlib.o

UU:	UU.o
	$(CC) -o $@ $@.o chlib.o

$(USERS): chlib.o $(HOSTLIB)

$(SERVERS): chlib.o

chstat.o: chstat.c
	$(CC) -c $(CFLAGS) -I$(SYSINCLUDES) -O chstat.c

prtable.o: prtable.c
	$(CC) -c $(CFLAGS) -I$(SYSINCLUDES) -O prtable.c

FILE.o glob.o: FILE.h

FILE: FILE.o glob.o chlib.o
	$(CC) -o $@ $@.o glob.o chlib.o $(HOSTLIB) -lcrypt
# -lcompat -lcrypt

MINI: MINI.o

chserver: chserver.o chlib.o
	$(CC) -o chserver chserver.o chlib.o $(HOSTLIB) $(JOBS)

chinit: chinit.o
	$(CC) -o chinit chinit.o $(HOSTLIB)

cheaddr: cheaddr.o
	$(CC) -o cheaddr cheaddr.o

chmailer: chmailer.o
	$(CC) -o chmailer chmailer.o chlib.o $(HOSTLIB)

chmail: chmail.o
	$(CC) -o chmail chmail.o $(HOSTLIB)

chaosmail: chaosmail.o
	$(CC) -o chaosmail chaosmail.o chlib.o $(HOSTLIB)

chconnect: chconnect.o
	$(CC) -o chconnect chconnect.o chlib.o $(HOSTLIB)


chsend:	chsend.o sendsys.o chlib.o 
	cc -o chsend chsend.o sendsys.o chlib.o $(HOSTLIB)

reply:	reply.o sendsys.o sendtxt.o chlib.o 
	cc -o reply reply.o sendsys.o sendtxt.o chlib.o $(HOSTLIB)

what:		what.o sendtxt.o
		cc -o what what.o sendtxt.o

webster:	webster.o
		cc -o $@ $@.o $(HOSTLIB)

newsends:	newsends.o
		cc -o newsends newsends.o


install:	$(PROGS)
	-for i in $(SERVERS); do install -c -s -m 775 $$i $(DESTSERVERS)/$$i; done
	-for i in  $(USERS); do install -c -s -m 755 $$i $(DESTUSERS)/$$i; done
#	-rm $(DESTUSERS)/tn
#	ln $(DESTUSERS)/chtelnet $(DESTUSERS)/chtn
#	ln $(DESTUSERS)/chfinger $(DESTUSERS)/f
	-cd $(DESTDAEMONS); rm -f $(DAEMONS)
	-for i in $(DAEMONS); do install -c -s -o root -m 755 $$i $(DESTDAEMONS)/$$i; done
#	install -s -m 755 -o network $(MAILER) $(DESTMAILER)/$(MAILER)
	cd $(DESTSERVERS); chmod 775 $(SERVERS); chmod 755 $(RSERVERS); chmod 735 $(WSERVERS); strip $(SERVERS)


clean:
	rm -f $(PROGS) *.o

special: chmkdev
	chmkdev | sh
	chmod 666 /dev/ch*
