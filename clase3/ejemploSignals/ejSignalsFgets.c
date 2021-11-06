#include <stdio.h>
#include <unistd.h>
#include <signal.h>

void recibiSignal(int sig)
{
	write(1,"se presiono ctrl+c!!\n",21);
}

int main(void)
{
	struct sigaction sa;

	sa.sa_handler = recibiSignal;
	sa.sa_flags = SA_RESTART; //0
	// Con SA_RESTART vuelve a ejecutar.
	// Con 0 sigue ejecutando.
    
    sigemptyset(&sa.sa_mask);
	sigaction(SIGINT,&sa,NULL);
	
	char s[200];
	if(fgets(s,sizeof(s),stdin) == NULL)
	{
		perror("error en fgets");
	}
	else
	{
		printf("ingresaste:%s\n",s);
	}

	return 0;
}
