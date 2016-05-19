#define _XOPEN_SOURCE
#include<stdlib.h>
#include<stdio.h>
#include<sys/ipc.h>
#include<sys/types.h>
#include<sys/shm.h>
#include<sys/sem.h>
#include<sys/msg.h>
#include<signal.h>
#include<unistd.h>
#include<string.h>

#define BUFFSIZE 500
#define REQUEST 1
#define RESPONSE 2

int messID;
int key;



struct memory
{
	long mes_type;
	char buffer[BUFFSIZE];
	int wrong;
	pid_t server;
	pid_t client;
};

struct memory data;

void handler(int sig)
{
	exit(0);
	signal(SIGINT, handler);
}

int main(int argc, char** argv)
{

	key = ftok("/bin/ls", '1');
	if(key == -1)
	{
		perror("Key gen failture");
		return 1;
	}
	messID = msgget(key, 0);
	if (messID == -1) 
	{
		printf("msgget failed\n");
		return 1;
	}
	
	signal(SIGINT, handler);
	data.client = getpid();
	int i;
	int c;
	char* string = malloc(1000);
	int size = 1000;
	i = 0;
	data.mes_type = REQUEST; 
	msgsnd(messID, &data, sizeof(struct memory) - sizeof(long), 0);
	while(1)
	{
		msgrcv(messID, &data, sizeof(struct memory) - sizeof(long), RESPONSE, 0);
		if(data.wrong == 1)
		{
			break;
		}

		int j = 0;
		while(data.buffer[j] != '\0' && j < BUFFSIZE)
		{
			string[i] = data.buffer[j];
			++j;
			++i;
			if(i == size)
			{
				string = realloc(string, size + BUFFSIZE);
			}
		}
		if(data.buffer[j] == '\0')
		{
			string[i] = '\0';
			i = 0;
			if(string[0] == '.' && string[1] == '\0')
			{
				handler(SIGINT);
			}
			if(strlen(string) % 2 != 0)
			{
				printf("%s\n", string);
			}
			i = 0;
		}
		data.mes_type = REQUEST;
		data.client = getpid();
		msgsnd(messID, &data, sizeof(struct memory) - sizeof(long), 0); 
	}
	return 0;
}
