/*
 * This file provides the glue between the 4.2 BSD ethernet device drivers and
 * the Chaos NCP.
 */

#ifdef BSD_INTF
#include "chether-bsd42.c"
#endif /* BSD_INTF */

#ifdef linux
#include "chether-linux.c"
#endif /* linux */
