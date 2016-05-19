#include<stdio.h>
#include<stdlib.h>
#include <unistd.h>
#include <err.h>
#include <fcntl.h>
#include <sys/wait.h>

int main(int argc, char** argv){

    int pipe1[2];
    pid_t pr1, pr2, pr3;
    int status1, status2, status3;
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

    if( pr1 == 0 )
    {
		pid_t son1, son2, son3;
		int stat1, stat2, stat3;
        close(pipe1[0]);
        close(pipe1[1]);
		son1 = fork();

		if(son1 == 0)
		{
			execlp( argv[1], argv[1], NULL);
			printf("We must not be here\n");
		}


		waitpid(son1, &stat1, 0);
		int fd = open("log", O_APPEND);
		if(stat1)
		{	
			son2 = fork();
			if(son2 == 0)
			{
				dup2(fd, 1);
				execlp("echo", "echo", "bee", NULL);
			}
			waitpid(son2, &stat2, 0);
		}
		else
		{
			son3 = fork();
			if(son3 == 0)
			{
				dup2(fd, 1);
				execlp("echo", "echo", "ok", NULL);
			}
			waitpid(son3, &stat3, 0);
		}
		close(fd);
    }

	waitpid(pr1, &status1, 0);
	if(status1)
	{
		return 10;
	}
	int fd = open("log", O_RDONLY);
    pr2 = fork();

    if( pr2 == -1 )
    {
        perror("Can't fork process");
		return 4;
    }
	
    if( pr2 == 0 )
    {
		dup2(fd, 0);
		dup2(pipe1[1], 1);
        close(pipe1[0]);
        close(pipe1[1]);

        execlp( argv[2], argv[2], NULL);
        printf("We should not get here, execlp not working");
    }
	close(fd);
    pr3 = fork();

    if( pr3 == -1 )
    {
        perror("Can't fork process");
		return 4;
    }

    if( pr3 == 0 )
    {
        dup2( pipe1[0], 0 );
        close(pipe1[0]);
        close(pipe1[1]);

        execlp(argv[3], argv[3], NULL);
        printf("We should not get here, execlp not working");
    }
    close(pipe1[0]);
    close(pipe1[1]);
    waitpid( pr2, &status2, 0 );
    waitpid( pr3, &status3, 0 );
    if( status1 || status2)
    {
        return 9;
    }
	system("echo OOK");
	return 0;
}
