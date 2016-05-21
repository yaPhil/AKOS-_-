/*распечатать всю иерархию процессов с отца*/

#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>

volatile pid_t p0, t1, t2, t3, t4, id;
volatile int pipeFd[2];
volatile int success = 0;

void handler1(int sig);
void handler2(int sig);
void handler3(int sig);

void successHandler(int sig) // для прерывания цикла
{
	if(sig == SIGUSR2) 
	{
		success = 1;
		signal(SIGUSR2, successHandler);
	}
	return;
}

void handler1(int sig)
{
	if(sig == SIGUSR1)
	{
		read(pipeFd[0], &t2, sizeof(pid_t));
		read(pipeFd[0], &t3, sizeof(pid_t));
		print_info(t1);
		kill(p0, SIGUSR2); // кидаем сигнал в отца
		close(pipeFd[0]);
		close(pipeFd[1]);
		exit(0);
	}
}

void handler2(int sig)
{
	if(sig == SIGUSR2)
	{
		read(pipeFd[0], &t3, sizeof(pid_t));
		print_info(t2);
		close(pipeFd[0]);
		close(pipeFd[1]);
		exit(0);
	}
}

void print_info(pid_t pid)
{
	if(pid == p0)
	{
		printf("*p0:%lu\n", (unsigned long long)pid);
	}
	else
	{
		printf("p0:%lu\n", (unsigned long long)p0);
	}
	if(pid == t1)
	{
		printf("*t1:%lu\n", (unsigned long long)pid);
	}
	else
	{
		printf("t1:%lu\n", (unsigned long long)t1);
	}
	if(pid == t2)
	{
		printf("*t2:%lu\n", (unsigned long long)pid);
	}
	else
	{
		printf("t2:%lu\n", (unsigned long long)t2);
	}
	if(pid == t3)
	{
		printf("*t3:%lu\n", (unsigned long long)pid);
	}
	else
	{
		printf("t3:%lu\n", (unsigned long long)t3);
	}
}

int main()
{
	p0 = getpid();
	pipe(pipeFd);
	t1 = fork();
	if(t1 == 0) //esli 1 syn
	{
		t1 = getpid();
		signal(SIGUSR1, &handler1);
		while(2 * 2 == 4)  //zdem signal
		{
			pause();
		}
		return 0;
	}

	t2 = fork();
	if(t2 == 0) //esli 2 syn
	{
		t2 = getpid();
		signal(SIGUSR1, handler2);
		while(2 * 2 == 4)  //zdem signal
		{
			pause();
		}
		return 0;
	}

	t3 = fork();
	if(t3 == 0) //esli 3 syn
	{
		print_info(t3);
		return 0;
	}


	write(pipeFd[1], &t2, sizeof(pid_t)); //из отца добавляем пиды сыновей в пайп для 1 сына
	write(pipeFd[1], &t3, sizeof(pid_t));
	signal(SIGUSR2, successHandler);
	kill(t1, SIGUSR1);
	while(!success) // ждем сигнала от 1 сына
	{
		pause();
	}

	write(pipeFd[1], &t3, sizeof(pid_t)); // для 2 сына
	kill(t2, SIGUSR1);
	
	wait(NULL);
	wait(NULL);
	wait(NULL);
	return 0;
}
