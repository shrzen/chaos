#include <stdlib.h>

#include "../h/chaos.h"
#include "chsys.h"
#include "../chunix/chconf.h"
#include "../chunix/challoc.h"
#include "../chncp/chncp.h"

#include "log.h"

char *
ch_alloc(int data_size, int ignore)
{
	struct packet *pkt;
	int alloc_size = sizeof(struct packet) + data_size;

	tracef(TRACE_LOW, "ch_alloc(size=%d)", data_size);

	pkt = (struct packet *)malloc(alloc_size);
	if (pkt == 0)
		return NULL;

	if (1) memset((char *)pkt, 0, alloc_size);

	return pkt;
}

void
ch_free(char *pkt)
{
	free((char *)pkt);
}

