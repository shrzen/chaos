#include "../h/chaos.h"
#include "chsys.h"
#include "chconf.h"
#include "../chncp/chncp.h"

#if defined(linux) && defined(__KERNEL__)
#include "chlinux.h"
#endif

#ifdef BSD42
#include "time.h"
#include "kernel.h"
#endif

#if 0
#include "chip.h"
#endif

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
#if defined(linux) && !defined(__KERNEL__)
int Chhz = 60;		/* This is set correctly at auto-conf time but needs
			 * a non-zero initial value at boot time.
			 */
#endif

/*
 * Reset the NCP and all devices.
 */
void
chreset(void)
{
	register struct chroute *r;

	for (r = Chroutetab; r < &Chroutetab[CHNSUBNET]; r++)
		if (r->rt_cost == 0)
			r->rt_cost = CHHCOST;
#ifdef NCHDR
	chdrinit();
#endif
#ifdef NCHCH
	chchinit();
#endif
#ifdef NCHIL
	chilinit();
#endif
#ifdef NCHETHER
	cheinit();
#endif
/* If we have an internet... allow UNC encapsulation. */
#ifdef INET
#ifdef NCHIP
	chipattach ();
#endif
#endif
	/*
	 * This is necessary to preserve the modularity of the
	 * NCP.
	 */
#ifdef linux
	Chhz = 1000;
#else
	Chhz = hz;
#endif
}

void
chdeinit(void)
{
#if NCHETHER > 0
	chedeinit();
#endif
}

/*
 * Check for interface timeouts
 */
void
chxtime(void)
{
#ifdef NDR11C
	chdrxtime();
#endif
}
