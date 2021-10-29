#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>


int main(void)
{
	pid_t pid;
	int rv;
	int sarasa;
	switch(pid = fork())
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
		printf(" CHILD: Llamo a exec...\n ");
		char* const args[] = {"/bin/ls","-lah",NULL};
		execv("/bin/ls",args);
		printf(" CHILD: I'm outta here!\n");
		exit(rv);
		break;


		default:
		printf("PARENT: This is the parent process!\n");
		printf("PARENT: My PID is %d\n", getpid());
		printf("PARENT: My childs PID is %d\n", pid);
		wait(&sarasa);
		printf("PARENT: My child's exit status is: %d\n", WEXITSTATUS(sarasa));
		printf("PARENT: I'm outta here!\n");
	}
}

