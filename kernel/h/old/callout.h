/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)callout.h	7.1 (Berkeley) 6/4/86
 */
/*
 * RCS Info	
 *	$Header: /home/ams/c-rcs/chaos-2000-07-03/kernel/h/old/callout.h,v 1.1.1.1 1998/09/07 18:56:08 brad Exp $
 *	$Locker:  $
 */

/*
 * The callout structure is for
 * a routine arranging
 * to be called by the clock interrupt
 * (clock.c) with a specified argument,
 * in a specified amount of time.
 * Used, for example, to time tab
 * delays on typewriters.
 */

struct	callout {
	int	c_time;		/* incremental time */
	caddr_t	c_arg;		/* argument to routine */
	int	(*c_func)();	/* routine */
	struct	callout *c_next;
};
#ifdef KERNEL
struct	callout *callfree, *callout, calltodo;
int	ncallout;
#endif
