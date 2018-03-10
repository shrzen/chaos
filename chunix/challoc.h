/* challoc.h --- definitions for memory allocator
 *
 * Definitions which are shared between the system dependent storage
 * allocator and other system dependent modules.
 */

extern char *ch_alloc(int size, int cantwait);
extern void ch_free(char *p);
extern int ch_size(char *p);
extern int ch_badaddr(char *p);

extern void ch_bufalloc(void);
extern void ch_buffree(void);
