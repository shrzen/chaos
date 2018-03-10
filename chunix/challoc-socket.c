#include <stdlib.h>
#include <string.h>

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
	int alloc_size = data_size;

	tracef(TRACE_LOW, "ch_alloc(size=%d)", data_size);

	pkt = malloc(alloc_size);
	if (pkt == 0)
		return NULL;

	memset((char *)pkt, 0, alloc_size);

	return pkt;
}

void
ch_free(char *pkt)
{
	free((char *)pkt);
}

int
ch_size(char *p)
{
	assert("unimplemented");
}

int
ch_badaddr(char *p)
{
	return 0;
}

void ch_bufalloc(void)
{
	/* Do nothing. */
}

void ch_buffree(void)
{
	/* Do nothing. */
}
