#include "chuser.h"
#include "chclock.h"
#include "chsend.h"
#include "chrcv.h"
#include "chutil.h"
#include "chtime.h"
#include "chether.h"

int ch_size(char *p);
void ch_free(char *p);

#define cli() local_irq_disable()
#define sti() local_irq_enable()

#define verify_area(x,y,z) (access_ok(x,y,z) ? 0 : 1)
