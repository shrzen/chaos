# For running under GNU/Linux
MAILDAEMON =
MAILER = chaosmail

# For running under 4.1 BSD:
# MAILDAEMON = chmailer
# MAILER = chmail

#                        TELNET       SUPDUP       TTYLINK
SERVERS = NAME SEND FILE        RTAPE        SPICE         RAMTEK \
	  LGP MAIL SMTP NETLOAD UU MINI

#                      chstat
#              prtable					chsupdup
USERS = chread chosend        chup hostat chtelnet cftp          chfinger chtime \
	chtape         conn chaccept chwrite \
	chsend reply what newsends chrut

DAEMONS = chserver chinit cheaddr $(MAILDAEMON) chconnect

CFLAGS = -I../h -I../libhosts

all: $(USERS) $(SERVERS) $(DAEMONS) $(MAILER) chmkdev

# chreject is a link to chaccept
chaccept: chaccept.o chlib.o

chread: chread.o chlib.o
	$(CC) -o $@ $^ ../libhosts/libhosts.a

chosend: chosend.o chlib.o
	$(CC) -o $@ $^ ../libhosts/libhosts.a

chmkdev: chmkdev.o

chstat: chstat.o

chwrite: chwrite.o chlib.o
	$(CC) -o $@ $^ ../libhosts/libhosts.a

prtable: prtable.o

conn: conn.o chlib.o
	$(CC) -o $@ $^ ../libhosts/libhosts.a

chtelnet: chtelnet.o chlib.o
	$(CC) -o $@ $^ ../libhosts/libhosts.a

cftp: cftp.o chlib.o
	$(CC) -o $@ $^ ../libhosts/libhosts.a

chsupdup: chsupdup.o chlib.o
	$(CC) -o $@ $^ ../libhosts/libhosts.a -lcurses -ltinfo

chfinger: chfinger.o chlib.o
	$(CC) -o $@ $^ ../libhosts/libhosts.a

hostat: hostat.o chlib.o
	$(CC) -o $@ $^ ../libhosts/libhosts.a

chup: chup.o chlib.o
	$(CC) -o $@ $^ ../libhosts/libhosts.a

chtime: chtime.o chlib.o
	$(CC) -o $@ $^ ../libhosts/libhosts.a

chrut: chrut.o chlib.o
	$(CC) -o $@ $^ ../libhosts/libhosts.a

chtape: chtape.o record.o chlib.o
	$(CC) -o $@ $^ ../libhosts/libhosts.a

MAIL: MAIL.o chlib.o
	$(CC) -o $@ $^ ../libhosts/libhosts.a

NAME: NAME.o

NETLOAD: NETLOAD.o chlib.o
	$(CC) -o $@ $^ ../libhosts/libhosts.a

SEND: SEND.o chlib.o

SMTP: SMTP.o chlib.o
	$(CC) -o $@ $^ ../libhosts/libhosts.a

TELNET: TELNET.o

SUPDUP: SUPDUP.o

TTYLINK: TTYLINK.o

RTAPE: RTAPE.o record.o chlib.o

SPICE: SPICE.o chlib.o

RAMTEK: RAMTEK.o chlib.o

LGP: LGP.o chlib.o

UU: UU.o chlib.o

MINI: MINI.o chlib.o

FILE: FILE.o glob.o chlib.o
	$(CC) -o $@ $^ ../libhosts/libhosts.a -lcrypt

chserver: chserver.o chlib.o
	$(CC) -o $@ $^ ../libhosts/libhosts.a

chinit: chinit.o chlib.o
	$(CC) -o $@ $^ ../libhosts/libhosts.a

cheaddr: cheaddr.o

chmailer: chmailer.o chlib.o
	$(CC) -o $@ $^ ../libhosts/libhosts.a

chmail: chmail.o chlib.o
	$(CC) -o $@ $^ ../libhosts/libhosts.a

chaosmail: chaosmail.o chlib.o
	$(CC) -o $@ $^ ../libhosts/libhosts.a

chconnect: chconnect.o chlib.o
	$(CC) -o $@ $^ ../libhosts/libhosts.a

chsend: chsend.o

reply: reply.o sendsys.o sendtxt.o

what: what.o sendtxt.o

webster: webster.o

newsends: newsends.o

prefix ?= /usr/local
exec_prefix ?= $(prefix)
bindir ?= $(exec_prefix)/bin
sbindir ?= $(exec_prefix)/sbin
libexecdir ?= $(exec_prefix)/libexec

install:
	cp $(USERS) $(DESTDIR)/$(bindir)
	cp $(SERVERS) $(DESTDIR)/$(libexecdir)
	cp $(DAEMONS) $(DESTDIR)/$(sbindir)
	cp $(MAILER) $(DESTDIR)/$(sbindir)
	cp chmkdev $(DESTDIR)/$(sbindir)

clean:
	rm -f *.o
	rm -f $(USERS)
	rm -f $(SERVERS)
	rm -f $(DAEMONS)
	rm -f $(MAILER)
	rm -f chmkdev

