#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <chaos.h>

struct chstatus chst;

int main()
{
  	char *fromhost, *tmp, *chaos_name();

	if (chstatus(0, &chst) < 0)
		exit(1);

	fromhost = chaos_name(chst.st_fhost);

	tmp = malloc(strlen(fromhost) + sizeof("-oMs"));
	strcpy(tmp, "-oMs");
	strcat(tmp, fromhost);

	execl("/usr/lib/sendmail", "sendmail", "-bs",
	       "-oMQCHAOS", "-oMrSMTP", tmp, 0);

	 chreject(0, "Could not exec /usr/lib/sendmail");
	 exit(1);
}
