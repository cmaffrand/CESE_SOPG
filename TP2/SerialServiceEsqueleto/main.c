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

#include <signal.h>

char lineState[NUMBER_OF_LINES] = {0, 0, 0, 0};
pthread_mutex_t mutexLineState = PTHREAD_MUTEX_INITIALIZER;

void recibiSignal(int sig)
{
	if ((sig == SIGINT) || (sig = SIGTERM))
	{
		printf(CLOSE_MSG);
		exit(EXIT_SUCCESS);
	}
}

void bloquearSign(void)
{
	sigset_t set;
	int s;
	sigemptyset(&set);
	sigaddset(&set, SIGINT);
	sigaddset(&set, SIGTERM);
	pthread_sigmask(SIG_BLOCK, &set, NULL);
}

void desbloquearSign(void)
{
	sigset_t set;
	int s;
	sigemptyset(&set);
	sigaddset(&set, SIGINT);
	sigaddset(&set, SIGTERM);
	pthread_sigmask(SIG_UNBLOCK, &set, NULL);
}

void *tpcInterface(void *param)
{
	socklen_t addr_len;
	struct sockaddr_in clientaddr;
	struct sockaddr_in serveraddr;
	char buffer[TCP_MAX_CHARS];
	int n;
	int newfd;
	char tcpFrame[TCP_MAX_CHARS] = TCP_MSG;
	char tcpState[NUMBER_OF_LINES] = {0, 0, 0, 0};
	char tcpStateLast[NUMBER_OF_LINES] = {0, 0, 0, 0};

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
	}

	// Abrimos puerto con bind()
	if (bind(s, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) == -1)
	{
		close(s);
		perror("listener: bind\r\n");
		exit(EXIT_FAILURE);
	}

	// Seteamos socket en modo Listening
	if (listen(s, 10) == -1) // backlog=10
	{
		perror("error en listen\r\n");
		exit(EXIT_FAILURE);
	}

	while (1)
	{
		// Ejecutamos accept() para recibir conexiones entrantes
		addr_len = sizeof(struct sockaddr_in);
		if ((newfd = accept(s, (struct sockaddr *)&clientaddr, &addr_len)) == -1)
		{
			perror("error en accept\r\n");
			exit(EXIT_FAILURE);
		}
		printf("%s: ", (const char *)param);
		printf("conexion desde:  %s -> ", inet_ntoa(clientaddr.sin_addr));
		printf("Puerto: %d -> ", TCP_PORT);
		printf("FileDescriptor: %d\r\n", newfd);

		// Leemos mensaje de cliente
		if ((n = read(newfd, buffer, TCP_MAX_CHARS)) == -1)
		{
			perror("Error leyendo mensaje en socket\r\n");
			exit(EXIT_FAILURE);
		}
		buffer[n] = 0;
		printf("%s: ", (const char *)param);
		printf("Status Recibido: %s", buffer);

		for (char j = 0; j < NUMBER_OF_LINES; j++)
		{
			tcpState[j] = buffer[TCP_DATAIN + j] - '0';
			// Si existe un cambio en la web
			if (tcpStateLast[j] != tcpState[j])
			{
				pthread_mutex_lock(&mutexLineState);
				lineState[j] = tcpState[j];
				pthread_mutex_unlock(&mutexLineState);
			}
			else
			{
				pthread_mutex_lock(&mutexLineState);
				// Si existe un cambio en UART
				while (lineState[j] != tcpState[j])
				{
					tcpFrame[DATAOUT_CHAR] = '0' + j;
					tcpState[j]++;
					if (tcpState[j] > 2)
					{
						tcpState[j] = 0;
					}
					// Enviamos mensaje a cliente
					if (write(newfd, tcpFrame, TCP_MSG_LENGTH) == -1)
					{
						perror("Error escribiendo mensaje en socket\r\n");
						exit(EXIT_FAILURE);
					}
					printf("%s: ", (const char *)param);
					printf("Comando enviado a Web originado en UART: %.*s", (int)TCP_MSG_LENGTH, tcpFrame);
					sleep(1);
				}
				pthread_mutex_unlock(&mutexLineState);
			}
			tcpStateLast[j] = tcpState[j];
		}
		// Cerramos conexion con cliente
		close(newfd);
	}
	exit(EXIT_FAILURE);
}

void *serialInterface(void *param)
{
	int serialPort, serialStatus;
	int initPort = 0;
	char serialBufferIn[SERIAL_MAX_CHARS];
	char serialBufferOut[SERIAL_MAX_CHARS] = SERIAL_OUTPUT_INIT;
	char ledState[NUMBER_OF_LINES] = {0, 0, 0, 0};

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
			// Recibe comando de Toggle
			if (serialBufferIn[INIT_CHAR] == 'T')
			{
				printf("%s: ", (const char *)param);
				printf("Comando valido recibido: %.*s -> ", serialStatus - 2, serialBufferIn);
				printf("serialStatus: %d\r\n", serialStatus);
				ledState[serialBufferIn[DATAIN_CHAR] - '0']++;
				if (ledState[serialBufferIn[DATAIN_CHAR] - '0'] > 2)
				{
					ledState[serialBufferIn[DATAIN_CHAR] - '0'] = 0;
				}
				pthread_mutex_lock(&mutexLineState);
				lineState[serialBufferIn[DATAIN_CHAR] - '0'] = ledState[serialBufferIn[DATAIN_CHAR] - '0'];
				pthread_mutex_unlock(&mutexLineState);
				serialBufferOut[LED0_CMD_CHAR] = ledState[0] + '0';
				serialBufferOut[LED1_CMD_CHAR] = ledState[1] + '0';
				serialBufferOut[LED2_CMD_CHAR] = ledState[2] + '0';
				serialBufferOut[LED3_CMD_CHAR] = ledState[3] + '0';
				serial_send(serialBufferOut, SERIAL_OUTPUT_NUMBER_CHARS);
				printf("%s: ", (const char *)param);
				printf("Comando enviado a EDU CIAA originado en UART: %.*s", SERIAL_OUTPUT_NUMBER_CHARS, serialBufferOut);
			}
			// Recibe ACK
			else if (serialBufferIn[INIT_CHAR] == 'O')
			{
				printf("%s: ", (const char *)param);
				printf("ACK recibido: %.*s -> ", serialStatus - 2, serialBufferIn);
				printf("serialStatus: %d\r\n", serialStatus);
			}
		}
		else
		{
			// Actualiza estado de las lineas ante cambios en TCP
			int change = 0;
			for (char i = 0; i < NUMBER_OF_LINES; i++)
			{
				pthread_mutex_lock(&mutexLineState);
				if (ledState[i] != lineState[i])
				{
					ledState[i] = lineState[i];
					serialBufferOut[LED0_CMD_CHAR + i * 2] = ledState[i] + '0';
					change = 1;
				}
				pthread_mutex_unlock(&mutexLineState);
			}
			if (change == 1)
			{
				serial_send(serialBufferOut, SERIAL_OUTPUT_NUMBER_CHARS);
				printf("%s: ", (const char *)param);
				printf("Comando enviado a EDU CIAA originado en TCP: %.*s", SERIAL_OUTPUT_NUMBER_CHARS, serialBufferOut);
			}
		}
		sleep(1);
	}

	printf("%s: ", (const char *)param);
	printf("Puerto serie no pudo ser inicializado -> ");
	printf("serialPort: %d\r\n", serialPort);
	exit(EXIT_FAILURE);
}

int main(void)
{
	int serialThreadCheck, tcpThreadCheck;
	const char *message1 = "Thread Serie";
	const char *message2 = "Thread TCP";
	struct sigaction sa;
	pthread_t serialThread, tcpThread;

	printf("Main: Inicio Serial Service\r\n");

	// Manejo de seniales
	sa.sa_handler = recibiSignal;
	sa.sa_flags = SA_RESTART;
	sigemptyset(&sa.sa_mask);
	if (sigaction(SIGINT, &sa, NULL) == -1)
	{
		perror("sigaction\r\n");
		exit(EXIT_FAILURE);
	}
	if (sigaction(SIGTERM, &sa, NULL) == -1)
	{
		perror("sigaction\r\n");
		exit(EXIT_FAILURE);
	}

	// Creacion de threads
	bloquearSign();
	serialThreadCheck = pthread_create(&serialThread, NULL, serialInterface, (void *)message1);
	if (serialThreadCheck != 0)
	{
		printf("No se pudo crear el thread del puerto serie\r\n");
		exit(EXIT_FAILURE);
	}
	printf("Main: Thread del puerto serie creado\n\r");
	tcpThreadCheck = pthread_create(&tcpThread, NULL, tpcInterface, (void *)message2);
	if (tcpThreadCheck != 0)
	{
		printf("No se pudo crear el thread del TCP\r\n");
		exit(EXIT_FAILURE);
	}
	printf("Main: Thread TCP creado\r\n");
	desbloquearSign();

	// No se hace nada en el main más que esperar las señales
	while (1)
	{
		sleep(100);
	}
	exit(EXIT_FAILURE);
}
