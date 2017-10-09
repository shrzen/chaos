#include <stdio.h>
#include <chaos/user.h>
#include <hosttab.h>

/*
 * Initialize the the NCP with my name and number.
 */
main()
{
	char *name;
	int addr;
	int fd;

	if ((name = host_me()) == NULL ||
	    (addr = chaos_addr(name)) == 0 ||
	    (fd = open(CHURFCDEV, 0)) < 0 ||
	    ioctl(fd, CHIOCADDR, addr) < 0 ||
	    ioctl(fd, CHIOCNAME, name) < 0) {
		fprintf(stderr, "CHAOS NET CAN'T BE INITIALIZED!\n");
		fprintf(stderr, "I DON'T KNOW WHO I AM. Fix host table.");
		exit(1);
	}
	fprintf(stderr, "Chaos NCP started. I am '%s' at 0%o\n", name, addr);
	exit(0);
}
