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
	/*kill(data.client, SIGINT);*/

	data.mes_type = RESPONSE;
	data.buffer[0] = '.';
	data.buffer[1] = '\0';
	msgsnd(messID, &data, sizeof(struct memory) - sizeof(long), 0);

	msgctl(messID, IPC_RMID, NULL);
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
	messID = msgget(key, IPC_CREAT | IPC_EXCL | 0600);
	if (messID == -1) 
	{
		printf("msgget failed\n");
		return 1;
	}
	
	signal(SIGINT, handler);
	data.server = getpid();
	int i;
	int c;
	i = 0;
	pid_t first_client;
	msgrcv(messID, &data, sizeof(struct memory) - sizeof(long), REQUEST, 0);
	first_client = data.client;
	while(2 * 2 == 4)
	{
		c = fgetc(stdin);
		if((char) c == '\n' || (char) c == '\0')
		{
			data.buffer[i] = '\0';
			++i;
		}
		if((char) c == EOF)
		{
			data.buffer[i] = '\0';
			++i;
		}
		if((char) c != '\n' && (char) c != EOF && (char) c != '\0')
		{
			data.buffer[i] = (char)c;
			++i;
		}
		if(i == BUFFSIZE || c == '\n' || c == '\0' || c == EOF)
		{
				
			data.mes_type = RESPONSE;
			data.wrong = 0;
			msgsnd(messID, &data, sizeof(struct memory) - sizeof(long), 0);

			msgrcv(messID, &data, sizeof(struct memory) - sizeof(long), REQUEST, 0);
			while(data.client != first_client)
			{
				data.mes_type = RESPONSE;
				data.wrong = 1;
				msgsnd(messID, &data, sizeof(struct memory) - sizeof(long), 0);

				msgrcv(messID, &data, sizeof(struct memory) - sizeof(long), REQUEST, 0);
			}
			if(c == EOF)
			{
				handler(SIGINT);
			}
			else
			{
				i = 0;
			}
		}
	}
	return 0;
}
