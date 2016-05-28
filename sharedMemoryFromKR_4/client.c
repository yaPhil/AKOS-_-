#define _XOPEN_SOURCE
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<sys/ipc.h>
#include<sys/types.h>
#include<sys/shm.h>
#include<sys/sem.h>
#include<signal.h>
#include<unistd.h>

#define NUMSEMS 3
#define BUFFSIZE 500

int sheMemID;
int semofID;
int key;

struct memory
{
    char buffer[BUFFSIZE];
    int checked;
    pid_t client;
};

struct memory **data;

struct sembuf send[1] = {{0, 1, 0}};
struct sembuf wait_1[1] = {{1, -1, 0}};
struct sembuf wait_2[1] = {{2, -1, 0}};

int main(int argc, char** argv)
{
    key = ftok("/bin/ls", '1');
    if(key == -1)
    {
        perror("Key gen failture");
        return 1;
    }
    sheMemID = shmget(key, 2 * sizeof(struct memory), IPC_CREAT | IPC_EXCL | 0600);
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

    data = (struct memory**)shmat(sheMemID, NULL, 0);
    if(data == (struct memory**)(-1))
    {
        perror("Shmat failture");
        return 4;
    }
    if(data[0]->client == 0)
    {
        data[0]->client = getpid();
    }
    else
    {
        if(data[1]->client == 0)
            data[1]->client = getpid();
    }
    pid_t myPid = getpid();
    FILE* input = fopen(argv[1], "r");
    while(1)
    {
        if(data[0]->client == myPid)
        {
            fscanf(input, "%s", data[0]->buffer);
            data[0]->checked = 1;
            semop(semofID, send, 1);
            semop(semofID, wait_1, 1);
        }
        else
        {
            fscanf(input, "%s", data[1]->buffer);
            data[1]->checked = 1;
            semop(semofID, send, 1);
            semop(semofID, wait_2, 1);
        }
    }
    return 0;
}
