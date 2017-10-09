# description file for -lhosts library

DESTDIR = /usr/lib
SUBS = hostlib.o readhosts.o
CFLAGS = -O

all: libhosts.a hosttab

libhosts.a: $(SUBS)
	ar uv libhosts.a $(SUBS)
	ranlib libhosts.a

$(SUBS): hosttab.h

hosts2: hosts2.c hosttab.h
	$(CC) $(CFLAGS) -o hosts2 hosts2.c

hosttab: hosttab.c  hosttab.h
	$(CC) -c hosttab.c
	mv hosttab.o hosttab
	strip hosttab

hosttab.c: hosts hosts2 whoami
	hosts2 `cat whoami`

hosts: hosts.global hosts.local
	cat hosts.global hosts.local > hosts

install: all
	cp libhosts.a $(DESTDIR)/libhosts.a
	ranlib $(DESTDIR)/libhosts.a
	chmod 644 $(DESTDIR)/libhosts.a
	cp hosttab /etc/hosttab
	chmod 644 /etc/hosttab
	cp hosttab.h /usr/include
	chmod 644 /usr/include/hosttab.h
clean:
	rm -f *.o  hosts2 hosttab.c hosts libhosts.a hosttab
