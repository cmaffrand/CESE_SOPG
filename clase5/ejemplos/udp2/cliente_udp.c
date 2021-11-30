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

    int s = socket(PF_INET,SOCK_DGRAM, 0);

    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(4096);
    serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    if(serveraddr.sin_addr.s_addr==INADDR_NONE)
    {
        fprintf(stderr,"ERROR invalid server IP\r\n");
        return 1;
    }

    int numBytes = sendto(s,"hola",5,0, (struct sockaddr*)&serveraddr, sizeof(serveraddr) );
    printf("Se enviaron %d bytes\n",numBytes);

    printf("recibo del server:\r\n");
    char buffer[128];

    // al haber hecho un sendto antes, se hizo un bind al puerto local del cliente 
    // por el que se envio
    // entonces puedo escuchar sin haber hecho un bind del socket previamente
    
    // voy a escuchar la respuesta del packet que envie.
    numBytes = recvfrom(s,buffer,127,0, 0, 0 );
    printf("Se recibio: '%s' \r\n",buffer);

    printf("recibo del server2:\r\n");
    numBytes = recvfrom(s,buffer,127,0, 0, 0 );
    printf("Se recibio2: '%s' \r\n",buffer);


    close(s);

    return 0;
}

