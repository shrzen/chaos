#include <linux/sched.h>

#include "chncp/chuser.h"
#include "chncp/chclock.h"
#include "chncp/chsend.h"
#include "chncp/chrcv.h"
#include "chncp/chutil.h"
#include "chunix/chtime.h"
#include "chunix/challoc.h"
#include "chether.h"

int ch_size(char *p);
void ch_free(char *p);

#define cli() local_irq_disable()
#define sti() local_irq_enable()

#define verify_area(x,y,z) (access_ok(x,y,z) ? 0 : 1)
