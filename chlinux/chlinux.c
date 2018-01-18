/*
 * Copied from chaos.c for linux
 * Basically the same functionality as chaos.c, but linux specific.
 * This is the o/s specific wrapper to generic NCP code in ../chncp
 *
 * the model is a pseudo device which "clones" fd's; not right in today's
 * world but it made sense in 1985.  rather than make this a full blown
 * network family I chose to just port the existing functionality.
 *
 * brad@parker.boston.ma.us
 */

/*
 * UNIX device driver interface to the Chaos N.C.P.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/types.h> // xx brad
#include <linux/poll.h> // xx brad
#include <linux/file.h>
#include <linux/anon_inodes.h>

#include "../h/chaos.h"
#include "chsys.h"
#include "chconf.h"
#include "../chncp/chncp.h"
#include "charp.h"
/*#include <linux/config.h>*/

#include <linux/types.h>
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

#include "chlinux.h"

#define f_data private_data

int			Rfcwaiting;	/* Someone waiting on unmatched RFC */
wait_queue_head_t 	Rfc_wait_queue;/* rfc input wait queue */
///---!!!int			Chrfcrcv;
int			chdebug = 1;

#define CHIOPRIO	(PZERO+1)	/* Just interruptible */

#ifdef DEBUG_CHAOS
#define ASSERT(x,y)	if(!(x)) printk("%s: Assertion failure\n",y);
#define DEBUGF		if (chdebug) printk
#else
#define ASSERT(x,y)
#define DEBUGF		if (0) printk
#endif

void chtimeout_stop(void);
void chtimeout(unsigned long t);
  
static struct inode *get_inode(struct inode *i)
{
	struct inode * inode;

	inode = new_inode(i->i_sb);
	if (!inode)
		return NULL;

	inode->i_mode = S_IFSOCK;
///---!!!	inode->i_sock = 1;
	inode->i_uid = current_fsuid();
	inode->i_gid = current_fsgid();

	return inode;
}

static void free_inode(struct inode *inode)
{
	iput(inode);
}

static void praddr(struct seq_file *m, short h)
{
	seq_printf(m, "%02d.%03d",
		       ((chaddr *)&h)->subnet & 0377,
		       ((chaddr *)&h)->host & 0377);
}

static void pridle(struct seq_file *m, chtime n)
{
	register int idle;

	idle = Chclock - n;
	if (idle < 0)
		idle = 0;
	if (idle < Chhz) {
		seq_printf(m, "%3dt", idle);
		return;
	}

	idle += Chhz/2;
	idle /= Chhz;
	if (idle < 100) {
	    seq_printf(m, "%3ds", idle);
	    return;
	}
	else if (idle < 60*99) {
	    seq_printf(m, "%3dm", (idle + 30) / 60);
	    return;
	}

	seq_printf(m, "%3dh", (idle + 60*30) / (60*60));
}

int
chaos_proc_show(struct seq_file *m, void *v)
{
        /* From  net/socket.c  */
        extern int socket_get_info(char *, char **, off_t, int);
        extern struct proto packet_prot;

        int i, len = 0;

        seq_printf(m,"Myaddr is 0%o\n", Chmyaddr);

#if 0
	len += sprintf(buffer+len"Connections:\n # T St  Remote Host  Idx Idle Tlast Trecd Tackd Tw Rlast Rackd Rread Rw Flags\n");

	for (i = 0; i < CHNCONNS; i++)
	    if (Chconntab[i] != NOCONN)
		prconn(Chconntab[i], i);

	if (Chrfclist)
	    prrfcs();
#endif

	{
	    struct chxcvr *xp;
	    struct device *dev;
	    extern struct chxcvr chetherxcvr[NCHETHER];

	    for (xp = chetherxcvr; xp->xc_etherinfo.bound_dev; xp++) {
		if (xp->xc_etherinfo.bound_dev == 0)
		    break;
		if ((dev = xp->xc_etherinfo.bound_dev)) {
		    seq_printf(m, "%-8s Netaddr: ",
				   /*dev->name*/"enp0s25");
		    praddr(m,
				  CH_ADDR_SHORT(xp->xc_addr));
		    seq_printf(m," Devaddr: %x\n",
				   xp->xc_devaddr);

		    seq_printf(m,
				   " Received Transmitted CRC(xcvr) CRC(buff) Overrun Aborted Length Rejected\n");
		    seq_printf(m,
				   "%9d%12d%10d%8d%8d%8d%8d%8d\n",
				   xp->LELNG_xc_rcvd, xp->xc_xmtd, xp->xc_crcr,
				   xp->xc_crci, xp->xc_lost, xp->xc_abrt,
				   xp->xc_leng, xp->xc_rej);
		    seq_printf(m,
				   " Tpacket Ttime Rpacket Rtime\n");
		    seq_printf(m,"%8x ", xp->xc_tpkt);
		    pridle(m,xp->xc_ttime);
		    seq_printf(m," %8x ", xp->xc_rpkt);
		    pridle(m,xp->xc_rtime);
		    seq_printf(m,"\n");
		}
	    }
	}

	{
		extern unsigned long	alloc_count;
		extern unsigned long	free_count;
		extern unsigned long	alloc_bytes;
		extern unsigned long	free_bytes;
		seq_printf(m,
			       "alloc  bytes   free   bytes\n");
		seq_printf(m,"%6d %7d %6d %7d\n",
			       alloc_count, alloc_bytes,
			       free_count, free_bytes);
		seq_printf(m,
			       "in-use bytes\n");
		seq_printf(m,"%6d %7d\n",
			       alloc_count - free_count,
			       alloc_bytes - free_bytes);
	}

#if 1
	{
	    extern struct ar_pair charpairs[NPAIRS];
	    register struct ar_pair *app;

		seq_printf(m,"Arp table:\n");
		seq_printf(m,"Addr   time\n");

		for (app = charpairs; app < &charpairs[NPAIRS]; app++) {
		    if (app->arp_chaos.host != 0) {
			seq_printf(m,"%7o %d\n",
				       app->arp_chaos.host, app->arp_time);
		    }
		}
	}
#endif
}


#if 0
int get_unused_fd(void)
{
	int fd;
	struct files_struct * files = current->files;

	fd = find_first_zero_bit(&files->open_fds, NR_OPEN);
	if (fd < current->rlim[RLIMIT_NOFILE].rlim_cur) {
		FD_SET(fd, &files->open_fds);
		FD_CLR(fd, &files->close_on_exec);
		return fd;
	}
	return -EMFILE;
}

inline void put_unused_fd(int fd)
{
	FD_CLR(fd, &current->files->open_fds);
}

static inline void remove_file_free(struct file *file)
{
	struct file *next, *prev;

	next = file->f_next;
	prev = file->f_prev;
	file->f_next = file->f_prev = NULL;
	if (first_file == file)
		first_file = next;
	next->f_prev = prev;
	prev->f_next = next;
}

static inline void put_last_free(struct file *file)
{
	struct file *next, *prev;

	next = first_file;
	file->f_next = next;
	prev = next->f_prev;
	next->f_prev = file;
	file->f_prev = prev;
	prev->f_next = file;
}

struct file * get_empty_filp(void)
{
	int i;
	struct file * f;

	if (!first_file)
		return NULL;

        for (f = first_file, i=0; i < nr_files; i++, f = f->f_next)
        	if (!f->f_count) {
			remove_file_free(f);
			memset(f,0,sizeof(*f));
			put_last_free(f);
			f->f_count = 1;
			f->f_version = ++event;
			return f;
		}

	return NULL;
}
#endif

static int
chaos_proc_open(struct inode *inode, struct file *file)
{
	DEBUGF("chaos_proc_show(inode=%p, file=%p)\n", inode, file);
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

	DEBUGF("chaos_init()\n");

	init_waitqueue_head(&Rfc_wait_queue);
	
	chaos_class = class_create(THIS_MODULE, "chaos");
	if (IS_ERR(chaos_class)) {
		printk(KERN_ALERT "failed to create device class\n");
		ret = PTR_ERR(chaos_class);
		goto err;
 	}
	
	chaos_major = register_chrdev(0, "chaos", &chaos_fops);
	if (chaos_major < 0) {
		printk(KERN_ALERT "failed to major number\n");
		ret = chaos_major;
		goto err_class;
	}
	DEBUGF("created chaos device (major %d)\n", chaos_major);
	
	chaos_device =
		device_create(chaos_class, NULL, MKDEV(chaos_major, 1), NULL,
			      "chaos");
	if (IS_ERR(chaos_device)) {
		printk(KERN_ALERT "failed to /dev/chaos\n");
		ret = PTR_ERR(chaos_device);
		goto err_chrdev;
	}
	DEBUGF("created /dev/chaos\n");
	
	churfc_device =
		device_create(chaos_class, NULL, MKDEV(chaos_major, 2), NULL,
			      "churfc");
	if (IS_ERR(churfc_device)) {
		printk(KERN_ALERT "failed to /dev/churfc\n");
		ret = PTR_ERR(churfc_device);
		goto err_device_chaos;
	}
	DEBUGF("created /dev/churfc\n");
	
	//---!!! Create the proc entry in /proc/net/chaos.
	if (!proc_create("chaos", 0, NULL, &chaos_proc_fops)) {
		printk(KERN_ALERT "failed to create proc entry\n");
		ret = -ENOMEM;
		goto err_device_churfc;
	}
	DEBUGF("created /proc/chaos\n");

	DEBUGF("chaos_init() ok\n");
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
	return ret;
}

void
chaos_deinit(void)
{
	DEBUGF("chaos_deinit()\n");

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
