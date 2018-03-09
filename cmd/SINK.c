#include <chaos.h>

struct chpacket pkt;

int
main (int argc, char **argv)
{
	chsetmode(0, CHRECORD);
	while (read(0, (char *)&pkt, sizeof(pkt)) > 0)
		;
}
