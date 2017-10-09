#include <sys/types.h>
#include <nlist.h>
#include <chaos/user.h>

#define VMUNIX

#ifdef	VMUNIX
#define	SYMFILE		"/vmunix"
#define	MEMORY		"/dev/kmem"
#else
#define SYMFILE		"/unix"
#define MEMORY		"/dev/kmem"
#endif

struct	nlist nlm[] = {
	{ "_bootime" },
	{ 0 },
};

main()
{
	time_t	now, bootime, uptime;
	int	mem;
	
	if ((mem = open(MEMORY, 0)) < 0) {
		lose("No Memory");
		exit(1);
	}
	nlist(SYMFILE, nlm);
	if (nlm[0].n_type == 0) {
		lose("No Namelist");
		exit(1);
	}
	if (nlm[0].n_type <= 0) {
		lose("No boot time");
		exit(1);
	}
	time(&now);
	lseek(mem, (long)nlm[0].n_value, 0);
	read(mem, &bootime, sizeof(bootime));
	
	/* uptime is supposed to be in 60ths of a second */

	uptime = (now - bootime) *  60L;
#if pdp11
	swaplong(&uptime);
#endif

	ioctl(1, CHIOCANSWER, 0);
	write(1, &uptime, sizeof( uptime ));
	close(0);
	close(1);
	exit(0);
}

lose (message)
char	*message;
{
	ioctl(0, CHIOCREJECT, message);
}

swaplong(ptr)
unsigned int	ptr[2];
{
	unsigned int temp;

	temp = ptr[0];
	ptr[0] = ptr[1];
	ptr[1] = temp;
}
