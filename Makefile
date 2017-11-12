
all clean:
	@for d in cmd kernel libhosts; do \
		(cd $$d; make $@); \
	done
