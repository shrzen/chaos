all clean:
	@for d in libhosts cmd chlinux; do \
		(cd $$d; make $@); \
	done

check:
	-mknod /dev/chaos c 80 1
	-mknod /dev/churfc c 80 2
	-rmmod chaosnet
	dmesg -C
	insmod chlinux/chaosnet.ko
	dmesg -c
	cmd/chinit 1 local
	dmesg -c
	cmd/chup local
	dmesg -c

.PHONY: TAGS
TAGS:
	find -name "*.[chCH]" -print | etags -
