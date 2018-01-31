// cheaddr -- set Chaos address on an interface

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/ioctl.h>

#include <chaos.h>

int
main(int argc, char **argv)
{
	int i;

	if (argc < 2) {
		printf("usage: %s <if-name> <node-number>\n", argv[0]);
		exit(1);
	}

#if NCHETHER
	struct chether che;

	strncpy(che.ce_name, argv[1], CHIFNAMSIZ);
	sscanf(argv[2], "%o", &i);
	che.ce_addr = i;

	printf("Setting device %s to chaos address 0%o\n", che.ce_name, che.ce_addr);
	if ((i = open(CHURFCDEV, 0)) < 0 || ioctl(i, CHIOCETHER, &che) < 0)
		printf("Can't set chaos address on ethernet: %d\n", errno);
#elif NCHIL
	struct chiladdr ca;

	ca.cil_device = atoi(argv[1]);
	sscanf(argv[2], "%o", &i);
	ca.cil_address = i;

	printf("Setting device %d to chaos address 0%o\n", ca.cil_device, ca.cil_address);
	if ((i = open(CHURFCDEV, 0)) < 0 || ioctl(i, CHIOCILADDR, &ca) < 0)
		printf("Can't set chaos address on ethernet: %d\n", errno);
#endif

	exit(0);
}
