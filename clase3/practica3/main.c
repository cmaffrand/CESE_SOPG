#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

void recibiSignal(int sig)
{
	write(1, "Llego sigchild\n", 18);
	wait(NULL);
}

int main(void)
{
	pid_t pid;
	int rv;
	int sarasa;
	struct sigaction sa;
	int fds[2];
	char buffer[64];

	sa.sa_handler = recibiSignal;
	sa.sa_flags = SA_RESTART; //0
	// Con SA_RESTART vuelve a ejecutar.
	// Con 0 sigue ejecutando.
	sigemptyset(&sa.sa_mask);

	pipe(fds); // falta controlar el error
	// fds[0] escritura
	// fds[1] lectura

	switch (pid = fork())
	{
	case -1:
		perror("fork"); /* something went wrong */
		exit(1);
		break;

	/* parent exits */
	case 0:
		close(fds[0]); // Se usa para escritura entonces cierro el de lectura
		printf(" CHILD: This is the child process!\n");
		printf(" CHILD: My PID is %d\n", getpid());
		printf(" CHILD: My parent's PID is %d\n", getppid());
		printf(" CHILD: Enter my exit status (make it small): ");
		
		sleep(5);
		write(fds[1],"Hola1",6);
		sleep(5);
		write(fds[1],"Hola2",6);

		printf(" CHILD: I'm outta here!\n");
		exit(0);
		break;

	default:

		close(fds[1]); // Se usa para lectura entonces cierro el de escritura
		printf("PARENT: This is the parent process!\n");
		printf("PARENT: My PID is %d\n", getpid());
		printf("PARENT: My childs PID is %d\n", pid);
		while (1)
		{
			int r = read(fds[0],buffer,sizeof(buffer));
			printf("Read Devolvio: %d\n",r);
			if (r > 0)
			{
				printf("PARENT: Llego %s\n", buffer);
			}
			else if (r == 0)
			{
				printf("Llego EOF\n");
				break;
			}
			else
			{
				perror("read");
				break;
			}
		}

		printf("PARENT: I'm outta here!\n");
	}
}