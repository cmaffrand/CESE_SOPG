
int serial_open(int pn, int baudrate);
void serial_send(char *pData, int size);
void serial_close(void);
int serial_receive(char *buf, int size);

#define PORT_CIAA 1
#define PORT_BAUDRATE 115200
#define SERIAL_MAX_CHARS 20
#define INIT_CHAR 1
#define DATAIN_CHAR 14
#define SERIAL_OUTPUT_NUMBER_CHARS 15
#define SERIAL_OUTPUT_INIT ">OUTS:0,0,0,0\r\n"
#define LED0_CMD_CHAR 6
#define LED1_CMD_CHAR 8
#define LED2_CMD_CHAR 10
#define LED3_CMD_CHAR 12

#define TCP_MSG ":LINEXTG\n\0"
#define TCP_MSG_LENGTH sizeof(TCP_MSG)
#define DATAOUT_CHAR 5
#define TCP_MAX_CHARS 128
#define TCP_PORT 10000
#define SERVER_TCP_ADDRESS "127.0.0.1"
#define TCP_DATAIN 7
#define NUMBER_OF_LINES 4
#define INVALID_FD -1

#define CLOSE_TCP_MSG "Se deconecta el Server TCP\r\n"
#define CLOSE_UART_MSG "Se cierra el puerto UART\r\n"
#define CLOSE_TCP_THREAD_MSG "Join del thread TCP\r\n"
#define CLOSE_UART_THREAD_MSG "Join del thread UART\r\n"
#define CLOSE_MSG "Se cierra el Serial Servive\r\n"
