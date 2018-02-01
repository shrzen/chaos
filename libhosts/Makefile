CFLAGS = -I.

all: libhosts.a chaosnames hosttab chaoshosts

libhosts.a: libhosts.a(hostlib.o readhosts.o)
chaosnames: chaosnames.o libhosts.a

hosttab: hosttab.global hosttab.site hosttab.local
	-cat $^ > $@

chaoshosts: chaosnames hosttab
	./chaosnames > $@

prefix ?= /usr/local
exec_prefix ?= $(prefix)
includedir ?= $(prefix)/include
sysconfdir ?= $(prefix)/etc
libdir ?= $(exec_prefix)/lib

install:
	cp libhosts.a $(DESTDIR)$(libdir)/libhosts.a
	cp hosttab.h $(DESTDIR)$(includedir)/hosttab.h
	cp hosttab $(DESTDIR)$(sysconfdir)/hosttab
	cp chaoshosts $(DESTDIR)$(sysconfdir)/chaoshosts

clean:
	rm -f *.o
	rm -f libhosts.a
	rm -f chaosnames
	rm -f hosttab chaoshosts
