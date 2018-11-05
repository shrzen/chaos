/*
 *	$Source: /afs/dev.mit.edu/source/src77/bsd-4.3/vax/sys/attic/chunix/RCS/chsys.h,v $
 *	$Author: cfields $
 *	$Locker:  $
 *	$Log: chsys.h,v $
 * Revision 1.1  1994/09/16  11:06:40  cfields
 * Initial revision
 *
 * Revision 1.3  87/04/10  15:50:22  rc
 * Added the Select wakeup code segment which previously was not there.
 * 
 * Revision 1.2  87/04/06  10:39:40  rc
 * The definition of spl6() is changed to splimp() in order to maintain
 * consistency with the rest of the Chaos code
 * 
 * Revision 1.1  84/06/12  20:08:55  jis
 * Initial revision
 * 
 */
#ifndef _CHSYS_
#define _CHSYS_
/*
 * Operating system dependent definitions for UNIX (currently on vax)
 * This file contains definitions which must be supplied to the system
 * independent parts of the NCP.
 * It should be minimal.
 */
#include "../h/param.h"		/* For u_char etc. */
#define DEBUG
#ifdef vax
#include "cht.h"
#endif
#define CHSTRCODE		/* UNIX interface needs stream code */
/*
 * OP Sys dependent part of connection structure
 */
struct csys_header {
#if NCHT > 0
	struct tty	*csys_ttyp;	/* tty struct ptr if there is one */
#endif
	char		csys_mode;	/* How we use this connection */
	char		csys_flags;	/* System dependent flags */
};
/*
 * cn_sflags definitions.
 */
#define CHIWAIT		01	/* Someone waiting on input */
#define CHOWAIT		02	/* Someone waiting to output */
#define CHRAW		04	/* this channel is open by raw driver */
#define CHCLOSING	010	/* Top end is closing output.  Any input
				   should abort the connection */
#define CHFWRITE	020	/* Connection is open for writing */
#define cn_mode		cn_syshead.csys_mode
#define cn_sflags	cn_syshead.csys_flags
#define cn_ttyp		cn_syshead.csys_ttyp
#if NCHT ==  0
#define chtnstate()
#define chtxint()
#define chtrint()
#endif

/*
 * macro definitions for process wakeup
 */
#define NEWSTATE(x)	{ \
				wakeup((char *)(x)); \
				if ((x)->cn_mode == CHTTY) \
					chtnstate(conn);	\
				else { \
					INPUT(conn); \
					OUTPUT(conn); \
				} \
			}

#define INPUT(x)	{ \
				if ((x)->cn_sflags&CHCLOSING) { \
					(x)->cn_sflags &= ~CHCLOSING; \
					clsconn(conn, CSCLOSED, NOPKT); \
				} else { \
					if ((x)->cn_sflags&CHIWAIT) { \
						(x)->cn_sflags &= ~CHIWAIT; \
						wakeup((char *)&(x)->cn_rhead); \
					} \
					if ((x)->cn_rsel) { \
						selwakeup((x)->cn_rsel, (x)->cn_flags & CHSELRCOLL); \
						(x)->cn_rsel = 0; \
						(x)->cn_flags &= ~CHSELRCOLL; \
					} \
					if ((x)->cn_mode == CHTTY) \
						chtrint(x); \
				} \
			}
#define OUTPUT(x)	{ \
				if ((x)->cn_sflags&CHOWAIT) { \
					(x)->cn_sflags &= ~CHOWAIT; \
					wakeup((char *)&(x)->cn_thead); \
		  		} \
				if ((x)->cn_tsel) { \
					selwakeup((x)->cn_tsel, (x)->cn_flags & CHSELTCOLL); \
					(x)->cn_tsel = 0; \
					(x)->cn_flags &= ~CHSELTCOLL; \
				} \
				if ((x)->cn_mode == CHTTY) \
					chtxint(x); \
			}
#define RFCINPUT	{ \
				if (Rfcwaiting) { \
					Rfcwaiting = 0; \
					wakeup((char *)&Chrfclist); \
				} \
			}
/*
 * These should be lower if software interrupts are used.
 */
#define LOCK		(void) splimp()
#define UNLOCK		(void) spl0()

#define NOINPUT(conn)
#define NOOUTPUT(conn)

extern int Rfcwaiting;

#define CHWCOPY(from, to, count, arg, errorp) \
		(char *)chwcopy(from, to, count, arg, errorp)
#define CHRCOPY(from, to, count, arg, errorp) \
		(char *)chrcopy(from, to, count, arg, errorp)
#endif
