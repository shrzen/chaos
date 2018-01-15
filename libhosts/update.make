chaostables:
	make -f update.make go
	make -f update.make clean

go:	/etc/hosttab /etc/chaoshosts

/etc/hosttab:		hosts.global hosts.site\
		        hosts.local 
	make -f update.make hosttab
	install -c -m 644 hosttab /etc/hosttab

/etc/chaoshosts: /etc/hosttab
	make -f update.make chaoshosts
	install -c -m 644 chaoshosts /etc/chaoshosts

hosttab: hosts
	cp hosts hosttab

hosttab.c: hosts hosts2
	hosts2 `/bin/hostname | tr a-z A-Z`

hosts:	hosts.global hosts.site \
		hosts.local
	-cat hosts.global hosts.site \
		hosts.local >hosts

chaoshosts: /etc/hosttab chaosnames
	./chaosnames > chaoshosts

clean:
	rm -f hosttab.c hosttab chaoshosts

