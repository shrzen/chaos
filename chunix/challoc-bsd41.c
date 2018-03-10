/*
 * This allocator is for non 4.2 BSD systems and uses disk buffers for space.
 */

#include "../chncp/chncp.h"
#include "../h/buf.h"

#define BUFPRI PRIBIO		/* Sleep priority for buffers */
#define	b_bits		b_blkno	/* Bit map for chaos bufs in buffer (0=free) */
#define b_list		b_resid	/* Index in chsize that buffer belongs to */
#define b_nfree	b_bcount	/* Number of free chaos buffers in buffer */

#ifdef vax
#define CHMAXBUF 25
#define CHNMINPKT 2
#define CHNMAXPKT 36

#ifdef BSD42
#define bpfromch(x) (&buf[((x) - buffers) / MAXBSIZE ])
#define CHBLKSIZE 1024
#else
#define bpfromch(x) (&buf[((x) - buffers) / BSIZE ])
#define CHBLKSIZE BSIZE
#endif

#else				/* i.e. not a vax system but a PDP11 */
#define CHBLKSIZE BSIZE
#define CHMAXBUF 12
#define CHNMINPKT 1
#define CHNMAXPKT 9

#ifdef	UCB_BUFOUT
extern char abuffers[NABUF][BSIZE];
#define bpfromch(x) (&abuf[((x) - abuffers) / BSIZE ])
#else
extern char buffers[NBUF][(BSIZE+BSLOP)];
#define bpfromch(x) (&buf[((x) - buffers) / (BSIZE+BSLOP) ])
#endif

#endif

/*
 * Structure for keeping track of various sizes of chaos buffers.
 * Initialized values should be parameterized.
 */
struct chsize {
	short ch_bufsize;	/* Size of packets to allocate from buffer */
	short ch_mxbufs;	/* Maximum buffers to allocate to this size */
	short ch_bufcount;	/* The count of buffers already allocated */
	short ch_buffree;	/* Number of free buffers on this list now */
	struct buf *ch_bufptr;	/* Pointer to buffers of this size */
} Chsizes[] = {
#define CHMINPKT 32
	{ CHMINPKT,	CHNMINPKT, },
	{ 128,	(CHNCONNS + 16)/(CHBLKSIZE/128), },
	{ 512,	CHNMAXPKT, },
};

#define NSIZES (sizeof(Chsizes)/sizeof(Chsizes[0]))

int Chbufwait;		/* Someone's waiting for buffers */
struct buf *Chbuflist;	/* Unassigned buffers */
int	Chnbufs;

/*
 * Allocate a chunk at least "size" large,
 * set flag means called from interrupt level - don't hang waiting for buffers
 * just return NULL
 */
char *
ch_alloc(size, flag)
{
	struct chsize *sp;
	struct buf *bp;
	int j;
	long bit;
	int opl;

	opl = splimp();
again:
	for (sp = Chsizes; sp < &Chsizes[NSIZES]; sp++) {
		if (sp->ch_bufsize < size)
			continue;
		if (sp->ch_buffree == 0) {
			if (sp->ch_bufcount == sp->ch_mxbufs ||
			    (bp = Chbuflist) == NULL)
				continue;
			Chbuflist = bp->av_forw;
			bp->av_forw = sp->ch_bufptr;
			sp->ch_bufptr = bp;
			bp->b_nfree = j = CHBLKSIZE / sp->ch_bufsize;
			bp->b_bits = 0;
			bp->b_list = sp - Chsizes;
			sp->ch_buffree += j;
			sp->ch_bufcount++;
		} else 
			for (bp = sp->ch_bufptr;; bp = bp->av_forw)
				if (bp == NULL)
					panic("buffer lost somewhere");
				else if (bp->b_nfree != 0)
					break;
		/* Here bp points to a buffer to allocate */
		for (bit = 1L, j = 0; ; bit <<= 1, j++)
			if (!(bit & bp->b_bits))
				break;
		bp->b_bits |= bit;
		bp->b_nfree--;
		sp->ch_buffree--;
		debug(DALLOC,printf("Alloc: size=%d,adr = %x\n", size, bp->b_un.b_addr+(j * sp->ch_bufsize)));
		splx(opl);
		return (bp->b_un.b_addr+(j * sp->ch_bufsize));
	}
	if (!flag) {
		Chbufwait++;
		sleep((caddr_t)&Chbufwait, BUFPRI);
		goto again;
	}
	debug(DALLOC|DABNOR,printf("Alloc: size=%d, failed\n", size));
	splx(opl);
	return((caddr_t)0);
}
/*
 * Free the previously allocated storage at "p"
 */
void
ch_free(p)
char *p;
{
	struct buf *bp;
	struct chsize *sp;
	int opl;
	long bit;

	bp = bpfromch(p);
	sp = &Chsizes[bp->b_list];
	debug(DALLOC,printf("Free: addr=%x\n", p));
	bit = 1L << ((p - bp->b_un.b_addr) / sp->ch_bufsize);
	if (!(bp->b_bits & bit)) {
		printf("Free: buffer %x already freed\n", p);
		panic("Chaos buffer already freed");
	}
	bp->b_nfree++;
	bp->b_bits &= ~bit;
	sp->ch_buffree++;
	opl = splimp();
	if (Chbufwait) {
		wakeup((caddr_t)&Chbufwait);
		Chbufwait = 0;
	}
	splx(opl);
}

#ifdef DEBUG_CHAOS
/*
 * Check that address p is in the range of possible allocated packets
 */
ch_badaddr(p)
char *p;
{
	struct buf *bp;
	struct chsize *sp;
	int opl = splimp();

	for (sp = Chsizes; sp < &Chsizes[NSIZES]; sp++)
		for (bp = sp->ch_bufptr; bp; bp = bp->av_forw)
			if (p >= bp->b_un.b_addr &&
			    p < bp->b_un.b_addr + CHBLKSIZE) {
				splx(opl);
				return(0);
			}
	splx(opl);
	return(1);
}
#endif
/*
 * Return the size of the place pointed at by "p"
 */
int
ch_size(p)
char *p;
{
	struct buf *bp;

	bp = bpfromch(p);
	return (Chsizes[bp->b_list].ch_bufsize);
}
/*
 * Allocate some space when a new connection is created
 */
void
ch_bufalloc()
{
	int cnt;
	struct buf *bp;
	struct buf *geteblk();

	if (sizeof(bp->b_bits) != 4)
		panic("challoc bits");
	if (Chnbufs < 8)
		cnt = 4;
	else
		cnt = 1;
	if ((Chnbufs + cnt) > CHMAXBUF)
		return;
	Chnbufs += cnt;
	for (; cnt > 0; cnt--) {
#ifndef vax
#ifdef UCB_BUFOUT
		if (abfreelist.av_forw == &abfreelist)
#else
		if (bfreelist.av_forw == &bfreelist)
#endif
			break;
#endif
#ifdef BSD42
		bp = geteblk(CHBLKSIZE);
#else
		bp = geteblk();
#endif
		CHLOCK;
		bp->av_forw = Chbuflist;
		Chbuflist = bp;
		CHUNLOCK;
	}
	Chnbufs -= cnt;
}

void
ch_buffree()
{
	int cnt;
	struct buf *bp;

	if (Chnbufs <= 8)
		cnt = 4;
	else
		cnt = 1;
	if (Chnbufs - cnt >= CHMAXBUF)
		return;
	CHLOCK;
	for (; cnt > 0 && (bp = Chbuflist) != NULL; cnt--) {
		Chbuflist = bp->av_forw;
#ifdef	UCB_BUFOUT
		abrelse(bp);
#else
		brelse(bp);
#endif
		Chnbufs--;
	}
	CHUNLOCK;
}
