/*
 * Linux device driver interface to the Chaos N.C.P.
 *
 * The model is a pseudo device which "clones" FD's; not right in
 * today's world but it made sense in 1985.  Rather than make this a
 * full blown network family this just ports the existing
 * functionality.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/poll.h>
#include <linux/file.h>
#include <linux/anon_inodes.h>
#include <linux/cred.h>
#include <linux/seq_file.h>
#include <linux/param.h>
#include <linux/signal.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>

#include <asm/ioctls.h>
#include <asm/segment.h>
#include <asm/irq.h>
#include <asm/io.h>

#include "chaos.h"
#include "chsys.h"
#include "chunix/chconf.h"
#include "chncp/chncp.h"
#include "chunix/charp.h"

#include "chlinux.h"

#define f_data private_data

extern struct chxcvr chetherxcvr[NCHETHER];

wait_queue_head_t Rfc_wait_queue;

void chtimeout_stop(void);
void chtimeout(unsigned long t);

void
praddr(struct seq_file *m, short h)
{
	seq_printf(m, "%02d.%03d", ((chaddr *)&h)->subnet & 0377,
		   ((chaddr *)&h)->host & 0377);
}

void
pridle(struct seq_file *m, chtime n)
{
	int idle;

	idle = Chclock - n;
	if (idle < 0)
		idle = 0;
	if (idle < Chhz)
		seq_printf(m, "%3dt", idle);
	else {
		idle += Chhz/2;
		idle /= Chhz;
		if (idle < 100)
			seq_printf(m, "%3ds", idle);
		else if (idle < 60*99)
			seq_printf(m, "%3dm", (idle + 30) / 60);
		else
			seq_printf(m, "%3dh", (idle + 60*30) / (60*60));
	}
}

void
prconn(struct seq_file *m, struct connection *conn, int i)
{
	///---!!! See chstat for what to do.
}

void
prrfcs(struct seq_file *m)
{
	///---!!! See chstat for what to do.
}

static char *xstathead =
	" Received Transmitted CRC(xcvr) CRC(buff) Overrun Aborted Length Rejected";

void
prxcvr(struct seq_file *m, char *name, struct chxcvr *xcvr, int nx)
{
	struct chxcvr *xp = xcvr;
	int i;

	for (i = 0; i < nx; i++, xp++) {
		if (xp->xc_etherinfo.bound_dev == 0)
			break;
		seq_printf(m, "%-8s %2d Netaddr: ", name, i);
		praddr(m, CH_ADDR_SHORT(xcvr->xc_addr));
		seq_printf(m, " Devaddr: %x\n", xp->xc_devaddr);
		seq_printf(m, "%s\n%9D%12D%10D%8D%8D%8D%8D%8D\n", xstathead,
			   xp->LELNG_xc_rcvd, xp->xc_xmtd, xp->xc_crcr, xp->xc_crci,
			   xp->xc_lost, xp->xc_abrt, xp->xc_leng, xp->xc_rej);
		seq_printf(m, " Tpacket Ttime Rpacket Rtime");
		seq_printf(m, "\n%6x ", xp->xc_tpkt);
		pridle(m, xp->xc_ttime);
		seq_printf(m, " %6x ", xp->xc_rpkt);
		pridle(m, xp->xc_rtime);
		seq_printf(m, "\n");
	}
}

/* This behaves like the chstat command.  */
int
chaos_proc_show(struct seq_file *m, void *v)
{
	int i;

	seq_printf(m, "Myaddr is 0%o", Chmyaddr);
	seq_printf(m, "\nConnections:\n # T St  Remote Host  Idx Idle Tlast Trecd Tackd Tw Rlast Rackd Rread Rw Flags\n");
	for (i = 0; i < CHNCONNS; i++)
		if (Chconntab[i] != NOCONN)
			prconn(m, Chconntab[i], i);
	if (Chrfclist)
		prrfcs(m);
	prxcvr(m, "ETHER", chetherxcvr, NCHETHER);
	return 0;
}

static int
chaos_proc_open(struct inode *inode, struct file *file)
{
	trace("inode=%p, file=%p\n", inode, file);
	return single_open(file, chaos_proc_show, NULL);
}

static struct file_operations chaos_proc_fops = {
	.open = chaos_proc_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

ssize_t chrread(struct file *fp, char *buf, size_t count, loff_t *offset);
ssize_t chrwrite(struct file *file, const char *buf, size_t count, loff_t *offset);
long chrioctl(struct file *fp, unsigned int cmd, unsigned long addr);
int chropen(struct inode * inode, struct file * file);
int chrclose(struct inode * inode, struct file * file);

static struct file_operations chaos_fops = {
	.read =	chrread,
	.write = chrwrite,
	.unlocked_ioctl = chrioctl,
	.open =	chropen,
	.release = chrclose
};

static struct class *chaos_class;
static int chaos_major;

static struct device *chaos_device;
static struct device *churfc_device;

int
chaos_init(void)
{
	int ret;

	trace();

	init_waitqueue_head(&Rfc_wait_queue);

	chaos_class = class_create(THIS_MODULE, "chaos");
	if (IS_ERR(chaos_class)) {
		printk(KERN_ALERT "failed to create device class\n");
		ret = PTR_ERR(chaos_class);
		goto err;
	}

	chaos_major = register_chrdev(0, "chaos", &chaos_fops);
	if (chaos_major < 0) {
		printk(KERN_ALERT "failed to register major number\n");
		ret = chaos_major;
		goto err_class;
	}
	trace("created chaos device (major %d)\n", chaos_major);

	chaos_device =
		device_create(chaos_class, NULL, MKDEV(chaos_major, 1), NULL,
			      "chaos");
	if (IS_ERR(chaos_device)) {
		printk(KERN_ALERT "failed to create /dev/chaos\n");
		ret = PTR_ERR(chaos_device);
		goto err_chrdev;
	}
	trace("created /dev/chaos\n");

	churfc_device =
		device_create(chaos_class, NULL, MKDEV(chaos_major, 2), NULL,
			      "churfc");
	if (IS_ERR(churfc_device)) {
		printk(KERN_ALERT "failed to create /dev/churfc\n");
		ret = PTR_ERR(churfc_device);
		goto err_device_chaos;
	}
	trace("created /dev/churfc\n");

	//---!!! Create the proc entry in /proc/net/chaos.
	if (!proc_create("chaos", 0, NULL, &chaos_proc_fops)) {
		printk(KERN_ALERT "failed to create proc entry\n");
		ret = -ENOMEM;
		goto err_device_churfc;
	}
	trace("created /proc/chaos\n");

	trace("chaos_init() ok\n");
	return 0;

	remove_proc_entry("chaos", NULL);
err_device_churfc:
	device_destroy(chaos_class, MKDEV(chaos_major, 2)); // CHURFC
err_device_chaos:
	device_destroy(chaos_class, MKDEV(chaos_major, 1)); // CHAOS
err_chrdev:
	unregister_chrdev(chaos_major, "chaos");
err_class:
	class_destroy(chaos_class);
err:
	trace("chaos_init() FAIL:\n", ret);
	return ret;
}

void
chaos_deinit(void)
{
	trace();

	chtimeout_stop();
	chdeinit();

	remove_proc_entry("chaos", NULL);

	device_destroy(chaos_class, MKDEV(chaos_major, 2)); // CHURFC
	device_destroy(chaos_class, MKDEV(chaos_major, 1)); // CHAOS

	unregister_chrdev(chaos_major, "chaos");
	class_destroy(chaos_class);
}

MODULE_LICENSE("GPL");
module_init(chaos_init);
module_exit(chaos_deinit);
