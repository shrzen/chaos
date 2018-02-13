/*
 * This file contains software configuration parameters for the NCP,
 * as well as the definitions
 * for the hardware interfaces in use
 */

/*
 * Software configuration parameters - system independent
 */
#ifdef vax
#define CHNCONNS	0140	/* Maximum number of connections */
#else
#define CHNCONNS	20	/* Maximum number of connections */
#endif
#define CHDRWSIZE	5	/* Default receive window size */
#define CHMINDATA	(32-CHHEADSIZE)
#define CHSHORTTIME	(Chhz>>4)	/* Short time for retransmits */
#define CHNSUBNET	(CHMAXDATA/sizeof(struct rut_data))	/* Number of subnets in routing table */

/*
 * Software configuration parameters - system dependent
 */
#define chroundup(n)	((n) <= (32-CHHEADSIZE) ? (32-CHHEADSIZE) : \
			 (n) <= (128-CHHEADSIZE) ? (128-CHHEADSIZE) : \
			 (512-CHHEADSIZE) )

#define CHBUFTIME	5*Chhz	/* timeout ticks for no buffers (used?) */

void chreset(void);
void chdeinit(void);
void chxtime(void);

/*
 * Hardware configuration parameters
 *
 * Foreach interface type (e.g. xxzzz), you need:

#include "chxx.h"
#if NCHXX > 0
#include <chaos/xxzzz.h>	* for device register definitions and
				   struct xxxxinfo defining software state *
extern short xxxxhosts[];	* array of actual host numbers
				   (initialized in chconf.c) *
				* only needed for interfaces that don't know
				  their own address *
#endif
 */

#if 0 ///---!!! Old devices.
/*
 * For dr11-c's
 */
#include "chdr.h"
#ifdef NCHDR
#include "../chncp/dr11c.h"
extern short dr11chosts[];	/* local host address per dr11c interface */
#endif

/*
 * For ch11's - if not vax we must specify number of interfaces here.
 */
#include "chch.h"
#ifdef NCHCH
#include "../chncp/ch11.h"
#endif

/*
 * For chil's.
 */
#include "chil.h"
#ifdef NCHIL
#ifdef BSD42
#include "../vaxif/if_ilreg.h"
#include "../vaxif/if_il.h"
#else BSD42
#include "../h/if_ilreg.h"
#include "../h/if_il.h"
#endif
#include "../chncp/chil.h"
#endif
#endif

/*
 * For ethernet interfaces in 4.2BSD
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
