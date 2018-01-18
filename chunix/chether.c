#include <linux/types.h> // xx brad

#include "../h/chaos.h"
#include "chsys.h"
#include "chconf.h"
#include "../chncp/chncp.h"
#include "challoc.h"
#include "charp.h"

#ifdef BSD42
#include "mbuf.h"
#include "errno.h"
#include "socket.h"
#include "net/if.h"
#include "netinet/in.h"
#include "netinet/if_ether.h"
#endif

#ifdef linux
#include <linux/signal.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/proc_fs.h>
#include <linux/if_arp.h>
#include <linux/netdevice.h>

#include <asm/uaccess.h>

#include <asm/byteorder.h>

#include "chlinux.h"

#define ETHERTYPE_ARP ETH_P_ARP
#endif

#ifdef DEBUG_CHAOS
extern int		chdebug;
#define DEBUGF		if (chdebug) printf
#else
#define ASSERT(x,y)
#define DEBUGF		if (0) printf
#endif

#ifdef linux
#define printf printk
#endif

void chedeinit(void);

/*
 * This file provides the glue between the 4.2 BSD ethernet device drivers and
 * the Chaos NCP.
 */
struct chxcvr chetherxcvr[NCHETHER];
struct ar_pair charpairs[NPAIRS];
long	charptime = 1;	/* LRU clock for ar_pair slots */

#ifdef BSD_INTF
#include "chether-bsd.c"
#endif /* BSD_INTF */

#ifdef linux
#include "chether-linux.c"
#endif /* linux */
