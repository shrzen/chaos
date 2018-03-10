/* chconf.h --- configuration parameters for NCP
 *
 * This file contains software configuration parameters for the NCP,
 * as well as the definitions for the hardware interfaces in use.
 */

/*
 * Software configuration parameters - system independent
 */

#define CHNCONNS	20	/* Maximum number of connections */
#define CHDRWSIZE	5	/* Default receive window size */
#define CHMINDATA	(32-CHHEADSIZE)
#define CHSHORTTIME	(Chhz>>4)	/* Short time for retransmits */
#define CHNSUBNET	(CHMAXDATA/sizeof(struct rut_data))	/* Number of subnets in routing table */

#define CHHZ 1000		/* Initial tick. */

/*
 * Software configuration parameters - system dependent
 */

#define chroundup(n)	((n) <= (32-CHHEADSIZE) ? (32-CHHEADSIZE) : \
			 (n) <= (128-CHHEADSIZE) ? (128-CHHEADSIZE) : \
			 (512-CHHEADSIZE) )

#define CHBUFTIME	5*Chhz	/* timeout ticks for no buffers (used?) */

/*
 * Hardware configuration parameters
 *
 * Foreach interface type (e.g. xxzzz), you need:

#ifdef NCHXX
#include "chxx.h"
#include <chaos/xxzzz.h>	* for device register definitions and
				   struct xxxxinfo defining software state *
extern short xxxxhosts[];	* array of actual host numbers
				   (initialized in chconf.c) *
				* only needed for interfaces that don't know
				  their own address *
#endif
 */

/*
 * For dr11-c's
 */
#ifdef NCHDR
#include "chdr.h"
#include "../chncp/dr11c.h"
extern short dr11chosts[];	/* local host address per dr11c interface */
#endif

/*
 * For ch11's - if not vax we must specify number of interfaces here.
 */
#ifdef NCHCH
#include "chch.h"
#include "../chncp/ch11.h"
#endif

/*
 * For chil's.
 */
#ifdef NCHIL
#include "chil.h"
#ifdef BSD42
#include "../vaxif/if_ilreg.h"
#include "../vaxif/if_il.h"
#else
#include "../h/if_ilreg.h"
#include "../h/if_il.h"
#endif
#include "../chncp/chil.h"
#endif
#endif

/*
 * For Ethernet interfaces.
 */
#ifdef NCHETHER
#ifdef BSD42
#include "chether.h"
#endif
#include "../chunix/chenet.h"
#endif

/*
 * This union should contain all device info structures in use.
 */
union xcinfo {
#ifdef NCHDR
	struct dr11cinfo	xc_dr11c;	/* for dr11c's */
#endif
#ifdef NCHCH
	struct ch11info		xc_ch11;	/* for ch11's */
#endif
#ifdef NCHIL
	struct chilinfo		xc_chil;	/* for chil's */
#define xc_ilinfo xc_info.xc_chil
#endif
#ifdef NCHETHER
	struct chetherinfo	xc_chether;	/* for chether's */
#define xc_etherinfo xc_info.xc_chether
#endif
	/* other devices go here */
};
