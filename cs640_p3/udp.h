#include "common.h"
#include "helper.h"

typedef struct params_
{
    int socket_desc;
    struct sockaddr_in remaddr;
    socklen_t len;
}params; 

void initUdpServer(char *hname, char *port, int kflag);
void initUdpClient(char *hname, char *port, char *saddr);
void* recvUdpMsg(void *socket_desc);


