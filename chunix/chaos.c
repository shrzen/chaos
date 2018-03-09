/*
 * UNIX device driver interface(s) to the Chaos N.C.P.
 */
#include "../h/chaos.h"
#include "chsys.h"
#include "chconf.h"
#include "../chncp/chncp.h"

#ifdef BSD42
#include "chaos-bsd42.c"
#elif defined(linux) && defined(__KERNEL__)
#include "chaos-linux.c"
#else
#include "chaos-socket.c"
#endif
