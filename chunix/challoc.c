/*
 * challoc.c
 * UNIX memory allocator. For V7, 4.1 BSD, 4.2 BSD and Linux.
 * This file really contains three entirely separate implementations, one
 * for 4.2 BSD which uses its network buffers, and the other for 11 and
 * 4.1 BSD implementations, and one implementation for Linux.
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
