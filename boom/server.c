#define _XOPEN_SOURCE 500
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>
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

struct mine
{
    int row;
    int column;
    int owner;
};

int mapRows = 0;
int mapColumns = 0;
int initialHealth = 0;
int hitValue = 0, reloadTime = 0, mineTime = 0, stayDrop = 0, moveDrop = 0, immortalTime = 0;
double stepDelay = 0;

char** fileMap = NULL;
char** justMap = NULL;

struct item* items = NULL;
struct mine* mines = NULL;
int numMines = 0;
int numItems = 0;

int socketID;
int bindReturn;
int acceRet;
struct sockaddr_in server_addr;

pthread_t* arrThread;
pthread_mutex_t **mutex;
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

int min(const int a, const int b)
{
    if(a < b)
        return a;
    else
        return b;
}
int max(const int a, const int b)
{
    if(a > b)
        return a;
    else
        return b;
}

char* intToStr(int a, int* sz)
{
    char* str = NULL;
    int size = 0;
    int i = 0;
    while(a != 0)
    {
        int tmp = a % 10;
        str = realloc(str, (size + 1)*sizeof(char));
        str[size] = '0' + tmp;
        ++size;
        a /= 10;
    }
    for(i = 0; i < size / 2; ++i)
    {
        char t = str[i];
        str[i] = str[size - i - 1];
        str[size - i - 1] = t;
    }
    *sz = size;
    return str;
}

char* concatinate(char* name, char* a, char* b, char* c, int sz1, int sz2, int sz3, int sz_n)
{
    char* ans = NULL;
    int i = 0;
    ans = malloc((sz_n + sz1 + sz2 + sz3 + 5) * sizeof(char));
    for(i = 0; i < sz_n; ++i)
    {
        ans[i] = name[i];
    }
    ans[sz_n] = ' ';
    for(i = 0; i < sz1; ++i)
    {
        ans[i + sz_n + 1] = a[i];
    }
    ans[sz_n + sz1 + 1] = ' ';
    for(i = 0; i < sz2; ++i)
    {
        ans[i + sz_n + sz1 + 2] = b[i];
    }
    ans[sz_n + sz1 + sz2 + 2] = ' ';
    for(i = 0; i < sz3; ++i)
    {
        ans[i + sz_n + sz1 + sz2 + 3] = c[i];
    }
    ans[sz_n + sz1 + sz2 + sz3 + 3] = '\n';
    ans[sz_n + sz1 + sz2 + sz3 + 4] = '\0';
    return ans;
}

char* concatinate2(char* name, int f)
{
    char* res = malloc(strlen(name) + 1);
    int i = 0;
    int sz = strlen(name);
    for(i = 0; i < sz; ++i)
    {
        res[i] = name[i];
    }
    res[sz] = ' ';
    if(f == 0)
    {
        res = realloc(res, sz + 13);
        res[sz + 1] = 'i'; res[sz + 2] = 's'; res[sz + 3] = ' '; res[sz + 4] = 'c';
        res[sz + 5] = 'r'; res[sz + 6] = 'e'; res[sz + 7] = 'a'; res[sz + 8] = 't';
        res[sz + 9] = 'o'; res[sz + 10] ='r'; res[sz + 11] = '\n'; res[sz + 12] = '\0';
    }
    else
    {
        res = realloc(res, sz + 12);
        res[sz + 1] = 'i'; res[sz + 2] = 's'; res[sz + 3] = ' '; res[sz + 4] = 'p';
        res[sz + 5] = 'l'; res[sz + 6] = 'a'; res[sz + 7] = 'y'; res[sz + 8] = 'e';
        res[sz + 9] = 'r'; res[sz + 10] = '\n'; res[sz + 11] = '\0';
    }
    return res;
}

void sendMap(char** lastMap, char** newMap, int socket)
{
    int maxStr = 21, maxCol = 21;
    int i = 0, j = 0;
    int f = 0;
    for(i = 0; i < maxStr; ++i)
    {
        for(j = 0; j < maxCol; ++j)
        {
            if(newMap[i][j] != lastMap[i][j])
            {
                f = 1;
                break;
            }
        }
        if(f == 1)
            break;
    }
    if(f == 1)
    {
        send(socket, &maxStr, sizeof(int), 0);
        send(socket, &maxCol, sizeof(int), 0);
        for(i = 0; i < maxStr; ++i)
        {
            for(j = 0; j < maxCol; ++j)
            {
                send(socket, &newMap[i][j], sizeof(char), 0);
                lastMap[i][j] = newMap[i][j];
            }
        }

    }
}

int isMine(int i, int j)
{
    int k = 0;
    for(k = 0; k < numMines; ++k)
    {
        if(mines[k].row == i && mines[k].column == j)
        {
            return mines[k].owner;
        }
    }
    return 0;
}

int isItem(int i, int j)
{
    int k = 0;
    for(k = 0; k < numItems; ++k)
    {
        if(items[k].row == i && items[k].column == j)
        {
            return 1;
        }
    }
    return 0;
}


void getCurentMap(int i, int j, char** map, int number)
{
    int firstRow = 0, lastRow = 0, firstColumn = 0, lastColumn = 0;
    int strings, size, k, m;
    int maxStr = 21, maxCol = 21;
    int cur_i = 0, cur_j = 0;
    firstRow = max(i - 10, 0);
    lastRow = min(i + 10, mapRows + 1);
    firstColumn = max(0, j - 10);
    lastColumn = min(mapColumns + 1, j + 10);
    strings = lastRow - firstRow + 1;
    size = lastColumn - firstColumn + 1;
    if(firstRow == 0)
    {
        for(k = 0; k < maxStr - strings; ++k)
        {
            for(m = 0; m < maxCol; ++m)
            {
                map[cur_i][cur_j] = '#';
                ++cur_j;
            }
            ++cur_i;
            cur_j = 0;
        }
        if(firstColumn == 0)
        {
            for(k = firstRow; k <= lastRow; ++k)
            {
                for(m = 0; m < maxCol - size; ++m)
                {
                    map[cur_i][cur_j] = '#';
                    ++cur_j;
                }
                for(m = firstColumn; m <= lastColumn; ++m)
                {
                    if(justMap[k][m] == '#' || justMap[k][m] == '@')
                    {
                        map[cur_i][cur_j] = justMap[k][m];
                    }
                    else
                    {
                        int flagItem = isItem(k, m);
                        int flagMine = isMine(k, m);
                        if(flagItem == 1 && flagMine == number)
                        {
                            map[cur_i][cur_j] = 'T';
                        }
                        else if(flagItem == 1)
                        {
                            map[cur_i][cur_j] = 'I';
                        }
                            else if (flagMine == number)
                            {
                                map[cur_i][cur_j] = 'M';
                            }
                                else
                                    map[cur_i][cur_j] = ' ';
                    }
                    ++cur_j;

                }
                ++cur_i;
                cur_j = 0;
            }
        }
        else
        {
            for(k = firstRow; k <= lastRow; ++k)
            {
                for(m = firstColumn; m <= lastColumn; ++m)
                {
                    if(justMap[k][m] == '#' || justMap[k][m] == '@')
                    {
                        map[cur_i][cur_j] = justMap[k][m];
                    }
                    else
                    {
                        int flagItem = isItem(k, m);
                        int flagMine = isMine(k, m);
                        if(flagItem == 1 && flagMine == number)
                        {
                            map[cur_i][cur_j] = 'T';
                        }
                        else if(flagItem == 1)
                        {
                            map[cur_i][cur_j] = 'I';
                        }
                            else if (flagMine == number)
                            {
                                map[cur_i][cur_j] = 'M';
                            }
                                else
                                    map[cur_i][cur_j] = ' ';
                    }
                    ++cur_j;
                }
                for(m = 0; m < maxCol - size; ++m)
                {
                    map[cur_i][cur_j] = '#';
                    ++cur_j;
                }
                ++cur_i;
                cur_j = 0;
            }
        }
    }
    else
    {
        if(firstColumn == 0)
        {
            for(k = firstRow; k <= lastRow; ++k)
            {
                for(m = 0; m < maxCol - size; ++m)
                {
                    map[cur_i][cur_j] = '#';
                    ++cur_j;
                }
                for(m = firstColumn; m <= lastColumn; ++m)
                {
                    if(justMap[k][m] == '#' || justMap[k][m] == '@')
                    {
                        map[cur_i][cur_j] = justMap[k][m];
                    }
                    else
                    {
                        int flagItem = isItem(k, m);
                        int flagMine = isMine(k, m);
                        if(flagItem == 1 && flagMine == number)
                        {
                            map[cur_i][cur_j] = 'T';
                        }
                        else if(flagItem == 1)
                        {
                            map[cur_i][cur_j] = 'I';
                        }
                            else if (flagMine == number)
                            {
                                map[cur_i][cur_j] = 'M';
                            }
                                else
                                    map[cur_i][cur_j] = ' ';
                    }
                    ++cur_j;
                }
                ++cur_i;
                cur_j = 0;
            }
        }
        else
        {
            for(k = firstRow; k <= lastRow; ++k)
            {
                for(m = firstColumn; m <= lastColumn; ++m)
                {
                    if(justMap[k][m] == '#' || justMap[k][m] == '@')
                    {
                        map[cur_i][cur_j] = justMap[k][m];
                    }
                    else
                    {
                        int flagItem = isItem(k, m);
                        int flagMine = isMine(k, m);
                        if(flagItem == 1 && flagMine == number)
                        {
                            map[cur_i][cur_j] = 'T';
                        }
                        else if(flagItem == 1)
                        {
                            map[cur_i][cur_j] = 'I';
                        }
                            else if (flagMine == number)
                            {
                                map[cur_i][cur_j] = 'M';
                            }
                                else
                                    map[cur_i][cur_j] = ' ';
                    }
                    ++cur_j;
                }
                for(m = 0; m < maxCol - size; ++m)
                {
                    map[cur_i][cur_j] = '#';
                    ++cur_j;
                }
                ++cur_i;
                cur_j = 0;
            }
        }
        for(k = 0; k < maxStr - strings; ++k)
        {
            for(m = 0; m < maxCol; ++m)
            {
                map[cur_i][cur_j] = '#';
                ++cur_j;
            }
            ++cur_i;
            cur_j = 0;
        }
    }
}

void useItem(int i, int j, struct member* p)
{
    int k = 0;
    for(k = 0; k < numItems; ++k)
    {
        if(items[k].row == i && items[k].column == j)
        {
            p->health += items[k].effect;
            items[k].effect = items[k].row = items[k].column = 0;
            return;
        }
    }
    return;
}

void blowUp(int i, int j, struct member* p)
{
    int k = 0;
    for(k = 0; k < numMines; ++k)
    {
        if(mines[k].row == i && mines[k].column == j)
        {
            if(mines[k].owner != p->number)
            {
                p->health -= hitValue;
                mines[k].owner = mines[k].row = mines[k].column = 0;
                return;
            }
        }
    }
    return;
}

void* fthread(void* arg)
{
	struct member* person = (struct member*)arg;
	if(creatorNumber == -1)
	{
        int size = 33;
        char* answer = NULL;
        int strings = 1;
        creatorNumber = person->number;
        size = 2;
        send(person->fd, &size, sizeof(int), 0);
        size = 33;
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
        strings = 1;
        send(person->fd, &strings, sizeof(int), 0);
        send(person->fd, &size, sizeof(int), 0);
        send(person->fd, "Input 1 to start game; 2 to see list of players\n\0", size, 0);
        //answer[0] = '9';
        recv(person->fd, &size, sizeof(int), 0);
        printf("----get size%d\n", size);
        answer = realloc(answer, size);
        recv(person->fd, answer, size, 0);
        printf("get %d answer %s\n", size, answer);
        while(size != 2 || (answer[0] != '2' && answer[0] != '1'))
        {
            int sz = 49;
            int str = 1;
            send(person->fd, &str, sizeof(int), 0);
            send(person->fd, &sz, sizeof(int), 0);
            send(person->fd, "Input 1 to start game; 2 to see list of players\n\0", sz, 0);
            recv(person->fd, &size, sizeof(int), 0);
            recv(person->fd, answer, size, 0);
        }
        while(answer[0] == '2')
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
            
            printf("send mess %d\n", cnt);
            
            for(i = 0; i < playerNumber; ++i)
            {
                if(data[i].active == 1)
                {
                    if(data[i].role == 1)
                    {
                        char* res = concatinate2(data[i].name, 0);
                        sz = strlen(data[i].name) + 13;
                        send(person->fd, &sz, sizeof(int), 0);
                        while(sz != 0)
                        {
                            sz -= send(person->fd, res + strlen(res) + 1 - sz, sz * sizeof(char), 0);
                        }
                        free(res);
                    }
                    else
                    {
                        char* res = concatinate2(data[i].name, 1);
                        sz = strlen(data[i].name) + 12;
                        send(person->fd, &sz, sizeof(int), 0);
                        while(sz != 0)
                        {
                            sz -= send(person->fd, res + strlen(res) + 1 - sz, sz * sizeof(char), 0);
                        }
                        free(res);
                    }
                }
            }
            sz = 49;
            send(person->fd, &sz, sizeof(int), 0);
            send(person->fd, "Input 1 to start game; 2 to see list of players\n\0", sz, 0);
            printf("send invite\n");
            recv(person->fd, &sz, sizeof(int), 0);
            recv(person->fd, answer, sz, 0);
            printf("%c--------%d\n", answer[0], sz);
            while(sz != 2 || (answer[0] != '2' && answer[0] != '1'))
            {
                int snd = 49;
                int str = 1;
                send(person->fd, &str, sizeof(int), 0);
                send(person->fd, &snd, sizeof(int), 0);
                send(person->fd, "Input 1 to start game; 2 to see list of players\n\0", snd, 0);
                
                recv(person->fd, &sz, sizeof(int), 0);
                recv(person->fd, answer, sz, 0);
            }
        }
        
        isGameStart = 1;
        gameStart = time(0);
        strings = 1;
        size = 42;
        send(person->fd, &strings, sizeof(int), 0);
        send(person->fd, &size, sizeof(int), 0);
        send(person->fd, "Input 1 to end game; 2 to see statistics\n\0", size, 0);
        
        recv(person->fd, &size, sizeof(int), 0);
        recv(person->fd, answer, size, 0);
        while(size != 2 || (answer[0] != '1' && answer[0] != '2'))
        {
            int sz = 42;
            int str = 1;
            send(person->fd, &str, sizeof(int), 0);
            send(person->fd, &sz, sizeof(int), 0);
            send(person->fd, "Input 1 to end game; 2 to see statistics\n\0", size, 0);
            recv(person->fd, &size, sizeof(int), 0);
            recv(person->fd, answer, size, 0);
        }

        while(answer[0] == '2')
        {
            int i = 0, cnt = 0, sz = 0;
            for(i = 0; i < playerNumber; ++i)
            {
                if(data[i].active == 1 )
                {
                    ++cnt;
                }
            }
            cnt += 2;
            send(person->fd, &cnt, sizeof(int), 0);
            sz = 25;
            send(person->fd, &sz, sizeof(int), 0);
            send(person->fd, "Name pos_i pos_j health\n\0", sz, 0);
            for(i = 0; i < playerNumber; ++i)
            {
                if(data[i].active == 1)
                {
                    if(data[i].role == 1)
                    {
                        char* res = concatinate2(data[i].name, 0);
                        sz = strlen(data[i].name) + 13;
                        send(person->fd, &sz, sizeof(int), 0);
                        while(sz != 0)
                        {
                            sz -= send(person->fd, res + strlen(res) + 1 - sz, sz, 0);
                        }
                    }
                    else
                    {
                        int sz1 = 0, sz2 = 0, sz3 = 0;
                        char* pos_i = NULL, *pos_j = NULL, *health = NULL;
                        char* res = NULL;
                        pos_i = intToStr(data[i].pos_i, &sz1);
                        pos_j = intToStr(data[i].pos_j, &sz2);
                        health = intToStr(data[i].health, &sz3);
                        res = concatinate(data[i].name, pos_i, pos_j, health, sz1, sz2, sz3, strlen(data[i].name));
                        sz = sz1 + sz2 + sz3 + strlen(data[i].name) + 5;
                        send(person->fd, &sz, sizeof(int), 0);
                        while(sz != 0)
                        {
                            sz -= send(person->fd, res + strlen(res) + 1 - sz, sz * sizeof(char), 0);
                        }
                        free(res);
                        free(pos_i);
                        free(pos_j);
                        free(health);
                    }
                }
            }
            sz = 42;
            send(person->fd, &sz, sizeof(int), 0);
            send(person->fd, "Input 1 to end game; 2 to see statistics\n\0", sz, 0);
            
            recv(person->fd, &size, sizeof(int), 0);
            recv(person->fd, answer, size, 0);
            while(size != 2 || (answer[0] != '1' && answer[0] != '2'))
            {
                int sz = 42;
                int str = 1;
                send(person->fd, &str, sizeof(int), 0);
                send(person->fd, &sz, sizeof(int), 0);
                send(person->fd, "Input 1 to end game; 2 to see statistics\n\0", size, 0);
                recv(person->fd, &size, sizeof(int), 0);
                recv(person->fd, answer, size, 0);
            }
        }
        /*
         ENDING GAME
         */
	}
    else
    {
        int size = 41;
        int strings = 1;
        unsigned int state = 1;
        int message = 0;
        char **lastMap = NULL, **newMap = NULL;
        int i = 0, j = 0;
        if(isGameStart == 1)
        {
            size = 3;
            send(person->fd, &size, sizeof(int), 0);
            while(isGameStart == 1)
            {
            }
        }
        else
        {
            size = 1;
            send(person->fd, &size, sizeof(int), 0);
        }
        send(person->fd, &stepDelay, sizeof(double), 0);
        size = 41;
        send(person->fd, &strings, sizeof(int), 0);
        send(person->fd, &size, sizeof(int), 0);
        send(person->fd, "You are player\nEnter your name and wait\n\0", size, 0);
        recv(person->fd, &size, sizeof(int), 0);
        person->name = malloc(size);
        person->active = 1;
        person->role = 0;
        recv(person->fd, person->name, size, 0);
        printf("%s\n",person->name);
        while(isGameStart == 0)
        {
            
        }
        person->pos_i = rand_r(&state) % mapRows + 1;
        person->pos_j = rand_r(&state) % mapColumns + 1;

        pthread_mutex_lock(&mutex[person->pos_i][person->pos_j]);
        while(justMap[person->pos_i][person->pos_j] == '@')
        {
            int last_i = person->pos_i, last_j = person->pos_j;
            person->pos_i = rand_r(&state) % mapRows + 1;
            person->pos_j = rand_r(&state) % mapColumns + 1;
            pthread_mutex_lock(&mutex[person->pos_i][person->pos_j]);
            pthread_mutex_unlock(&mutex[last_i][last_j]);
        }
        justMap[person->pos_i][person->pos_j] = '@';
        pthread_mutex_unlock(&mutex[person->pos_i][person->pos_j]);
        message = 1;
        strings = 1;
        size = 39;
        send(person->fd, &strings, sizeof(int), 0);
        send(person->fd, &size, sizeof(int), 0);
        send(person->fd, "Press u to use, s to shoot, m to mine\n\0", size, 0);
        lastMap = malloc(21 * sizeof(char*));
        newMap = malloc(21 * sizeof(char*));
        for(i = 0; i < 21; ++i)
        {
            lastMap[i] = malloc(21 * sizeof(char));
            newMap[i] = malloc(21 * sizeof(char));
        }
        while(2 * 2 == 4)
        {
            int cmd = 0;
            recv(person->fd, &cmd, sizeof(int), 0);
            if(cmd == 0)
            {
               person->health -= stayDrop;
            }
            if(person->mine + mineTime < time(0))
            {
                if((char)cmd == 's')
                {
                    if(gameStart + immortalTime < time(0))
                    {

                    }
                }
                if((char)cmd == 'u')
                {
                    useItem(person->pos_i, person->pos_j, person);
                }
                if((char)cmd == 'm')
                {
                    if(gameStart + immortalTime < time(0) && person->mine + mineTime < time(0) && isMine(person->pos_i, person->pos_j) == 0)
                    {
                        struct mine tmp;
                        tmp.row = person->pos_i;
                        tmp.column = person->pos_j;
                        tmp.owner = person->number;
                        mines = realloc(mines, (numMines + 1) * sizeof(struct mine));
                        mines[numMines] = tmp;
                        ++numMines;
                        person->mine = time(0);
                    }
                }
                if(cmd == KEY_LEFT)
                {
                    pthread_mutex_lock(&mutex[person->pos_i][person->pos_j]);
                    pthread_mutex_lock(&mutex[person->pos_i][person->pos_j - 1]);
                    if(justMap[person->pos_i][person->pos_j - 1] != '@' &&
                       justMap[person->pos_i][person->pos_j - 1] != '#')
                    {
                        int flagItem = isItem(person->pos_i, person->pos_j);
                        if(flagItem == 1)
                        {
                            justMap[person->pos_i][person->pos_j] = 'I';
                        }
                        else
                            justMap[person->pos_i][person->pos_j] = ' ';
                        justMap[person->pos_i][person->pos_j - 1] = '@';
                        person->health -= moveDrop;
                        blowUp(person->pos_i, person->pos_j - 1, person);
                        pthread_mutex_unlock(&mutex[person->pos_i][person->pos_j - 1]);
                        pthread_mutex_unlock(&mutex[person->pos_i][person->pos_j]);
                        person->pos_j--;

                    }
                    else
                    {
                        pthread_mutex_unlock(&mutex[person->pos_i][person->pos_j - 1]);
                        pthread_mutex_unlock(&mutex[person->pos_i][person->pos_j]);
                    }

                }
                if(cmd == KEY_RIGHT)
                {
                    pthread_mutex_lock(&mutex[person->pos_i][person->pos_j]);
                    pthread_mutex_lock(&mutex[person->pos_i][person->pos_j + 1]);
                    if(justMap[person->pos_i][person->pos_j + 1] != '@' &&
                       justMap[person->pos_i][person->pos_j + 1] != '#')
                    {
                        int flagItem = isItem(person->pos_i, person->pos_j);
                        if(flagItem == 1)
                        {
                            justMap[person->pos_i][person->pos_j] = 'I';
                        }
                        else
                            justMap[person->pos_i][person->pos_j] = ' ';
                        justMap[person->pos_i][person->pos_j + 1] = '@';
                        blowUp(person->pos_i, person->pos_j + 1, person);
                        person->health -= moveDrop;
                        pthread_mutex_unlock(&mutex[person->pos_i][person->pos_j + 1]);
                        pthread_mutex_unlock(&mutex[person->pos_i][person->pos_j]);
                        person->pos_j++;
                    }
                    else
                    {
                        pthread_mutex_unlock(&mutex[person->pos_i][person->pos_j + 1]);
                        pthread_mutex_unlock(&mutex[person->pos_i][person->pos_j]);
                    }
                }
                if(cmd == KEY_UP)
                {
                    pthread_mutex_lock(&mutex[person->pos_i][person->pos_j]);
                    pthread_mutex_lock(&mutex[person->pos_i - 1][person->pos_j]);
                    if(justMap[person->pos_i - 1][person->pos_j] != '@' &&
                       justMap[person->pos_i - 1][person->pos_j] != '#')
                    {
                        int flagItem = isItem(person->pos_i, person->pos_j);
                        if(flagItem == 1)
                        {
                            justMap[person->pos_i][person->pos_j] = 'I';
                        }
                        else
                            justMap[person->pos_i][person->pos_j] = ' ';
                        justMap[person->pos_i - 1][person->pos_j] = '@';
                        blowUp(person->pos_i - 1, person->pos_j, person);
                        person->health -= moveDrop;
                        pthread_mutex_unlock(&mutex[person->pos_i - 1][person->pos_j]);
                        pthread_mutex_unlock(&mutex[person->pos_i][person->pos_j]);
                        person->pos_i--;
                    }
                    else
                    {
                        pthread_mutex_unlock(&mutex[person->pos_i - 1][person->pos_j]);
                        pthread_mutex_unlock(&mutex[person->pos_i][person->pos_j]);
                    }
                }
                if(cmd == KEY_DOWN)
                {
                    pthread_mutex_lock(&mutex[person->pos_i][person->pos_j]);
                    pthread_mutex_lock(&mutex[person->pos_i + 1][person->pos_j]);
                    if(justMap[person->pos_i + 1][person->pos_j] != '@' &&
                       justMap[person->pos_i + 1][person->pos_j] != '#')
                    {
                        int flagItem = isItem(person->pos_i, person->pos_j);
                        if(flagItem == 1)
                        {
                            justMap[person->pos_i][person->pos_j] = 'I';
                        }
                        else
                            justMap[person->pos_i][person->pos_j] = ' ';
                        justMap[person->pos_i + 1][person->pos_j] = '@';
                        blowUp(person->pos_i + 1, person->pos_j, person);
                        person->health -= moveDrop;
                        pthread_mutex_unlock(&mutex[person->pos_i + 1][person->pos_j]);
                        pthread_mutex_unlock(&mutex[person->pos_i][person->pos_j]);
                        person->pos_i++;
                    }
                    else
                    {
                        pthread_mutex_unlock(&mutex[person->pos_i + 1][person->pos_j]);
                        pthread_mutex_unlock(&mutex[person->pos_i][person->pos_j]);
                    }
                }
            }
            getCurentMap(person->pos_i, person->pos_j, newMap, person->number);
            sendMap(lastMap, newMap, person->fd);
        }
    }
    return NULL;
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
    
    printf("%d %d %d %d %d %d %d %d %lf  %d\n", mapRows, mapColumns, initialHealth,
           hitValue, reloadTime, mineTime, stayDrop, moveDrop, stepDelay, immortalTime);

    fclose(mapOut);
    mapOut = NULL;

    mapOut = fopen("justMap.out", "r+");

	justMap = scanFile(mapOut, &mapErr, &mapSize);
    
    mutex = malloc((mapRows + 1) * sizeof(pthread_mutex_t*));
    for(i = 0; i <= mapRows; ++i)
    {
        mutex[i] = malloc((mapColumns + 1) * sizeof(pthread_mutex_t));
    }
    
    for(i = 1; i <= mapRows; ++i)
    {
        for(j = 1; j <= mapColumns; ++j)
        {
            pthread_mutex_init(&(mutex[i][j]), NULL);
        }
    }
    
    printf("%d HERE %d !!!!!! %d\n", mapSize, numItems, mapErr);
    printf("%d %d\n", items[2].row, items[2].column);
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
