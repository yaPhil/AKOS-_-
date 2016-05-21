#define _POSIX_C_SOURCE 199309L
#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>
#include<sys/types.h>

volatile int cnt = 0;

void handler(int signum, siginfo_t *info, void* ptr)
{
	if(cnt < 10)
	{
		++cnt;
		printf("Signal: %d Pid: %d\n", signum, (int)(info->si_pid));
	}
	else
	{
		printf("More than 10 signals\n");
		exit(0);
	}
}

int main()
{
	struct sigaction act;
	int i;
	sigaction(SIGINT, NULL, &act);
	act.sa_flags = SA_SIGINFO;
	act.sa_handler = NULL;
	act.sa_sigaction = handler;

	sigaction(SIGINT, &act, NULL);
	printf("My pid %d\n", (int)getpid());
	for(i = 1; i < 32; ++i)
	{
		if(i != SIGKILL && i != SIGSTOP)
			sigaction(i, &act, NULL);
	}
	while(2 * 2 == 4)
	{
	}
	return 0;
}

