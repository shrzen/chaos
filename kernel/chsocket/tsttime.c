#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sys/poll.h>
#include <sys/uio.h>

int verbose;
u_char buffer[4096];
u_char *msg, resp[8];
int fd;

void node_demux(unsigned long id);

void
send_chaos(void)
{
    u_short b[64];
    u_char lenbytes[4];
    struct iovec iov[2];
    int wsize, plen, ret;

    wsize = 0;
    memset((char *)b, 0, sizeof(b));

    printf("sending fake time packet\n");
    b[wsize++] = htons(5); /* op = ANS */
    b[wsize++] = htons(16+8); /* count*/
    b[wsize++] = 0;
    b[wsize++] = 0;
    b[wsize++] = 0;
    b[wsize++] = 0;
    b[wsize++] = 0;
    b[wsize++] = 0;
    b[wsize++] = htons(('T' << 8) | 'I');
    b[wsize++] = htons(('M' << 8) | 'E');
    b[wsize++] = 0;
    b[wsize++] = 0;

    b[wsize++] = htons(0401); /* dest */
    b[wsize++] = 0; /* source */
    b[wsize++] = 0; /* checksum */


    plen = wsize*2;

    lenbytes[0] = plen >> 8;
    lenbytes[1] = plen;
    lenbytes[2] = 1;
    lenbytes[3] = 0;

    iov[0].iov_base = lenbytes;
    iov[0].iov_len = 4;

    iov[1].iov_base = b;
    iov[1].iov_len = wsize*2;

    ret = writev(fd, iov, 2);
    if (ret <  0) {
        perror("writev");
    }
}

int main()
{
    int waiting;

    if ((fd = connect_to_server()) != -1) {
        exit(1);
    }

    while (1) {
        sleep(1);
        send_chaos();
    }

    exit(0);
}
