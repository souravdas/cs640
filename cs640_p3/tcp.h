#include "common.h"
#include "helper.h"

void initTcpServer(char *hname, char *port, int kflag);
void initTcpClient(char *hname, char *port, char *saddr);
void* recvMsg(void *socket_desc);

