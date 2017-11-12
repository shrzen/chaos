/*
 *	$Source: /usr/src/sys/netchaos/chunix/RCS/chenet.h,v $
 *	$Author: mbm $
 *	$Locker: mbm $
 *	$Log:	chenet.h,v $
 * Revision 1.2  86/09/29  14:47:32  mbm
 * ETHERPUP_CHAOSTYPE --> ETHERTYPE_CHAOS
 * 
 * Revision 1.1  84/06/12  20:08:54  jis
 * Initial revision
 * 
 */
struct chetherinfo	{
#ifdef BSD42
	struct arpcom *che_arpcom;
#endif
#ifdef linux
	void *prot_hook1;
	void *prot_hook2;
	void *bound_dev;
#endif
};
#define ETHERTYPE_CHAOS		0x0804		/* Chaos protocol */
