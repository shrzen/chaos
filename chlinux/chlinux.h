#include <linux/errno.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/signal.h>
#include <linux/time.h>
#include <linux/types.h>

#include "chncp/chuser.h"
#include "chncp/chclock.h"
#include "chncp/chsend.h"
#include "chncp/chrcv.h"
#include "chncp/chutil.h"
#include "chunix/chtime.h"
#include "chunix/challoc.h"
#include "chunix/chether.h"

int ch_size(char *p);
void ch_free(char *p);

#define cli() local_irq_disable()
#define sti() local_irq_enable()

#define verify_area(x,y,z) (access_ok(x,y,z) ? 0 : 1)

#define printf printk

#define ASSERT(x,msg)	WARN_ON(x)

#define trace(fmt, args...) printk("%s:%d: %s: " fmt, __FILE__, __LINE__, __PRETTY_FUNCTION__, ##args)
