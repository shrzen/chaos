/*
 * Operating system dependent definitions
 * This file contains definitions which must be supplied to the system
 * independent parts of the NCP.
 * It should be minimal.
 */

#include <linux/sched.h>
#include <linux/wait.h>

#include "chlinux.h"

#define DEBUG_CHAOS

#define CHSTRCODE		/* UNIX interface needs stream code */

/*
 * OP Sys dependent part of connection structure
 */
struct csys_header {
	char		csys_mode;	/* How we use this connection */
	char		csys_flags;	/* System dependent flags */
	wait_queue_head_t csys_state_wait;	/* state change wait queue */
	wait_queue_head_t csys_write_wait;	/* select wait queue */
	wait_queue_head_t csys_read_wait;	/* select wait queue */
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
#define cn_state_wait	cn_syshead.csys_state_wait
#define cn_rfc_wait	cn_syshead.csys_rfc_wait
#define cn_write_wait	cn_syshead.csys_write_wait
#define cn_read_wait	cn_syshead.csys_read_wait

#define chtnstate(a)
#define chtxint(a)
#define chtrint(a)

/*
 * macro definitions for process wakeup
 */

#define NEWSTATE(x)	{ \
				wake_up_interruptible(&(x)->cn_state_wait); \
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
					if ((x)->cn_sflags & CHIWAIT) { \
						(x)->cn_sflags &= ~CHIWAIT; \
						wake_up_interruptible(&(x)->cn_read_wait); \
					} \
					if ((x)->cn_mode == CHTTY) \
						chtrint(x); \
				} \
			}

#define OUTPUT(x)	{ \
				if ((x)->cn_sflags & CHOWAIT) { \
					(x)->cn_sflags &= ~CHOWAIT; \
					wake_up_interruptible(&(x)->cn_write_wait); \
		  		} \
				if ((x)->cn_mode == CHTTY) \
					chtxint(x); \
			}

#define RFCINPUT	{ \
				if (Rfcwaiting) { \
					Rfcwaiting = 0; \
					wake_up_interruptible(&Rfc_wait_queue); \
				} \
			}
/*
 * These should be lower if software interrupts are used.
 */

#define CHLOCK		spin_lock_irq(&chaos_lock)
#define CHUNLOCK	spin_unlock_irq(&chaos_lock)

#define NOINPUT(conn)
#define NOOUTPUT(conn)

extern int Rfcwaiting;
extern wait_queue_head_t Rfc_wait_queue;	/* rfc input wait queue */

#define CHWCOPY(from, to, count, arg, errorp) \
		(char *)chwcopy(from, to, count, arg, errorp)
#define CHRCOPY(from, to, count, arg, errorp) \
		(char *)chrcopy(from, to, count, arg, errorp)
