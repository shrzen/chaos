/*
 * int chopen(int address, char *contact, int mode, int async, char *data, int dlength, int rwsize)
 * int chlisten(char *contact, int mode, int async, int rwsize)
 * int chreject(int fd, char *string)
 * int chstatus(int fd, struct chstatus *chst)
 * int chsetmode(int fd, int mode)
 * int chwaitfornotstate(int fd, int state)
 */

#define NO_CHAOS_SOCKET 1

#if NO_CHAOS_SOCKET
#include "chlib-ioctl.c"
#else
#include "chlib-socket.c"
#endif
