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

FILE* file;

struct sembuf scanned[1] = {{0, -1, 0}};
struct sembuf sended[1] = {{1, 1, 0}};

void handler(int sig)
{
	printf("handler\n");
	kill(data->client, SIGINT);
	shmdt(data);
	semctl(semofID, NUMSEMS, IPC_RMID);
	shmctl(sheMemID, IPC_RMID, NULL);
	exit(0);
	signal(SIGINT, handler);
}

int main(int argc, char** argv)
{
	if(argc != 2)
	{
		printf("Wrong arg\n");
		return 5;
	}
	key = ftok("/bin/ls", '1');
	if(key == -1)
	{
		perror("Key gen failture");
		return 1;
	}
	sheMemID = shmget(key, sizeof(struct memory), IPC_CREAT | IPC_EXCL | 0600);
	if(sheMemID == -1)
	{
		perror("Memory alloc failture");
		return 2;
	}
	semofID = semget(key, NUMSEMS, IPC_CREAT | IPC_EXCL | 0600);
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
	
	signal(SIGINT, handler);
	file = fopen(argv[1], "r");
	if(file == NULL)
	{
		printf("GET REKT\n");
		return 6;
	}
	
	int num = 1;
	data->server = getpid();
	while(2 * 2 == 4)
	{
		int c;
		int i;
		if(num % 2 != 0)
		{
			i = 0;
			while(1)
			{
				c = fgetc(file);
				if((char) c == '\n' || (char) c == '\0')
				{
					data->buffer[i] = '\0';
					++i;
				}
				if((char) c == EOF)
				{
					data->buffer[i] = '\0';
					++i;
				}
				if((char) c != '\n' && (char) c != EOF && (char) c != '\0')
				{
					data->buffer[i] = (char)c;
					++i;
				}
				if(i == BUFFSIZE || c == '\n' || c == '\0' || c == EOF)
				{
					semop(semofID, sended, 1);
					semop(semofID, scanned, 1);
					if(c == EOF)
					{
						printf("EOF is here\n");
						handler(SIGINT);
					}
					else
					{
						if(c == '\n' || c == '\0')
						{
							printf("ya pidor\n");
							++num;
							break;
						}
						else
							i = 0;
					}
				}
			}
		}
		else
		{
			c = fgetc(file);
			while(c != EOF && c != '\n' && c != '\0')
			{
				c = fgetc(file);
			}
			if(c == EOF)
			{
				printf("EOF N2\n");
				handler(SIGINT);
			}
			else
				++num;
		}
	}
	return 0;
}
