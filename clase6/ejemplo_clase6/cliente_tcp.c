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
    struct sockaddr_in serveraddr;
	char buf[128];
	int s;

	// Creamos socket
	s = socket(PF_INET,SOCK_STREAM, 0);

	// Cargamos datos de direccion de server
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(4096);
    serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    if(serveraddr.sin_addr.s_addr==INADDR_NONE)
    {
        fprintf(stderr,"ERROR invalid server IP\r\n");
        return 1;
    }

	// Ejecutamos connect()
    if (connect(s, (const struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0)
    {
        fprintf(stderr,"ERROR connecting\r\n");
        close(s);
        exit(EXIT_FAILURE);
    }

	// Enviamos mensaje a server
    printf("Ingrese un mensaje:\r\n");
    fgets(buf,sizeof(buf),stdin);
    printf("envio:%s",buf);

    int n = write(s, buf, strlen(buf));
    if(n<=0)
    {
        fprintf(stderr,"ERROR sending command\r\n");
        close(s);
        exit(EXIT_FAILURE);
    }

	// Leemos respuesta de server
    n = read(s, buf, sizeof(buf));
    if(n>0)
    {
        printf("Recibi:'%s'\r\n",buf);
    }

	close(s);

	return 0;
}

