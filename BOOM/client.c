#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#include <netinet/ip.h>
#include <netinet/in.h>

#define MAGIC_CONST 5

char* hostName;
int portNum = 0;
int socketID = 0;

struct hostent *hst;
struct sockaddr_in server_addr;


void getMessage()
{
    int strings = 0;
    int size = 0;
    int i = 0;
    char* msg = NULL;
    recv(socketID, &strings, sizeof(int), 0);
    for(i = 0; i < strings; ++i)
    {
        recv(socketID, &size, sizeof(int), 0);
        msg = malloc(size);
        recv(socketID, msg, size, 0);
        printf("%s", msg);
        free(msg);
    }
}

void sendMessage()
{
    int size = 0;
    char* msg = NULL;
    char c;
    scanf("%c", &c);
    while (c != '\n' && c != '\0')
    {
        msg = realloc(msg, size + 1);
        msg[size] = c;
        scanf("%c", &c);
        ++size;
    }
    msg = realloc(msg, size + 1);
    msg[size] = '\0';
    ++size;
    send(socketID, &size, sizeof(int), 0);
    send(socketID, msg, size, 0);
    free(msg);
}

int main(int argc, char* argv[])
{
    int opt = 0, port = 0;
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
    printf("before while\n");
    while(1)
    {
        getMessage();
        sendMessage();
    }
    
    return 0;
}
