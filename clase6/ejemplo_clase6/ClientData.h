#include <pthread.h>
struct S_ClientData
{
	int fd;
	pthread_t thread;
	int flagFree;
};
typedef struct S_ClientData ClientData;

void cd_init(ClientData* clients, int len);
int cd_getFreeIndex(ClientData* clients, int len);

