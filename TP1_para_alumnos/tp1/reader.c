#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdint.h>

#define FIFO_NAME "myfifo"
#define BUFFER_SIZE 300

int main(void)
{
    uint8_t inputBuffer[BUFFER_SIZE];
    int32_t bytesRead, returnCode, fd;
    FILE *Sing, *Log;

    // Create and initialize new files
    Sing = fopen("/tmp/Sing.txt", "w");
    fprintf(Sing, "Recieved Signals:\n");
    fclose(Sing);
    printf("Signals File Created\n");
    Log = fopen("/tmp/Log.txt", "w");
    fprintf(Log, "Recieved Data:\n");
    fclose(Log);
    printf("Data File Created\n");

    /* Create named fifo. -1 means already exists so no action if already exists */
    if ((returnCode = mknod(FIFO_NAME, S_IFIFO | 0666, 0)) < -1)
    {
        printf("Error creating named fifo: %d\n", returnCode);
        exit(1);
    }

    /* Open named fifo. Blocks until other process opens it */
    printf("waiting for writers...\n");
    if ((fd = open(FIFO_NAME, O_RDONLY)) < 0)
    {
        printf("Error opening named fifo file: %d\n", fd);
        exit(1);
    }

    /* open syscalls returned without error -> other process attached to named fifo */
    printf("got a writer\n");

    /* Loop until read syscall returns a value <= 0 */
    do
    {
        /* read data into local buffer */
        if ((bytesRead = read(fd, inputBuffer, BUFFER_SIZE)) == -1)
        {
            perror("read");
        }
        else
        {
            inputBuffer[bytesRead] = '\0';
            printf("reader: read %d bytes: \"%s\"\n", bytesRead, inputBuffer);
            if (inputBuffer[0] == 'D')
            {
                Log = fopen("/tmp/Log.txt", "a");
                fputs(inputBuffer,Log);
                fprintf(Log, "\n");
                fclose(Log);
            }
            else if (inputBuffer[0] == 'S')
            {
                Sing = fopen("/tmp/Sing.txt", "a");
                fputs(inputBuffer,Sing);
                fprintf(Sing, "\n");
                fclose(Sing);
            }
            
        }
    } while (bytesRead > 0);

    return 0;
}
