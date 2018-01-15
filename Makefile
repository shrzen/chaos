all clean:
	@for d in libhosts cmd kernel/chlinux kernel/chsocket; do \
		(cd $$d; make $@); \
	done

.PHONY: TAGS
TAGS:
	find -name "*.[chCH]" -print | etags -

check: all
	-rmmod kernel/chlinux/chaosnet.ko
	insmod kernel/chlinux/chaosnet.ko
	cmd/chinit 1 local
	cmd/cheaddr enp0s25 401
	cmd/chtime local
