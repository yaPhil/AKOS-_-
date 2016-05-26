#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#include <netinet/ip.h>
#include <netinet/in.h>

#define MAGIC_CONST 5

char*  scanString(FILE* file, int* err, int* size)
{
    size_t bufSize = (size_t)sysconf(_SC_PAGESIZE);
    size_t numSym = 0;
    size_t i = 0;
    char* string = NULL;
    char* buffer = NULL;
    if(file == NULL)
    {
        *err = 4;
        *size = 0;
        return NULL;
    }
    string = malloc(bufSize);
    if(string == NULL)
    {
        *err = 3;
        *size = 0;
        return NULL;
    }
    buffer = malloc(bufSize);
    if(buffer == NULL)
    {
        *err = 3;
        free(string);
        return NULL;
    }
    while(2*2 == 4)
    {
        int c = fgetc(file);

        if((char)c != '\n' && (char)c != '\0' && (char)c != EOF)
        {
            buffer[i] = c;
            ++i;
            if(i == bufSize)
            {
                char* tmp = NULL;
                memmove(string + numSym, buffer, bufSize);
                i = 0;
                numSym += bufSize;
                tmp = realloc(string, numSym + bufSize);
                if(tmp == NULL)
                {
                    *err = 3;
                    break;
                }
                else
                    string = tmp;
            }
        }
        else
        {
            memmove(string + numSym, buffer, i);
            numSym += i;
            string[numSym] = '\0';
            if((char)c == '\n')
            {
                *err = 0;
            }
            if((char)c == '\0')
            {
                *err = 1;
            }
            if((char)c == EOF)
            {
                *err = 2;
            }
            break;
        }
    }
    free(buffer);
    *size = numSym;
    return string;
}

char** scanFile(FILE* file, int* err, int* size)
{
    char** result = NULL;
    int numStr = 0;
    if(file == NULL)
    {
        *size = 0;
        *err = 4;
        return NULL;
    }
    while(2 * 2 == 4)
    {
        int errStr, sizeStr;
        char** tmp = NULL;
        char* str = scanString(file, &errStr, &sizeStr);
        if(errStr == 3)
        {
            *err = errStr;
            *size = numStr;
            return result;
        }
        if(errStr == 5)
        {
            *err = errStr;
            *size = numStr;
            return result;
        }
        if(errStr == 2 && sizeStr == 0)
        {
            free(str);
            break;
        }
        tmp = realloc(result, (numStr + 1) * sizeof(char*));
        if(tmp == NULL)
        {
            *err = 3;
            *size = numStr;
            return result;
        }
        result = tmp;
        result[numStr] = str;
        ++numStr;
        if(errStr == 2)
            break;
    }
    *err = 0;
    *size = numStr;
    return result;
}

struct item
{
    int row;
    int column;
    int effect;
};

int mapRows = 0;
int mapColumns = 0;
int initialHealth = 0;
int hitValue = 0, reloadTime = 0, mineTime = 0, stayDrop = 0, moveDrop = 0, immortalTime = 0;
double stepDelay = 0;

char** fileMap = NULL;
char** justMap = NULL;

struct item* items = NULL;
int numItems = 0;

int socketID;
int bindReturn;
int acceRet;
struct sockaddr_in server_addr;

pthread_t* arrThread;
int playerNumber;

int gameStart;
int isGameStart;
int creatorNumber = -1;

struct member
{
	int fd;
	int pos_i;
	int pos_j;
	char* name;
	int health;
	int shot;
	int mine;
	int number;
	int role;
	int active;
};

struct member* data;

void handler(int sig)
{
    int i;
    close(socketID);
    for(i = 0; i < playerNumber; ++i)
    {
        pthread_cancel(arrThread[i]);
    }
    for(i = 0; i < playerNumber; ++i)
    {
        pthread_join(arrThread[i], NULL);
    }
    exit(0);
}

void* fthread(void* arg)
{
	struct member* person = (struct member*)arg;
	if(creatorNumber == -1)
	{
        int size = 33;
        char answer[2];
        int strings = 1;
        creatorNumber = person->number;
        send(person->fd, &strings, sizeof(int), 0);
        send(person->fd, &size, sizeof(int), 0);
		send(person->fd, "You are creator\nEnter your name\n\0", size, 0);
        recv(person->fd, &size, sizeof(int), 0);
        person->name = malloc(size);
        person->active = 1;
        person->role = 1;
        recv(person->fd, person->name, size, 0);
        printf("%s\n", person->name);
        size = 49;
        send(person->fd, &strings, sizeof(int), 0);
        send(person->fd, &size, sizeof(int), 0);
        send(person->fd, "Input 1 to start game; 2 to see list of players\n\0", size, 0);
        answer[0] = '2';
        while(answer[0] == '2')
        {
            recv(person->fd, &size, sizeof(int), 0);
            recv(person->fd, &answer, size, 0);
            if(answer[0] == '2')
            {
                int i = 0;
                int cnt = 0;
                int sz = 0;
                for(i = 0; i < playerNumber; ++i)
                {
                    if(data[i].active == 1)
                    {
                        ++cnt;
                    }
                }
                cnt += 2;
                send(person->fd, &cnt, sizeof(int), 0);
                sz = 14;
                send(person->fd, &sz, sizeof(int), 0);
                send(person->fd, "Players are:\n\0", sz, 0);
                for(i = 0; i < playerNumber; ++i)
                {
                    if(data[i].active == 1)
                    {
                        if(data[i].role == 1)
                        {
                            sz = strlen(data[i].name) + 14;
                            send(person->fd, &sz, sizeof(int), 0);
                            send(person->fd, data[i].name, strlen(data[i].name), 0);
                            send(person->fd, " are creator\n\0", 14, 0);
                        }
                        else
                        {
                            sz = strlen(data[i].name) + 13;
                            send(person->fd, &sz, sizeof(int), 0);
                            send(person->fd, data[i].name, strlen(data[i].name), 0);
                            send(person->fd, " are player\n\0", 13, 0);
                        }
                    }
                }
                sz = 49;
                send(person->fd, &sz, sizeof(int), 0);
                send(person->fd, "Input 1 to start game; 2 to see list of players\n\0", sz, 0);
            }
            else
            {
                
            }
        }
	}
    else
    {
        int size = 41;
        char answer[2];
        int strings = 1;
        send(person->fd, &strings, sizeof(int), 0);
        send(person->fd, &size, sizeof(int), 0);
        send(person->fd, "You are player\nEnter your name and wait\n\0", size, 0);
        recv(person->fd, &size, sizeof(int), 0);
        person->name = malloc(size);
        person->active = 1;
        person->role = 0;
        recv(person->fd, person->name, size, 0);
        printf("%s\n", person->name);
    }
}

int main(int argc, char* argv[])
{
    int opt = 0, map = 0, port = 0, log = 0, portNum = 0;
    char* mapFile = NULL;
    char* logFile = NULL;
    FILE* mapIn = NULL;
    FILE* logOut = NULL;
    FILE* mapOut = NULL;
    int mapErr = 0, mapSize = 0;
    int i = 0;
    int j = 0;
    while((opt = getopt(argc, argv, "m:p:l:")) != -1)
    {
        switch (opt)
        {
            case 'm':
                map = 1;
                mapFile = optarg;
                break;
            case 'p':
                port = 1;
                portNum = atoi(optarg);
                break;
            case 'l':
                log = 1;
                logFile = optarg;
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

    if(map == 0)
    {
        mapFile = malloc(8);
        mapFile = "mapFile\0";
    }
    if(log == 0)
    {
        logFile = malloc(10);
        logFile = "serverPid\0";
    }
    if(port == 0)
        portNum = 8000;

    mapIn = fopen(mapFile, "r");
    logOut = fopen(logFile, "w");

    if(logOut == NULL)
    {
        fprintf(stderr, "Cant print server pid\n");
        exit(3);
    }

    fprintf(logOut, "%d", getpid());
    fclose(logOut);
    fileMap = scanFile(mapIn, &mapErr, &mapSize);
    if(mapErr != 0)
    {
        fprintf(stderr, "Bad map file\n");
        exit(mapErr);
    }
    fclose(mapIn);
    mapOut = fopen("justMap.out", "w");
    if(mapOut == NULL)
    {
        fprintf(stderr, "Cant print map\n");
        exit(3);
    }
    i = 0;
    while(i < mapSize)
    {
        j = 0;

        while(j < strlen(fileMap[i]))
        {
            if(fileMap[i][j] >= '0' && fileMap[i][j] <= '9' && mapRows == 0 && mapColumns == 0)
            {
                while(fileMap[i][j] >= '0' && fileMap[i][j] <= '9')
                {
                    mapRows *= 10;
                    mapRows += fileMap[i][j] - '0';
                    ++j;
                }
                ++j;
                while(fileMap[i][j] >= '0' && fileMap[i][j] <= '9')
                {
                    mapColumns *= 10;
                    mapColumns += fileMap[i][j] - '0';
                    ++j;
                }
            }
            if(fileMap[i][j] == '#')
            {
                int k = 0;
                int m = 0;
				int st = j;
                for(k = 0; k < mapRows + 2; ++k)
                {
                    for(m = 0; m < mapColumns + 2; ++m)
					{
                        fprintf(mapOut, "%c", fileMap[i][j]);
						++j;
					}
					++i;
					j = st;
                    fprintf(mapOut, "\n");
                }
            }
            if(fileMap[i][j] >= '0' && fileMap[i][j] <= '9' && initialHealth == 0)
            {
                while(fileMap[i][j] >= '0' && fileMap[i][j] <= '9')
                {
                    initialHealth *= 10;
                    initialHealth += fileMap[i][j] - '0';
                    ++j;
                }
            }
            if(fileMap[i][j] >= '0' && fileMap[i][j] <= '9' && hitValue == 0)
            {
                while(fileMap[i][j] >= '0' && fileMap[i][j] <= '9')
                {
                    hitValue *= 10;
                    hitValue += fileMap[i][j] - '0';
                    ++j;
                }
            }
			if(fileMap[i][j] >= '0' && fileMap[i][j] <= '9' && reloadTime == 0)
			{
				while(fileMap[i][j] >= '0' && fileMap[i][j] <= '9')
				{
					reloadTime *= 10;
					reloadTime += fileMap[i][j] - '0';
					++j;
				}
			}
			if(fileMap[i][j] >= '0' && fileMap[i][j] <= '9' && mineTime == 0)
			{
				while(fileMap[i][j] >= '0' && fileMap[i][j] <= '9')
				{
					mineTime *= 10;
					mineTime += fileMap[i][j] - '0';
					++j;
				}
			}
			if(fileMap[i][j] >= '0' && fileMap[i][j] <= '9' && stayDrop == 0)
			{
				while(fileMap[i][j] >= '0' && fileMap[i][j] <= '9')
				{
					stayDrop *= 10;
					stayDrop += fileMap[i][j] - '0';
					++j;
				}
			}
			if(fileMap[i][j] >= '0' && fileMap[i][j] <= '9' && moveDrop == 0)
			{
				while(fileMap[i][j] >= '0' && fileMap[i][j] <= '9')
				{
					moveDrop *= 10;
					moveDrop += fileMap[i][j] - '0';
					++j;
				}
			}
			if(fileMap[i][j] >= '0' && fileMap[i][j] <= '9' && stepDelay == 0)
			{
				int cnt = 1;
				while(fileMap[i][j] >= '0' && fileMap[i][j] <= '9')
				{
					stepDelay *= 10;
					stepDelay += fileMap[i][j] - '0';
					++j;
				}
				++j;
				while(fileMap[i][j] >= '0' && fileMap[i][j] <= '9')
				{ 
					cnt *= 10;
					stepDelay *= 10;
					stepDelay += fileMap[i][j] - '0';
					++j;
				}
				stepDelay /= cnt;
			}
			if(fileMap[i][j] >= '0' && fileMap[i][j] <= '9' && immortalTime == 0)
			{
				while(fileMap[i][j] >= '0' && fileMap[i][j] <= '9')
				{
					immortalTime *= 10;
					immortalTime += fileMap[i][j] - '0';
					++j;
				}
			}
			if(fileMap[i][j] >= '0' && fileMap[i][j] <= '9')
			{
				struct item tmp;
                tmp.row = 0;
                tmp.column = 0;
                tmp.effect = 0;
				items = realloc(items, sizeof(struct item) * (numItems + 1));
				while(fileMap[i][j] >= '0' && fileMap[i][j] <= '9')
				{
					tmp.row *= 10;
					tmp.row += fileMap[i][j] - '0';
					++j;
				}
				while(fileMap[i][j] < '0' || fileMap[i][j] > '9')
					++j;
				while(fileMap[i][j] >= '0' && fileMap[i][j] <= '9')
				{
					tmp.column *= 10;
					tmp.column += fileMap[i][j] - '0';
					++j;
				}
				while(fileMap[i][j] < '0' || fileMap[i][j] > '9')
					++j;
				while(fileMap[i][j] >= '0' && fileMap[i][j] <= '9')
				{
					tmp.effect *= 10;
					tmp.effect += fileMap[i][j] - '0';
					++j;
				}
				items[numItems] = tmp;
				++numItems;
			}
			++j;
		}
        ++i;
    }
    
	printf("%d %d %d %d %d %d %d %d %lf  %d\n", mapRows, mapColumns, initialHealth, hitValue, reloadTime, mineTime, stayDrop, moveDrop, stepDelay, immortalTime);

    fclose(mapOut);
    mapOut = NULL;

    mapOut = fopen("justMap.out", "r+");

	justMap = scanFile(mapOut, &mapErr, &mapSize);
    printf("%d HERE %d !!!!!! %d\n", mapSize, numItems, mapErr);
/*    printf("%d %d\n", items[2].row, items[2].column);*/
	for(i = 0; i < numItems; ++i)
	{
        printf("%d %d\n",items[i].row, items[i].column );
		justMap[items[i].row][items[i].column] = 'I';
	}


	socketID = socket(PF_INET, SOCK_STREAM, 0);
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons((short)portNum);
	server_addr.sin_addr.s_addr = INADDR_ANY;

	bindReturn = bind(socketID, (struct sockaddr*)(&server_addr), sizeof(struct sockaddr_in));
	if(bindReturn == -1)
	{
		perror("Cant bind");
		return 6;
	}
	listen(socketID, MAGIC_CONST);
	signal(SIGINT, handler);
    printf("before while\n");
	while(2 * 2 == 4)
	{
		acceRet = accept(socketID, NULL, NULL);
		data = realloc(data, (playerNumber + 1) * sizeof(struct member));
		data[playerNumber].health = initialHealth;
		data[playerNumber].shot = 0;
		data[playerNumber].mine = 0;
		data[playerNumber].number = playerNumber;
        data[playerNumber].name = NULL;
		data[playerNumber].fd = acceRet;
		arrThread = realloc(arrThread, (playerNumber + 1) * sizeof(pthread_t));
		pthread_create(&arrThread[playerNumber], NULL, fthread, &data[playerNumber]);
		++playerNumber;
	}
    return 0;
}
