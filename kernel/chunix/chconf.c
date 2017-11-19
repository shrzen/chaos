#if defined(linux) && defined(__KERNEL__)
#include <linux/types.h>
#include <linux/signal.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/proc_fs.h>
#endif

#include "../h/chaos.h"
#include "chsys.h"
#include "chconf.h"
#include "../chncp/chncp.h"

#if defined(linux) && defined(__KERNEL__)
#include "chlinux.h"
#endif

#ifdef linux
//#include <linux/types.h>
//#include <linux/signal.h>
//#include <linux/errno.h>
//#include <linux/sched.h>
//#include <linux/proc_fs.h>
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
#if ams ///---!!!
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
#if NCHDR > 0
	chdrinit();
#endif
#if NCHCH > 0
	chchinit();
#endif
#if NCHIL > 0
	chilinit();
#endif
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
#ifdef linux
	Chhz = 0;
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
