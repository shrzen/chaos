/*
 *	$Source: /afs/dev.mit.edu/source/src77/bsd-4.3/vax/sys/attic/chncp/RCS/chil.h,v $
 *	$Author: cfields $
 *	$Locker:  $
 *	$Log: chil.h,v $
 * Revision 1.1  1994/09/16  11:06:21  cfields
 * Initial revision
 *
 * Revision 1.1  84/06/12  20:27:39  jis
 * Initial revision
 * 
 */
/*
 * Definitions for interlan ethernet interface used as chaos network interface
 */

/*
 * structure needed for each transmitter. (member of xminfo union).
 */
struct chilinfo	{
	struct chilsoft *il_soft;
};

#define CHILBASE	0764000		/* base UNIBUS address for ch11's */
#define CHILINC		010		/* ??? */


#define ILCHAOS_TYPE	0x0408		/* CHAOS protocol */
#define ILADDR_TYPE	0x0608		/* Address resolution (a la DCP) */
