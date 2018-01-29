all: hosts2 libhosts.a chaosnames

hosts2: hosts2.o
libhosts.a: libhosts.a(hostlib.o readhosts.o)
chaosnames: chaosnames.o libhosts.a

clean:
	rm -f *.o
	rm -f libhosts.a
	rm -f chaosnames
	rm -f hosts2
