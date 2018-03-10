/*
 * A 4.2 BSD interface using the network buffers.
 * Each allocation cannot exceed CLBYTES - MSIZE. (now 1024 - 128).
 * This is obviously a quick and dirty solution, but its so simple
 * that it shouldn't be much of a burden to change if the
 * other allocation code changes.  The alternative is to mung the entire
 * CHAOS NCP code to deal with buffer chains.
 *
 * The basic idea is that for allocations that fit entirely inside
 * an mbuf structure we do the normal thing, via MGET.  Otherwise
 * we allocate a cluster, leaving room for an mbuf at the front.
 * When freeing we recognize the clusters by their data address having an
 * address that is zero mod MSIZE, which can never happen for an
 * allocation inside an MBUF.  When we pass one of these larger
 * ones to a 4.2 BSD module, we must allocate an additional small
 * mbuf to point at the cluster since the 4.2 code requires a separate
 * mbuf to point at a cluster.
 *
 * A lot of this is because all the chaos code is written assuming
 * that buffers are contiguous rather than chained in small pieces.
 * For maximum size chaos packets, which use 512 bytes, the cluster,
 * being 1024 bytes is used up with an mbuf (128 bytes) and the
 * chaos packet (512 bytes) for a wastage of 1024 - 512 - 128.
 */

#include "mbuf.h"

/*
 * Macro to convert a chaos allocation into the corresponding mbuf pointer.
 */
#define chdtom(d) \
		((int)(d) & (MSIZE - 1) ? dtom(d) : (struct mbuf *)((int)(d)&~CLOFSET))

char *
ch_alloc(size, cantwait)
int size;
{
	struct mbuf *m;
	int offset;

	if (size > MLEN) {
		if (size > CLBYTES - MSIZE)
			panic("ch_alloc too big");
		MCLALLOC(m,1);
		offset = MSIZE;
	} else {
		MGET(m, !cantwait, MT_DATA);
		offset = MMINOFF;
	}
	if (m == 0)
		return 0;
	m->m_off = offset;
	m->m_len = size;
	m->m_next = 0;
	return mtod(m, char *);
}

void
ch_free(p)
char *p;
{
	struct mbuf *m = chdtom(p);
	struct mbuf *n;

	if (m->m_off < MSIZE) {
		MFREE(m, n);
	} else {
	        MCLFREE(m);
	}
}

int
ch_size(p)
char *p;
{
	return chdtom(p)->m_len;
}
/* ARGSUSED */
ch_badaddr(p)
char *p;
{
	return 0;
}
void ch_bufalloc(){}
void ch_buffree(){}
