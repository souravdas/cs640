#include "common.h"
#include "helper.h"

void initUdpServer(char *hname, char *port, int kflag);
void initUdpClient(char *hname, char *port, char *saddr);
void* recvUdpMsg(void *socket_desc);


