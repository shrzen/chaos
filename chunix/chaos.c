/* chaos.c --- driver interface(s) to the Chaos N.C.P.
 */

#ifdef BSD42
#include "chaos-bsd42.c"
#elif defined(linux) && defined(__KERNEL__)
#include "chaos-linux.c"
#else
#include "chaos-socket.c"
#endif
