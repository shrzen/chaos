/* chenet.h --- ---!!!
 */

struct chetherinfo {
#ifdef BSD42
	struct arpcom *che_arpcom;
#endif
#ifdef linux
	void *prot_hook1;
	void *prot_hook2;
	void *bound_dev;
#endif
};
#define ETHERTYPE_CHAOS 0x0804 /* Chaos protocol */
