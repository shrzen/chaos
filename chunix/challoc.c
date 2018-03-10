/* challoc.c --- memory allocator
 *
 * This file really contains several entirely separate
 * implementations, one for 4.2 BSD which uses its network buffers,
 * and the other for 11 and 4.1 BSD implementations, one
 * implementation for Linux and one implementation for Unix domain
 * sockets.
 *
 * char *ch_alloc(int size, int cantwait)
 * void ch_free(char *p)
 * int ch_size(char *p)
 * int ch_badaddr(char *p)
 *
 * void ch_bufalloc(void)
 * void ch_buffree(void)
 */

#if defined(linux) && defined(__KERNEL__)
#include "challoc-linux.c"
#elif defined(linux) && !defined(__KERNEL__)
#include "challoc-socket.c"
#elif BSD42
#include "challoc-bsd42.c"
#else
#include "challoc-bsd41.c"
#endif
