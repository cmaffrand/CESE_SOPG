#include "ClientData.h"

void cd_init(ClientData* clients, int len)
{
	int i;
	for(i=0;i<len;i++)
		clients[i].flagFree=1;
}

int cd_getFreeIndex(ClientData* clients, int len)
{
	int i;
	for(i=0;i<len;i++)
	{
		if(clients[i].flagFree)
			return i;
	}
	return -1;
}


