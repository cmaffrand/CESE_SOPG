#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "SerialManager.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

void *tpcInterface(void *param)
{
	socklen_t addr_len;
	struct sockaddr_in clientaddr;
	struct sockaddr_in serveraddr;
	char buffer[TCP_MAX_CHARS];
	int newfd;
	int n;
	int toggleLine = 0;
	char tcpFrame[TCP_MAX_CHARS] = TCP_MSG;

	// Creamos socket
	int s = socket(PF_INET, SOCK_STREAM, 0);

	// Cargamos datos de IP:PORT del server
	bzero((char *)&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(TCP_PORT);
	serveraddr.sin_addr.s_addr = inet_addr(SERVER_TCP_ADDRESS);
	if (serveraddr.sin_addr.s_addr == INADDR_NONE)
	{
		printf("%s: ", (const char *)param);
		fprintf(stderr, "ERROR invalid server IP\r\n");
		exit(EXIT_FAILURE);
		return 0;
	}

	// Abrimos puerto con bind()
	if (bind(s, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) == -1)
	{
		close(s);
		perror("listener: bind");
		exit(EXIT_FAILURE);
		return 0;
	}

	// Seteamos socket en modo Listening
	if (listen(s, 10) == -1) // backlog=10
	{
		perror("error en listen");
		exit(EXIT_FAILURE);
	}

	while (1)
	{
		// Ejecutamos accept() para recibir conexiones entrantes
		addr_len = sizeof(struct sockaddr_in);
		if ((newfd = accept(s, (struct sockaddr *)&clientaddr, &addr_len)) == -1)
		{
			perror("error en accept");
			exit(EXIT_FAILURE);
		}
		printf("%s: ", (const char *)param);
		printf("conexion desde:  %s\n", inet_ntoa(clientaddr.sin_addr));

		// Leemos mensaje de cliente
		if ((n = read(newfd, buffer, 128)) == -1)
		{
			perror("Error leyendo mensaje en socket");
			exit(EXIT_FAILURE);
		}
		buffer[n] = 0;
		printf("%s: ", (const char *)param);
		printf("Recibi %d bytes.: %s", n, buffer);

		// Enviamos mensaje a cliente
		toggleLine++;
		if (toggleLine > 3)
		{
			toggleLine = 0;
		}
		tcpFrame[DATAOUT_CHAR] = toggleLine + '0';

		if (write(newfd, tcpFrame, TCP_MSG_LENGTH) == -1)
		{
			perror("Error escribiendo mensaje en socket");
			exit(EXIT_FAILURE);
		}

		// Cerramos conexion con cliente
		close(newfd);
	}

	return 0;
}

void *serialInterface(void *param)
{
	int serialPort, serialStatus;
	int initPort = 0;
	char serialBufferIn[SERIAL_MAX_CHARS];
	char serialBufferOut[SERIAL_MAX_CHARS] = SERIAL_OUTPUT_INIT;
	char ledState[4] = {0, 0, 0, 0};

	printf("%s: ", (const char *)param);
	printf("Inicio Serial Service\r\n");
	// Abre el puerto serie
	serialPort = serial_open(PORT_CIAA, PORT_BAUDRATE);

	while (serialPort == 0)
	{
		if (initPort == 0)
		{
			printf("%s: ", (const char *)param);
			printf("Puerto serie inicializado -> ");
			printf("serialPort: %d\r\n", serialPort);
			// Resetea el estado de los leds
			serial_send(serialBufferOut, SERIAL_OUTPUT_NUMBER_CHARS);
			printf("%s: ", (const char *)param);
			printf("Reset de Leds: %.*s", SERIAL_OUTPUT_NUMBER_CHARS, serialBufferOut);
			initPort = 1;
		}

		serialStatus = serial_receive(serialBufferIn, SERIAL_MAX_CHARS);
		if (serialStatus != -1 && serialStatus != 0)
		{
			if (serialBufferIn[INIT_CHAR] == 'T') // Recibe comando de Toggle
			{
				printf("%s: ", (const char *)param);
				printf("Comando valido recibido: %.*s -> ", serialStatus - 2, serialBufferIn);
				printf("serialStatus: %d\r\n", serialStatus);
				ledState[serialBufferIn[DATAIN_CHAR] - '0']++;
				if (ledState[serialBufferIn[DATAIN_CHAR] - '0'] > 2)
				{
					ledState[serialBufferIn[DATAIN_CHAR] - '0'] = 0;
				}
				serialBufferOut[LED0_CMD_CHAR] = ledState[0] + '0';
				serialBufferOut[LED1_CMD_CHAR] = ledState[1] + '0';
				serialBufferOut[LED2_CMD_CHAR] = ledState[2] + '0';
				serialBufferOut[LED3_CMD_CHAR] = ledState[3] + '0';
				serial_send(serialBufferOut, SERIAL_OUTPUT_NUMBER_CHARS);
				printf("%s: ", (const char *)param);
				printf("Comando enviado a EDU CIAA: %*s", SERIAL_OUTPUT_NUMBER_CHARS, serialBufferOut);
			}
			else if (serialBufferIn[INIT_CHAR] == 'O') // Recibe ACK
			{
				printf("%s: ", (const char *)param);
				printf("ACK recibido: %.*s -> ", serialStatus - 2, serialBufferIn);
				printf("serialStatus: %d\r\n", serialStatus);
			}
		}
		sleep(1);
	}

	printf("%s: ", (const char *)param);
	printf("Puerto serie no pudo ser inicializado -> ");
	printf("serialPort: %d\r\n", serialPort);
	exit(EXIT_FAILURE);
	return 0;
}

int main(void)
{
	pthread_t serialThread, tcpThread;
	int serialThreadCheck, tcpThreadCheck;
	const char *message1 = "Thread Serie";
	const char *message2 = "Thread TCP";

	serialThreadCheck = pthread_create(&serialThread, NULL, serialInterface, (void *)message1);
	if (serialThreadCheck != 0)
	{
		printf("No se pudo crear el thread del puerto serie");
		return 0;
	}

	tcpThreadCheck = pthread_create(&tcpThread, NULL, tpcInterface, (void *)message2);
	if (tcpThreadCheck != 0)
	{
		printf("No se pudo crear el thread del TCP");
		return 0;
	}

	// Poner en manejo de seniales
	/*printf("Espero 10 segs \n");
	sleep(10);
	printf("Espero el hilo 1\n");
	pthread_join (thing1, NULL);
	printf("Espero el hilo 2\n");
	pthread_join (thing2, NULL);*/

	while (1)
	{
		sleep(100);
	}
	return 0;
}
