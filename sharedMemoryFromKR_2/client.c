#define _XOPEN_SOURCE
#include<stdlib.h>
#include<stdio.h>
#include<sys/ipc.h>
#include<sys/types.h>
#include<sys/shm.h>
#include<sys/sem.h>
#include<signal.h>
#include<unistd.h>

#define NUMSEMS 2
#define BUFFSIZE 500

int sheMemID;
int semofID;
int key;



struct memory
{
	char buffer[BUFFSIZE];
	pid_t server;
	pid_t client;
};

struct memory* data;

struct sembuf wakeServer[1] = {{0, 1, 0}};
struct sembuf waiting[1] = {{1, -1, 0}};

char* string;

void handler(int sig)
{
	pid_t pid = data->server;
	shmdt(data);
	kill(pid, SIGINT);
	exit(1);
}

void handler2(int sig)
{
	shmdt(data);
	exit(2);
}

int main()
{
	int i = 0;
	int size = 1000;
	signal(SIGALRM, handler);
	signal(SIGINT, handler2);
	alarm(30);
	key = ftok("/bin/ls", '1');
	if(key == -1)
	{
		perror("Key gen failture");
		return 1;
	}
	sheMemID = shmget(key, sizeof(struct memory), 0);
	if(sheMemID == -1)
	{
		perror("Memory alloc failture");
		return 2;
	}
	semofID = semget(key, NUMSEMS, 0);
	if(semofID == -1)
	{
		perror("Semofor failture");
		return 3;
	}
	data = (struct memory*)shmat(sheMemID, NULL, 0);
	if(data == (struct memory*)(-1))
	{
		perror("Shmat failture");
		return 4;
	}
	data->client = getpid();
	string = malloc(1000);
	semop(semofID, waiting, 1);
	i = 0;
	while(2 * 2 == 4)
	{
		int j = 0;
		while(data->buffer[j] != '\0' && j < BUFFSIZE)
		{
			string[i] = data->buffer[j];
			++j;
			++i;
			if(i == size)
			{
				string = realloc(string, size + BUFFSIZE);
			}
		}
		if(data->buffer[j] == '\0')
		{
			int flag = 0;
			string[i] = '\0';
			i = 0;
			while(string[i] != '\0')
			{
				if(string[i] == '#')
				{
					flag = 1;
					break;
				}
				else
					++i;
			}
			if(flag == 1)
			{
				printf("%s\n", string);
			}
			i = 0;
		}
		semop(semofID, wakeServer, 1);
		semop(semofID, waiting, 1);
	}
	return 0;
}
