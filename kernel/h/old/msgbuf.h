/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)msgbuf.h	7.1 (Berkeley) 6/4/86
 */
/*
 * RCS Info	
 *	$Header: /home/ams/c-rcs/chaos-2000-07-03/kernel/h/old/msgbuf.h,v 1.1.1.1 1998/09/07 18:56:09 brad Exp $
 *	$Locker:  $
 */

#define	MSG_MAGIC	0x063061
#define	MSG_BSIZE	(4096 - 3 * sizeof (long))
struct	msgbuf {
	long	msg_magic;
	long	msg_bufx;
	long	msg_bufr;
	char	msg_bufc[MSG_BSIZE];
};
#ifdef KERNEL
struct	msgbuf msgbuf;
#endif
