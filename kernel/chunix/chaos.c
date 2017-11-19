/*
 * Revision 1.6  87/04/06  10:28:05  rc
 * This is essentially the same as revision 1.5 except that the RCS log
 * was not updated to reflect the changes. In particular, calls to spl6()
 * are replaced with calls to splimp() in order to maintain consistency
 * with the rest of the Chaos code.
 * 
 * Revision 1.5  87/04/06  07:52:49  rc
 * Modified this file to include the select stuff for the ChaosNet. 
 * Essentially, the new distribution from "think" was integrated
 * together with the local modifications at MIT to reflect this
 * new version.
 * 
 * Revision 1.4.1.1  87/03/30  21:06:16  root
 * The include files are updated to reflect the correct "include" directory.
 * These changes were not previously RCSed. The original changes did not
 * have lines of the sort ... #include "../netchaos/...."
 * 
 * Revision 1.4  86/10/07  16:16:32  mbm
 * CHIOCGTTY ioctl; move DTYPE_CHAOS to file.h
 * 
 * Revision 1.2  84/11/05  18:24:31  jis
 * Add checks for allocation failure after some calls to pkalloc
 * so that when pkalloc fails to allocate a packet, the system
 * no longer crashes.
 * 
 * Revision 1.1  84/11/05  17:35:32  jis
 * Initial revision
 * 
 */

/*
 * UNIX device driver interface(s) to the Chaos N.C.P.
 */
#include "../h/chaos.h"
#include "chsys.h"
#include "chconf.h"
#include "../chncp/chncp.h"

#ifdef BSD42
#include "chaos-bsd.c"
#elif defined(linux) && defined(__KERNEL__)
#include "chaos-linux.c"
#elif
#include "chaos-socket.c"
#endif
