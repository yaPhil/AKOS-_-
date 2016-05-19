#include <unistd.h>
#include <err.h>
#include <fcntl.h>
#include <sys/wait.h>

int main(int argc, char** argv)
{

    int pipe1[2], pipe2[2], pipe3[2];
    pid_t pr1, pr2, pr3, pr4, pr5, pr6;
    int status1, status2, status3, status4, status5, status6;
    if( pipe( pipe1 ))
    {
        perror("Error opening pipe1");
		return 1;
    }
    if( pipe( pipe2 ))
    {
        perror("Error opening pipe2");
		return 2;
    }
	if(pipe(pipe3))
	{
		perrr("Error opening pipe3");
		return 3;
	}

    pr1 = fork();

    if( pr1 == -1 )
    {
        perror("Can't fork process");
		return 4;
    }
    if( pr1 == 0 )
    {
        dup2( pipe1[1], 1 );
        close(pipe1[0]);
        close(pipe1[1]);
        close(pipe2[0]);
        close(pipe2[1]);
        close(pipe3[0]);
        close(pipe3[1]);
        execlp( argv[1], argv[1], NULL);
        printf("We must not be here\n");
    }

    pr2 = fork();

    if( pr2 == -1 )
    {
        perror("Can't fork process");
		return 4;
    }

    if( pr2 == 0 )
    {
        dup2( pipe1[0], 0 );
        dup2( pipe2[1], 1 );
        close(pipe1[0]);
        close(pipe1[1]);
        close(pipe2[0]);
        close(pipe2[1]);
		close(pipe3[0]);
		close(pipe3[1]);

        execlp( argv[2], argv[2], NULL);
        printf("We should not get here, execlp not working");
    }

    pr3 = fork();

    if( pr3 == -1 )
    {
        perror("Can't fork process");
		return 4;
    }

    if( pr3 == 0 )
    {
        dup2( pipe2[0], 0 );
        close(pipe1[0]);
        close(pipe1[1]);
        close(pipe2[0]);
        close(pipe2[1]);
		close(pipe3[0]);
		close(pipe3[1]);
        execlp(argv[3], argv[3], NULL);
        printf("We should not get here, execlp not working");
    }
    close(pipe1[0]);
    close(pipe2[0]);
    close(pipe1[1]);
    close(pipe2[1]);
    waitpid( pr1, &status1, 0 );
    waitpid( pr2, &status2, 0 );
    waitpid( pr3, &status3, 0 );
    if( status1 || status2 || status3 )
    {
        return 6;
    }

	pr4 = fork();
	if(pr4 == 0)
	{
		close(pipe3[0]);
		close(pipe3[1]);
		execlp(argv[4], argv[4], NULL);

        printf("We should not get here, execlp not working");
	}
	waitpid(pr4, &status4, 0);
	if(status4)
	{
		return 7;
	}

	pr5 = fork();
	if(pr5 == 0)
	{
		dup2(pipe3[1], 1);
		close(pipe3[0]);
		close(pipe3[1]);
		execlp(argv[5], argv[5], argv[6], NULL);
		
        printf("We should not get here, execlp not working");
	}	
	pr6 = fork();
	if(pr6 == 0)
	{
		dup2(pipe3[0], 0);
		close(pipe3[0]);
		close(pipe3[1]);
		execlp(argv[7], argv[7], NULL);

        printf("We should not get here, execlp not working");
	}
	close(pipe3[0]);
	close(pipe3[1]);
	waitpid(pid5, &status5, 0);
	waitpid(pid6, &status6, 0);
	if(status5 || status6)
	{
		return 8;
	}
	return 0;
}
