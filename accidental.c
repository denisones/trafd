
/*-----------------------------------------------------------
 *      Daemon avaryinoi zagruzki ./trafd
 *      09/04/08
 *
 *
 *-----------------------------------------------------------
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <sys/types.h>
#include "command.h"

struct sockaddr_in sa;
int soc, sc, pid = 0;
char cm_[3];
fd_set fds;
NOT_AVARIYA n_av;

int sockfd;
struct sockaddr_in sine;
COMMAND *com;
FILE *flow;
char buf[256];

void delsp(char *ch)
{
	char *src = ch;
	char *dst = ch;
	char c, lastc = '*';
	while ((c = *src) != 0)
	{
		if ((c != ' ') || (lastc != ' '))
		{
			*dst = c;
			dst++;
			lastc = c;
		}
		src++;
	}
	*dst = 0;
}

int main()
{
	char s1[3], s2[3], s3[16], name_[10];

	if (fork())
	{
		return 0;
	}

	if (setsid() < 0)
	{
		perror("cannot disassociate from controlling TTY");
		exit(1);
	}

	while (1)
	{
		if ((flow = popen("ps ax | grep trafd | grep -v grep", "r")) != NULL)
		{
			if (fgets(buf, sizeof(buf), flow) != NULL)
			{
				delsp(buf);
				sscanf(buf, "%i %s %s %s %s", &pid, &s1, &s2, &s3, &name_);
				if (strcmp(name_, "./trafd") == 0)
				{
					// printf("Prilozgenie %s rabotaet! PID: %i \n",name_,pid);
				}
				else
				{
					// printf("AVARIYA!!! \n");
				}
			}
			else
			{
				system("./trafd restart");
			}
		}
		pclose(flow);
		sleep(1);
	}
	return 0;
}
