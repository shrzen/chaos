# $Id: Makefile,v 1.2 1999/11/08 15:29:35 brad Exp $

all clean:
	@for d in libhosts cmd kernel; do \
		(cd $$d; make $@); \
	done
