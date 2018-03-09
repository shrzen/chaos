#include <linux/types.h>
#include <linux/signal.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/proc_fs.h>

#define verify_area(type, addr, size) access_ok(type, addr, size) ? 0 : -EFAULT

///---???
#define memcpy_fromfs copy_from_user
#define memcpy_tofs copy_to_user

///---???
#include <linux/irqflags.h>

#define dev_get(x) __dev_get_by_name(&init_net,x)

extern spinlock_t chaos_lock;

extern int chdebug;

#ifdef DEBUG_CHAOS
#define ASSERT(x,y)	if(!(x)) printk("%s: Assertion failure\n",y);
#define trace		if (chdebug) printk
#else
#define ASSERT(x,y)
#define trace		if (0) printk
#endif

#define printf printk
