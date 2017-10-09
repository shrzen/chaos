#include "../chunix/chconf.h"
#include "../chunix/chsys.h"
#include <chaos/chaos.h>
#include <chaos/dev.h>
#include "../h/param.h"
#include "../h/inode.h"
#include "../h/file.h"
#include "../h/dir.h"
#include "../h/user.h"
/*
 * This file contains the chaos channel driver, which allows access to
 * preexisting channels , created since boot time, which are not valid
 * after a reboot.  This is slightly dangerous, but only super-users can
 * create device nodes any way, so they should just be removed at boot time.
 */

/*
 * Open a file on an existing channel.
 * Can be called by different processes at the same time.
 * Basically we just need to indicate that the channel is open
 * by the channel driver.
 */
chcopen(dev, flag)
dev_t dev;
{
	register struct connection *conn;

	if (minor(dev) < 0 || minor(dev) >= CHNCONNS ||
	    (conn = Chconntab[minor(dev)]) == NOCONN ||
	    conn->cn_state != CSOPEN)
		u.u_error = ENXIO;
	else
		conn->cn_sflags |= CHCHANNEL;
}
/*
 * Close a channel, called on the last file which has it open.
 * Unless the raw interface still has it open (not normally the case)
 * just close it here.
 */
chcclose(dev, flag)
dev_t dev;
{
	register struct connection *conn;

	if ((conn = Chconntab[minor(dev)]) == NOCONN)
		panic("chclose");
	if (conn->cn_sflags & CHRAW)
		conn->cn_sflags &= ~CHCHANNEL;
	else
		chclose(conn, flag);
	/*
	 * Here we do something REALLY nasty, to be sure that the inode
	 * can't be used any more.  Namely we CHANGE THE MINOR DEVICE NUMBER!!
	 * Can you believe it?  I can't think of any harm...
	 */
	panic("chcclose");
/*
	fp->f_inode->i_un.i_rdev = makedev(major(dev), CHBADCHAN);
	fp->f_inode->i_flag |= ICHG;
*/
}
chcread(dev)
dev_t dev;
{
	register struct connection *conn;

	if ((conn = Chconntab[minor(dev)]) == NOCONN)
		panic("chcread");
	chread(conn);
}
chcwrite(dev)
dev_t dev;
{
	register struct connection *conn;

	if ((conn = Chconntab[minor(dev)]) == NOCONN)
		panic("chcread");
	chwrite(conn);
}
chcioctl(dev, cmd, addr, flag)
dev_t dev;
caddr_t addr;
{
	register struct connection *conn;

	if ((conn = Chconntab[minor(dev)]) == NOCONN)
		panic("chcioctl");
	chioctl(conn, dev, cmd, addr, flag);
}
