/*
 *	$Source: /afs/dev.mit.edu/source/src77/bsd-4.3/vax/sys/attic/chunix/RCS/chconf.c,v $
 *	$Author: cfields $
 *	$Locker:  $
 *	$Log: chconf.c,v $
 * Revision 1.1  1994/09/16  11:06:40  cfields
 * Initial revision
 *
 * Revision 1.2  85/09/07  15:33:55  root
 * Removed chipattach call.
 * 
 * Revision 1.1  84/06/12  20:06:03  jis
 * Initial revision
 * 
 */
#ifndef lint
static char *rcsid_chconf_c = "$Header: /afs/dev.mit.edu/source/src77/bsd-4.3/vax/sys/attic/chunix/RCS/chconf.c,v 1.1 1994/09/16 11:06:40 cfields Exp $";
#endif lint
#include "../h/chaos.h"
#include "../chunix/chsys.h"
#include "../chunix/chconf.h"
#include "../chncp/chncp.h"
#include "../h/time.h"
#include "../h/kernel.h"
#include "chip.h"
/*
 * This file contains initializations of configuration dependent data
 * structures and device dependent initialization functions.
 */

/*
 * We must identify ourselves
 */
short Chmyaddr = -1;
char Chmyname[CHSTATNAME] = "Uninitialized";
short chhosts[] = {0};
int Chhz = 60;		/* This is set correctly at auto-conf time but needs
			 * a non-zero initial value at boot time.
			 */

/*
 * Reset the NCP and all devices.
 */
chreset()
{
	register struct chroute *r;

	for (r = Chroutetab; r < &Chroutetab[CHNSUBNET]; r++)
		if (r->rt_cost == 0)
			r->rt_cost = CHHCOST;
#if NCHDR > 0
	chdrinit();
#endif NCHDR
#if NCHCH > 0
	chchinit();
#endif NCHCH
#if NCHIL > 0
	chilinit();
#endif NCHIL
#if NCHETHER > 0
	cheinit();
#endif
/* If we have an internet... allow UNC encapsulation. */
#ifdef INET
#if NCHIP > 0
	chipattach ();
#endif
#endif
	/*
	 * This is necessary to preserve the modularity of the
	 * NCP.
	 */
	Chhz = hz;
}
/*
 * Check for interface timeouts
 */
chxtime()
{
#ifdef NDR11C
	chdrxtime();
#endif NDR11C
}
