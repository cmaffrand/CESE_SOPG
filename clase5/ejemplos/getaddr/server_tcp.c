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
	/*
	    bzero((char *) &serveraddr, sizeof(serveraddr));
	    serveraddr.sin_family = AF_INET;
	    serveraddr.sin_port = htons(4096);
  	    if(inet_pton(AF_INET, "127.0.0.1", &(serveraddr.sin_addr))<=0)
    	    {
        	fprintf(stderr,"ERROR invalid server IP\r\n");
        	return 1;
    	    }
	*/
	struct addrinfo hints;
	struct addrinfo *result;
	memset(&hints, 0, sizeof(struct addrinfo));
    	hints.ai_family = AF_INET; // ipv4
    	hints.ai_socktype = SOCK_STREAM; // tcp
    	hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */

	int r = getaddrinfo(NULL,"4096", &hints, &result); // NULL para localhost
    	if (r != 0) {
        	fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(r));
        	exit(EXIT_FAILURE);
    	}
	// result es lista enlazada!. En este ejemplo usamos la primera

	// Abrimos puerto con bind()
	//if (bind(s, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) == -1) {
	if (bind(s, (struct sockaddr*)result->ai_addr, result->ai_addrlen) == -1) {
		close(s);
		perror("listener: bind");
	}

	freeaddrinfo(result);

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

