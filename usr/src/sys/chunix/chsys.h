#ifndef _CHSYS_
#define _CHSYS_
/*
 * Operating system dependent definitions for UNIX (currently on VMUNIX)
 * This file contains definitions which must be supplied to the system
 * independent parts of the NCP.
 * It should be minimal.
 */
#ifndef pdp11
#define DEBUG
#endif
#define CHSTRCODE		/* UNIX interface needs stream code */
/*
 * OP Sys dependent part of connection structure
 */
struct csys_header {
	struct tty	*csys_ttyp;	/* tty struct ptr if there is one */
	char		csys_mode;	/* How we use this connection */
	char		csys_flags;	/* System dependent flags */
};
/*
 * cn_sflags definitions.
 */
#define CHIWAIT		01	/* Someone waiting on input */
#define CHOWAIT		02	/* Someone waiting to output */
#define CHRAW		04	/* this channel is open by raw driver */
#define CHCHANNEL	010	/* this channel is open by channel driver */
#define cn_mode		cn_syshead.csys_mode
#define cn_sflags	cn_syshead.csys_flags
#define cn_ttyp		cn_syshead.csys_ttyp
/*
 * macro definitions for process wakeup
 */
#define NEWSTATE(x)	{ \
				wakeup((char *)(x)); \
				if ((x)->cn_mode == CHTTY) \
					chttynstate(conn);	\
				else { \
					INPUT(conn); \
					OUTPUT(conn); \
				} \
			}

#define INPUT(x)	{ \
				if ((x)->cn_sflags&CHIWAIT) { \
					(x)->cn_sflags &= ~CHIWAIT; \
					wakeup((char *)&(x)->cn_rhead); \
		 		} \
				if ((x)->cn_mode == CHTTY) \
					chttyrint(x); \
			}
#define OUTPUT(x)	{ \
				if ((x)->cn_sflags&CHOWAIT) { \
					(x)->cn_sflags &= ~CHOWAIT; \
					wakeup((char *)&(x)->cn_thead); \
		  		} \
				if ((x)->cn_mode == CHTTY) \
					chttyxint(x); \
			}
#define RFCINPUT	{ \
				if (Rfcwaiting) { \
					Rfcwaiting = 0; \
					wakeup((char *)&Chrfclist); \
				} \
			}
/*
 * These should be lower is software interrupts are used.
 */
#define LOCK		(void) spl6()
#define UNLOCK		(void) spl0()

#define NOINPUT(conn)
#define NOOUTPUT(conn)

extern int Rfcwaiting;
/*
 * The third argument to iomove doesn't use B_READ and B_WRITE to
 * avoid being the only thing that needs UNIX include files.
 */
#define CHWCOPY(from, to, count) (iomove(to, count, 0), (char *)0)
#define CHRCOPY(from, to, count) (iomove(from, count, 1), (char *)0)
