#define _XOPEN_SOURCE
#include<stdlib.h>
#include<stdio.h>
#include<sys/ipc.h>
#include<string.h>
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

FILE* file_1;
FILE* file_2;

struct sembuf scanned[1] = {{0, -1, 0}};
struct sembuf send_to1[1] = {{1, 1, 0}};
struct sembuf send_to2[1] = {{2, 1, 0}};

void handler(int sig)
{
    printf("handler\n");
    shmdt(data);
    semctl(semofID, NUMSEMS, IPC_RMID);
    shmctl(sheMemID, IPC_RMID, NULL);
    exit(0);
    signal(SIGINT, handler);
}

int main(int argc, char** argv)
{
    if(argc != 3)
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

    signal(SIGINT, handler);
    file_1 = fopen(argv[1], "r");
    file_2 = fopen(argv[2], "r");
    if(file_1 == NULL || file_2 == NULL)
    {
        printf("GET REKT\n");
        return 6;
    }
    data[0]->client = 0;
    data[1]->client = 0;
    pid_t one = 0;
    pid_t two = 0;
    while(1)
    {
        semop(semofID, scanned, 1);
        if(one == 0)
        {
            if(data[0]->checked != 0)
                one = data[0]->client;
            else
                one = data[1]->client;
        }
        else
        {
            if(two == 0)
            {
                if(data[0]->client != one)
                    two = data[0]->client;
                else
                    two = data[1]->client;
            }
        }
        if(data[0]->checked != 0)
        {
            data[0]->checked = 0;
            if(data[0]->client == one)
            {
                fprintf(file_1, "%s", data[0]->buffer);
            }
            else
            {
                fprintf(file_2, "%s", data[0]->buffer);
            }
            semop(semofID, send_to1, 1);
            if(data[0]->buffer[0] == EOF)
                handler(SIGINT);

        }
        else
        {
            data[1]->checked = 0;
            if(data[1]->client == one)
            {
                fprintf(file_1, "%s", data[1]->buffer);
            }
            else
            {
                fprintf(file_2, "%s", data[1]->buffer);
            }
            semop(semofID, send_to2, 1);
            if(data[1]->buffer[1] == EOF)
                handler(SIGINT);
        }
    }
    return 0;
}
