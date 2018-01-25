all clean:
	@for d in libhosts cmd chlinux chsocket; do \
		(cd $$d; make $@); \
	done

check: all
	-rmmod chlinux/chaosnet.ko
	insmod chlinux/chaosnet.ko
	cmd/chinit 1 local
	cmd/cheaddr enp0s25 401
	cmd/chtime local

.PHONY: TAGS
TAGS:
	find -name "*.[chCH]" -print | etags -
