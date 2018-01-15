#include <chaos.h>

struct chpacket pkt;
main()
{
	chsetmode(0, CHRECORD);
	while (read(0, (char *)&pkt, sizeof(pkt)) > 0)
		;
}
