#define NO_CHAOS_SOCKET 1

#if NO_CHAOS_SOCKET
#include "chlib-ioctl.c"
#else
#include "chlib-socket.c"
#endif
