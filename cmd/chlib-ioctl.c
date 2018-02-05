#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/un.h>

#include <netinet/in.h>

#include <chaos.h>

int
chopen(int address, char *contact, int mode, int async, char *data, int dlength, int rwsize)
{
	struct chopen rfc;
	int f, connfd = -1;
	
	rfc.co_host = address;
	rfc.co_contact = contact;
	rfc.co_data = data;
	rfc.co_length = data ? (dlength ? dlength : strlen(data)) : 0;
	rfc.co_clength = strlen(contact);
	rfc.co_async = async;
	rfc.co_rwsize = rwsize;
	f = open(CHAOSDEV, mode);
#if defined(BSD42) || defined(linux)
	if (f >= 0) {
		connfd = ioctl(f, CHIOCOPEN, &rfc);
	}
	close(f);
#else
	if (f >= 0 && ioctl(f, CHIOCOPEN, &rfc))
		close(f);
	else
		connfd = f;
#endif
	return connfd;
}

int
chlisten(char *contact, int mode, int async, int rwsize)
{
	return chopen(0, contact, mode, async, 0, 0, rwsize);
}

int
chreject(int fd, char *string)
{
	struct chreject chr;
	
	if (string==0||strlen(string)==0)
		string = "No Reason Given";
	chr.cr_reason = string;
	chr.cr_length = strlen(string);
	return ioctl(fd, CHIOCREJECT, &chr);
}

int
chstatus(int fd, struct chstatus *chst)
{
	return ioctl(fd, CHIOCGSTAT, (char *)&chst);
}

int
chsetmode(int fd, int mode)
{
	return ioctl(fd, CHIOCSMODE, mode);
}

int
chwaitfornotstate(int fd, int state)
{
	return ioctl(fd, CHIOCSWAIT, state);
}
