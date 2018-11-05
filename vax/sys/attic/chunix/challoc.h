/*
 *	$Source: /afs/dev.mit.edu/source/src77/bsd-4.3/vax/sys/attic/chunix/RCS/challoc.h,v $
 *	$Author: cfields $
 *	$Locker:  $
 *	$Log: challoc.h,v $
 * Revision 1.1  1994/09/16  11:06:39  cfields
 * Initial revision
 *
 * Revision 1.1  84/06/12  20:08:52  jis
 * Initial revision
 * 
 */
/*
 * Definitions which are shared between the system dependent storage
 * allocator and other system dependent modules.
 */
#ifdef BSD42
/*
 * Macro to convert a chaos allocation into the corresponding mbuf pointer.
 */
#define chdtom(d) \
		((int)(d) & (MSIZE - 1) ? dtom(d) : (struct mbuf *)((int)(d)&~CLOFSET))
#endif
