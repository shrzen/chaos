/*
 *	$Source: /afs/dev.mit.edu/source/src77/bsd-4.3/vax/sys/attic/chunix/RCS/chtime.c,v $
 *	$Author: cfields $
 *	$Locker:  $
 *	$Log: chtime.c,v $
 * Revision 1.1  1994/09/16  11:06:38  cfields
 * Initial revision
 *
 * Revision 1.1  84/06/12  20:06:06  jis
 * Initial revision
 * 
 */
#ifndef lint
static char *rcsid_chtime_c = "$Header: /afs/dev.mit.edu/source/src77/bsd-4.3/vax/sys/attic/chunix/RCS/chtime.c,v 1.1 1994/09/16 11:06:38 cfields Exp $";
#endif lint
#include "../h/chaos.h"
#include "../h/param.h"
#include "../h/systm.h"
#include "../h/time.h"
#include "../h/kernel.h"
/*
 * Return the time according to the chaos TIME protocol, in a long.
 * No byte shuffling need be done here, just time conversion.
 */
ch_time(tp)
register long *tp;
{
#ifdef BSD42
	*tp = time.tv_sec;
#else
	*tp = time;
#endif
	*tp += 60L*60*24*((1970-1900)*365L + 1970/4 - 1900/4);
}
ch_uptime(tp)
register long *tp;
{
#ifdef BSD42
	*tp = (time.tv_sec - boottime.tv_sec) * 60L;
#else
	*tp = (time - bootime) * 60L;
#endif
}
