all clean:
	@for d in libhosts cmd kernel kernel/chsock; do \
		(cd $$d; make $@); \
	done

.PHONY: TAGS
TAGS:
	find -name "*.[chCH]" -print | etags -

check: all
	-rmmod kernel/chaosnet.ko
	insmod kernel/chaosnet.ko
	cmd/chinit 1 local
	cmd/cheaddr enp0s25 401
	cmd/chtime local
