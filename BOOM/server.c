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

struct item* items = NULL;


int main(int argc, char* argv[])
{
    int opt = 0, map = 0, port = 0, log = 0, portNum = 0;
    char* mapFile = NULL;
    char* logFile = NULL;
    char** fileMap = NULL;
    char** justMap = NULL;
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
    if(optind >= argc)
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
    for(i = 0; i < mapSize; ++i)
    {
        j = 0;

        while(j < strlen(fileMap[i]))
        {
            if(fileMap[i][j] >= '0' && fileMap[i][j] <= '9' && mapRows == 0 && mapColumns = 0)
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
                for(k = 0; k < mapRows + 2; ++k)
                {
                    for(m = 0; m < mapColumns + 2; ++m)
                        fprintf(mapOut, "%c", fileMap[i][j]);
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
        }
        ++i;
    }
    return 0;
}
