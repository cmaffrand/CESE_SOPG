#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

void recibiSignal(int sig)
{
	write(1,"Llego sigchild\n",18);
    wait(NULL);
}

int main(void)
{
	pid_t pid;
	int rv;
	int sarasa;
    struct sigaction sa;

	sa.sa_handler = recibiSignal;
	sa.sa_flags = SA_RESTART; //0
	// Con SA_RESTART vuelve a ejecutar.
	// Con 0 sigue ejecutando.
	sigemptyset(&sa.sa_mask);
	sigaction(SIGCHLD,&sa,NULL);

	switch (pid = fork())
	{
	case -1:
		perror("fork"); /* something went wrong */
		exit(1);
		break;

	/* parent exits */
	case 0:
		printf(" CHILD: This is the child process!\n");
		printf(" CHILD: My PID is %d\n", getpid());
		printf(" CHILD: My parent's PID is %d\n", getppid());
		printf(" CHILD: Enter my exit status (make it small): ");
		//scanf(" %d", &rv);
        sleep(5);
		printf(" CHILD: I'm outta here!\n");
		exit(0);
		break;

	default:
		printf("PARENT: This is the parent process!\n");
		printf("PARENT: My PID is %d\n", getpid());
		printf("PARENT: My childs PID is %d\n", pid);
		while (1)
        {
            sleep(1);
        }
        
		printf("PARENT: I'm outta here!\n");
	}
}