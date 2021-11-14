#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdint.h>
#include <unistd.h>
#include <signal.h>

#define FIFO_NAME "myfifo"
#define WRITE_HEADER "DATA:"
#define MSG_SIZE 300
#define BUFFER_SIZE MSG_SIZE - strlen(WRITE_HEADER)
#define SIGNAL_SIGUSR1_MSG "SIGN:1"
#define SIGNAL_SIGUSR2_MSG "SIGN:2"

int32_t fd;

void recibiSignal(int sig)
{
    uint32_t bytesWrote;
    if (sig == SIGUSR1)
    {
        if ((bytesWrote = write(fd, SIGNAL_SIGUSR1_MSG, strlen(SIGNAL_SIGUSR1_MSG))) == -1)
        {
            perror("write");
        }
        else
        {
            printf("writer: write %d bytes: %s\n", bytesWrote, SIGNAL_SIGUSR1_MSG);
        }
    }
    else if (sig = SIGUSR2)
    {
        if ((bytesWrote = write(fd, SIGNAL_SIGUSR2_MSG, strlen(SIGNAL_SIGUSR2_MSG))) == -1)
        {
            perror("write");
        }
        else
        {
            printf("writer: write %d bytes: %s\n", bytesWrote, SIGNAL_SIGUSR2_MSG);
        }
    }
}

int main(void)
{

    char outputBuffer[BUFFER_SIZE];
    char outputMsg[MSG_SIZE];
    uint32_t bytesWrote;
    int32_t returnCode;
    struct sigaction sa;

    /* Create named fifo. -1 means already exists so no action if already exists */
    if ((returnCode = mknod(FIFO_NAME, S_IFIFO | 0666, 0)) < -1)
    {
        printf("Error creating named fifo: %d\n", returnCode);
        exit(1);
    }

    /* Open named fifo. Blocks until other process opens it */
    printf("waiting for readers...\n");
    if ((fd = open(FIFO_NAME, O_WRONLY)) < 0)
    {
        printf("Error opening named fifo file: %d\n", fd);
        exit(1);
    }

    /* open syscalls returned without error -> other process attached to named fifo */
    printf("got a reader--type some stuff\n");
    sa.sa_handler = recibiSignal;
    sa.sa_flags = SA_RESTART; //0
    // Con SA_RESTART vuelve a ejecutar.
    // Con 0 sigue ejecutando.
    sigemptyset(&sa.sa_mask);
    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGUSR2, &sa, NULL);

    /* Loop forever */
    while (1)
    {
        memset(outputMsg,0,strlen(outputMsg));
        memcpy(outputMsg, WRITE_HEADER, strlen(WRITE_HEADER));

        /* Get some text from console */
        fgets(outputBuffer, BUFFER_SIZE, stdin);
        // fgets error handle

        // Concatenates Header with Bufffer
        strcat(outputMsg, outputBuffer);

        /* Write buffer to named fifo. Strlen - 1 to avoid sending \n char */
        if ((bytesWrote = write(fd, outputMsg, strlen(outputMsg) - 1)) == -1)
        {
            perror("write");
        }
        else
        {
            printf("writer: write %d bytes: %s\n", bytesWrote, outputMsg);
        }
    }
    return 0;
}
