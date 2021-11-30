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

	int s = socket(AF_INET,SOCK_DGRAM, 0);

    	bzero((char *) &serveraddr, sizeof(serveraddr));
    	serveraddr.sin_family = AF_INET;
    	serveraddr.sin_port = htons(4096);
	if(inet_pton(AF_INET, "127.0.0.1", &(serveraddr.sin_addr))<=0)
    	{
        	fprintf(stderr,"ERROR invalid server IP\r\n");
        	return 1;
    	}

    	int numBytes = sendto(s,"hola",5,0, (struct sockaddr*)&serveraddr, sizeof(serveraddr) );
	printf("Se enviaron %d bytes\n",numBytes);

	close(s);

	return 0;
}

