#include <a.out.h>
#include "hosttab.h"
#include <stdio.h>

struct host_data *host_data;

readhosts()
{
	register int i;
	register struct host_data *h;
	register struct host_entry *hp;
	register struct net_entry *np;
	register char **p;
	struct exec e;

	if ((i = open(HOSTTAB, 0)) < 0 ||
	    read(i, (char *)&e, sizeof(e)) != sizeof(e) ||
	    (host_data = h = (struct host_data *)malloc(e.a_data)) == NULL ||
	    read(i, (char *)host_data, e.a_data) != e.a_data) {
		fprintf(stderr, "readhosts: can't read host table: %s\n",
			HOSTTAB);
		exit(1);
	}
	close(i);
#define reloc(x, caste)	(x = caste((char *)h + (int)(x)))
	reloc(h->ht_nets, (struct net_entry *));
	reloc(h->ht_hosts, (struct host_entry *));
	reloc(h->ht_me, (struct host_entry *));
	for (np = h->ht_nets, i = 0; i < h->ht_nsize; np++, i++)
		reloc(np->net_name, (char *));
	for (hp = h->ht_hosts, i = 0; i < h->ht_hsize; hp++, i++) {
		reloc(hp->host_name, (char *));
		reloc(hp->host_system, (char *));
		reloc(hp->host_machine, (char *));
		reloc(hp->host_address, (struct net_address *));
		if (hp->host_nicnames) {
			reloc(hp->host_nicnames, (char **));
			for (p = hp->host_nicnames; *p; p++)
				reloc(*p, (char *));
		}
	}
}
