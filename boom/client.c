#define _XOPEN_SOURCE 500
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <locale.h>
#include <termios.h>
#include <arpa/inet.h>

#include <netinet/ip.h>
#include <netinet/in.h>

#define MAGIC_CONST 5

extern char *optarg;
extern int optind;

const int KEY_LEFT = 4479771;
const int KEY_UP = 4283163;
const int KEY_RIGHT = 4414235;
const int KEY_DOWN = 4348699;


char* hostName;
int portNum = 0;
int socketID = 0;
double stepDelay = 0;
int cmd = 0;

pthread_t reciver;

struct hostent *hst;
struct sockaddr_in server_addr;
struct termios oldTermParam;

void setNoncononical()
{
	int err;
	struct termios newTermParam = oldTermParam;
	newTermParam.c_lflag &= ~(ICANON);
	newTermParam.c_lflag &= ~(ECHO);
	newTermParam.c_cc[VMIN] = 1;
	/*printf("noncocnonical%d\n", newTermParam.c_lflag);*/
	err = tcsetattr(0, TCSANOW, &newTermParam);
}

void setCononical()
{
	int err = tcsetattr(0, TCSANOW, &oldTermParam);
	/*printf("cononical%d\n", oldTermParam.c_lflag);*/
}

void getMessage()
{
    printf("-------------reciving message\n");
    int strings = 0;
    int size = 0;
    int i = 0;
    char* msg = NULL;
    recv(socketID, &strings, sizeof(int), 0);
    for(i = 0; i < strings; ++i)
    {
        recv(socketID, &size, sizeof(int), 0);
        msg = malloc(size + 2);
        recv(socketID, msg, size, 0);
        printf("%s", msg);
        free(msg);
    }
    printf("-------------haave  recieved sfsdsg\n");
}

void getMap()
{
    int strings = 0, size = 0;
    int i = 0, j = 0;
    recv(socketID, &strings, sizeof(int), 0);
    recv(socketID, &size, sizeof(int), 0);
    for(i = 0; i < strings; ++i)
    {
        for(j = 0; j < size; ++j)
        {
            char c;
            recv(socketID, &c, sizeof(char), 0);
            printf("%c", c);
        }
        printf("\n");
    }
}

void sendMessage()
{
    int size = 0;
    char* msg = NULL;
    char c = ' ';
    printf("------------sending mess\n");
    c = fgetc(stdin);
    while (c != '\n' && c != '\0')
    {
        msg = realloc(msg, size + 1);
        msg[size] = c;
        c = fgetc(stdin);
        ++size;
    }
    msg = realloc(msg, size + 1);
    msg[size] = '\0';
    ++size;
    send(socketID, &size, sizeof(int), 0);
    send(socketID, msg, size, 0);
    printf("-------%d------- %s\n",size, msg);
    free(msg);
    printf("--------------have sent alalalal\n");
}

void* fthread(void* arg)
{
    int tmp = 0;
    while(1)
    {
        usleep(1000000 * stepDelay);
        send(socketID, &tmp, sizeof(int), 0);
        getMap();
    }
    return NULL;
}

int main(int argc, char* argv[])
{
    int opt = 0, port = 0, type = 0;
    while((opt = getopt(argc, argv, "s:p:")) != -1)
    {
        switch (opt)
        {
            case 's':
                hostName = optarg;
                break;
            case 'p':
                port = 1;
                portNum = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Bad arguments\n");
                exit(1);
        }
    }
    if(optind > argc)
    {
        fprintf(stderr, "Miss argument for option\n");
        exit(2);
    }
    
    if(port == 0)
        portNum = 8000;
    
    socketID = socket(PF_INET, SOCK_STREAM, 0);
    hst = gethostbyname(hostName);
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons((short)portNum);
    /*           */
    server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    /*            */
    /*memcpy(&(server_addr.sin_addr.s_addr), hst->h_addr_list[0], hst->h_length);*/
    
    if (connect(socketID, (struct sockaddr*)(&server_addr), sizeof(struct sockaddr_in)) == -1)
    {
        printf("connect fail\n");
        return 5;
    }
    recv(socketID, &type, sizeof(int), 0);
    if(type == 2)
    {
        printf("before while\n");
        while(1)
        {
            getMessage();
            sendMessage();
        }
    }
    else
    {
        int message = 0;
        if(type == 3)
        {
            printf("Sorry, wait ending of the game\n");
        }
        recv(socketID, &stepDelay, sizeof(double), 0);
        
        getMessage();
        sendMessage();
        getMessage();
        printf("Game starts\n");
        tcgetattr(0, &oldTermParam);
        setNoncononical();
        pthread_create(&reciver, NULL, fthread, NULL);
        
        while(2 * 2 == 4)
        {
            cmd = 0;
            read(0, &cmd, 4);
            send(socketID, &cmd, sizeof(int), 0);
        }
    }
    return 0;
}
