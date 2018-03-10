/*
 * Calls that are independant of implementation:
 *
 *   int chopen(int address, char *contact, int mode, int async, char *data, int dlength, int rwsize)
 *     Matching ioctl: CHIOCOPEN	
 *   int chlisten(char *contact, int mode, int async, int rwsize)
 *   int chreject(int fd, char *string)
 *     Matching ioctl: CHIOCREJECT	
 *   int chstatus(int fd, struct chstatus *chst)
 *     Matching ioctl: CHIOCGSTAT	
 *   int chsetmode(int fd, int mode)
 *     Matching ioctl: CHIOCSMODE -- Set the mode of this channel
 *   int chwaitfornotstate(int fd, int state)
 *     Matching ioctl: CHIOCSWAIT -- Wait for a different state
 *
 * Calls that have no corresponding wrapper function:
 *
 *   CHIOCRSKIP		Skip the last read unmatched RFC 
 *   CHIOCPREAD		Read my next data or control pkt 
 *   CHIOCFLUSH		flush current output packet 
 *   CHIOCANSWER	Answer an RFC (in RFCRECVD state) 
 *   CHIOCACCEPT	Accept an RFC, opening connection 
 *   CHIOCOWAIT		Wait until all output acked
 *   CHIOCADDR		Set my address 
 *   CHIOCNAME		Set my name 
 *   CHIOCILADDR	Set chaos address for chil 
 *   CHIOCETHER		Set chaos address for ether 
 *   CHIOCGTTY		Hook up tty and return unit
 *
 */

#define NO_CHAOS_SOCKET 1

#if NO_CHAOS_SOCKET
#include "chlib-ioctl.c"
#else
#include "chlib-socket.c"
#endif
