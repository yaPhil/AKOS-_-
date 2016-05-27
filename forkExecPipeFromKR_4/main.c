#include<stdio.h>
#include<stdlib.h>
#include <unistd.h>
#include <err.h>
#include <fcntl.h>
#include <sys/wait.h>

int main(int argc, char** argv)
{
    if(argc != 5)
    {
        printf("Bad arguments\n");
        return 20;
    }
    int pipe1[2];
    pid_t pr1, pr2, pr3;
    int status1, status2, status3;
    int fd;
    if( pipe( pipe1 ))
    {
        perror("Error opening pipe1");
        return 1;
    }
    pr1 = fork();
    if( pr1 == -1 )
    {
        perror("Can't fork process");
        return 4;
    }
    fd = open("f.res", O_RDONLY);
    if(pr1 == 0)
    {
        dup2(fd, 0);
        close(pipe1[1]);
        close(pipe1[0]);
        execlp(argv[1], argv[1], argv[2], NULL);
        printf("We must not be here\n");
    }
    close(fd);
    waitpid(pr1, &status1, 0);
    if(status1)
    {
        close(pipe1[0]);
        close(pipe1[1]);
        return 10;
    }
    else
    {
        pr2 = fork();
        if( pr2 == -1 )
        {
            perror("Can't fork process");
            return 4;
        }
        if(pr2 == 0)
        {
            dup2(pipe1[1], 1);
            close(pipe1[1]);
            close(pipe1[0]);
            execlp(argv[3], argv[3], NULL);
            printf("We must not be here\n");
        }

        pr3 = fork();
        if( pr3 == -1 )
        {
            perror("Can't fork process");
            return 4;
        }
        fd = open("f.res", O_APPEND);
        if(pr3 == 0)
        {
            dup2(pipe1[0], 0);
            dup2(fd, 1);
            execlp(argv[4], argv[4], NULL);
            printf("We must not be here\n");
        }
        close(pipe1[0]);
        close(pipe1[1]);
        close(fd);
        waitpid( pr2, &status2, 0 );
        waitpid( pr3, &status3, 0 );
        if(status2 || status3)
        {
            return 9;
        }
    }
    return 0;
}
