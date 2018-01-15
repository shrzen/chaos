#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/types.h> // xx brad
#include <linux/poll.h> // xx brad
#include <linux/file.h>
#include <linux/anon_inodes.h>

#include <linux/module.h>
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

static int			chdebug = 1;

#ifdef DEBUG_CHAOS
#define ASSERT(x,y)	if(!(x)) printk("%s: Assertion failure\n",y);
#define DEBUGF		if (chdebug) printk
#else
#define ASSERT(x,y)
#define DEBUGF		if (0) printk
#endif

unsigned long	alloc_count;
unsigned long	free_count;
unsigned long	alloc_bytes;
unsigned long	free_bytes;

char *
ch_alloc(int size, int cantwait)
{
	char *p;

	if ((p = kmalloc(size + 4, cantwait ? GFP_ATOMIC : GFP_KERNEL))) {
		*(int *)p = size;
		p += 4;
	}

	if (p) {
	  DEBUGF("ch_alloc(%d) = %p\n", size, p-4);

	  alloc_count++;
	  alloc_bytes += size;
	}

	return p;
}

void
ch_free(char *p)
{
	p -= 4;
	DEBUGF("ch_free(%p)\n", p);

	free_count++;
	free_bytes += *(int *)p;

	kfree(p);
}

int
ch_size(char *p)
{
	p -= 4;
	return *(int *)p;
}

void ch_bufalloc(void) {}
void ch_buffree(void) {}
