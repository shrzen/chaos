/*
 *	$Source: /afs/dev.mit.edu/source/src77/bsd-4.3/vax/sys/attic/chunix/RCS/chenet.h,v $
 *	$Author: cfields $
 *	$Locker:  $
 *	$Log: chenet.h,v $
 * Revision 1.1  1994/09/16  11:06:41  cfields
 * Initial revision
 *
 * Revision 1.2  86/09/29  14:47:32  mbm
 * ETHERPUP_CHAOSTYPE --> ETHERTYPE_CHAOS
 * 
 * Revision 1.1  84/06/12  20:08:54  jis
 * Initial revision
 * 
 */
struct chetherinfo	{
	struct arpcom *che_arpcom;
};
#define ETHERTYPE_CHAOS		0x0804		/* Chaos protocol */
