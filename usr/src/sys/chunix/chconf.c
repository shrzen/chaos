#include "../chunix/chconf.h"
#include "../chunix/chsys.h"
#include "chch.h"
#include <chaos/chaos.h>
#include "../h/param.h"
#include "../h/systm.h"
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
 * For each interface type we must define an array of
 * tranceiver structures and one of local host numbers
 */

#ifdef NDR11C
short dr11chosts[NDR11C] = {
/* the local hosts go here */
};
#endif NDR11C



/*
 * Reset all devices
 */
chreset()
{
	register struct chroute *r;

	for (r = Chroutetab; r < &Chroutetab[CHNSUBNET]; r++)
		if (r->rt_cost == 0)
			r->rt_cost = CHHCOST;
#ifdef NDR11C
	chdrinit();
#endif NDR11C
#ifdef NCHCH
	chchinit();
#endif NCHCH
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
