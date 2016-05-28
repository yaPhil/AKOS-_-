#define _XOPEN_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include<unistd.h>

#define REQUEST 1
#define RESPONSE 2

typedef struct {
    long mes_type;
    int a, b;
    long result;
    pid_t client;
} mes_t;

key_t key;
int msgid;
mes_t data;

void handler(int sig)
{
    if (sig == SIGINT)
    {
        exit(0);
    }
    signal(SIGINT, handler);
}

int myPid;

int main()
{
	int a, b;
    key = ftok("/bin/ls", '1');
    msgid = msgget(key, 0);

    if (msgid == -1) 
	{
        printf("msgget failed\n");
        return 1;
    }

    signal(SIGINT, handler);
	myPid = getpid();
	data.client = myPid;
	while(2 * 2 == 4)
	{
		scanf("%d%d", &a, &b);
		data.a = a;
		data.b = b;
		data.mes_type = REQUEST;
		msgsnd(msgid, &data, sizeof(mes_t) - sizeof(long), 0);
		msgrcv(msgid, &data, sizeof(mes_t) - sizeof(long), 2 + myPid, 0);
		printf("Result: %ld", data.result);
	}
	return 0;
}
