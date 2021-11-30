#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>


int main()
{
	socklen_t addr_len;
	struct sockaddr_in clientaddr;
	struct sockaddr_in serveraddr;
	char buffer[128];
	int newfd;
	int n;

	// Creamos socket
	int s = socket(PF_INET,SOCK_STREAM, 0);

	// Cargamos datos de IP:PORT del server
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(4096);
    serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    if(serveraddr.sin_addr.s_addr==INADDR_NONE)
    {
        fprintf(stderr,"ERROR invalid server IP\r\n");
        return 1;
    }

	// Abrimos puerto con bind()
	if (bind(s, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) == -1) {
		close(s);
		perror("listener: bind");
		return 1;
	}

	// Seteamos socket en modo Listening
	if (listen (s, 10) == -1) // backlog=10
  	{
    	perror("error en listen");
    	exit(1);
  	}

	while(1)
	{
		// Ejecutamos accept() para recibir conexiones entrantes
		addr_len = sizeof(struct sockaddr_in);
    	if ( (newfd = accept(s, (struct sockaddr *)&clientaddr, &addr_len)) == -1)
      	{
		      perror("error en accept");
		      exit(1);
	    }
	 	printf  ("server:  conexion desde:  %s\n", inet_ntoa(clientaddr.sin_addr));

		// Leemos mensaje de cliente
		if( (n =read(newfd,buffer,128)) == -1 )
		{
			perror("Error leyendo mensaje en socket");
			exit(1);
		}
		buffer[n]=0;
		printf("Recibi %d bytes.:%s\n",n,buffer);

		// Enviamos mensaje a cliente
    	if (write (newfd, "hola", 5) == -1)
    	{
      		perror("Error escribiendo mensaje en socket");
      		exit (1);
    	}

		// Cerramos conexion con cliente
    	close(newfd);
	}

	return 0;
}

