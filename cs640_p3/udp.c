#include "udp.h"


void* recvUdpMsg(void *socket_desc)
{
    int sock = *(int*)socket_desc;
    while (TRUE)
    {
        char server_reply[MAXLEN];
        memset(server_reply,0,MAXLEN);
        if (recvfrom(sock, server_reply , MAXLEN, 0, NULL, NULL)>0)
        {
            printf("%s", server_reply);
        }
        else
        {
            break;
        }
    }
    return NULL;
}

void* sendUdpMsg(void *args)
{
    char buffer[MAXLEN];
    params *param = (params*) args;
    int sock = param->socket_desc;

    while (fgets(buffer, MAXLEN, stdin) != NULL)
    {
        if (sendto(sock, buffer , strlen(buffer) , 0, (SA*)&(param->remaddr), param->len) < 0)
        {
            printf("Send failed\n");
            return NULL;
        }
        memset(buffer, 0, MAXLEN);
    }
    return NULL;
}

void initUdpServer(char *hname, char *port, int kflag)
{
    char buffer[MAXLEN];
    int *new_sock, socket_desc, i;
    struct sockaddr_in server, addr, remaddr;
    struct hostent *he;
    struct in_addr **addr_list;
    pthread_t listener_thread, snd_thread;
    params *args = (params*) malloc (sizeof(params));
    socklen_t len = sizeof(server);
    memset(&server,sizeof(struct sockaddr_in),0);

    if (hname)
    {
        //printf("[Debug]: Hostname %s\n",hname);
        if (!GetIPfromString(hname, &addr))
        {
            if ( (he = gethostbyname( hname ) ) == NULL)
            {

                printf("gethostbyname failed \n");
                return;
            }
            addr_list = (struct in_addr **) he->h_addr_list;
            for(i = 0; addr_list[i] != NULL; i++)
            {
                addr.sin_addr = *addr_list[i];
            }

        }
        //printf("Connecting to Dst IPAdress: %s, Port: %d \n", inet_ntoa(addr.sin_addr), htons(atoi(port)));
        server.sin_addr = addr.sin_addr;
    }
    else
        server.sin_addr.s_addr = INADDR_ANY;

    /*Create Socket*/
    socket_desc = socket(AF_INET ,SOCK_DGRAM , 0);
    if (socket_desc == -1)
    {
        printf("[Error:] Could not create socket");
    }
    //else
    //    printf("Udp Client Socket Created Successfully\n");


    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(port));

    new_sock = malloc(1);


    /*Bind*/
    if( bind(socket_desc,(SA *)&server , sizeof(server)) < 0)
    {
        printf("[Error:] Bind failed\n");
        return;
    }
    //printf("Bind done\n");
    //printf("Waiting for the first packet\n");

    *new_sock = socket_desc;
    memset(buffer,0,MAXLEN);
    while (TRUE)
    {
        if(recvfrom(socket_desc ,buffer , MAXLEN, 0, (SA*)&remaddr, &len)>0)
        {
            printf("%s", buffer);

            if (pthread_create(&listener_thread , NULL , recvUdpMsg, (void*) new_sock) < 0)
            {
                printf("[Error:] Could not create listen thread\n");
                return;
            }
        }
        args->remaddr = remaddr;
        args->len = len;
        args->socket_desc = socket_desc;

        /*while (fgets(buffer, MAXLEN, stdin) != NULL)
        {
            if (sendto(socket_desc , buffer , strlen(buffer) , 0, (SA*)&remaddr, len) < 0)
            {
               printf("Send failed\n");
               return;
            }
        //puts("Data Sent");
        }*/
        if (pthread_create(&snd_thread , NULL , sendUdpMsg, (void*) args) < 0)
        {
            printf("[Error:] Could not create listen thread\n");
            return;
        }

        pthread_join(snd_thread, NULL);
        pthread_join(listener_thread, NULL);
        pthread_cancel(listener_thread);
        
        if (!kflag)
           break; 
    }
    free(args);
    close(socket_desc);
    
}

void initUdpClient(char *hname, char *port, char *saddr)
{
    int *new_sock, socket_desc, i;
    struct sockaddr_in server, addr, myaddr;
    struct hostent *he;
    struct in_addr **addr_list;
    pthread_t listener_thread, snd_thread;
    params *args = (params*) malloc (sizeof(params));
    socklen_t len = sizeof(server);
    memset((char *)&server,sizeof(struct sockaddr_in),0);

    if (hname)
    {
        //printf("[Debug]: Hostname %s\n",hname);
        if (!GetIPfromString(hname, &addr))
        {
            if ( (he = gethostbyname( hname ) ) == NULL)
            {

                printf("gethostbyname failed \n");
                return;
            }
            addr_list = (struct in_addr **) he->h_addr_list;
            for(i = 0; addr_list[i] != NULL; i++)
            {
                addr.sin_addr = *addr_list[i];
            }

        }
        //printf("Connecting to Dst IPAdress: %s, Port: %d \n", inet_ntoa(addr.sin_addr), atoi(port));
        server.sin_addr = addr.sin_addr;
    }
    else
    {
        printf("[Error:] Why is Dst IP not given? \n");
        return;
    }

    /*Create Socket*/
    socket_desc = socket(AF_INET ,SOCK_DGRAM , 0);
    if (socket_desc == -1)
    {
        printf("[Error:] Could not create socket");
    }
    //else
    //    printf("Udp Client Socket Created Successfully\n");

    /*server.sin_addr = addr.sin_addr ;*/
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(port));
    
    /* Assign a dummy address to recv message */
    memset((char *)&myaddr, 0, sizeof(myaddr));

    if (!saddr)
        myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    else
    {
        if (!GetIPfromString(saddr, &myaddr))
        {
             printf("internal error\n");
             return;
        }
    }

    myaddr.sin_family = AF_INET;
    myaddr.sin_port = htons(0);

    if (bind(socket_desc, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0)
    {
        printf("bind failed\n");
        return;
    }

    new_sock = malloc(1);
    *(new_sock) = socket_desc;

    if (pthread_create(&listener_thread , NULL , recvUdpMsg , (void*) new_sock) < 0)
    {
        printf("[Error:] Could not create listen thread\n");
        return;
    }
    args->remaddr = server;
    args->len = len;
    args->socket_desc = socket_desc;
    /*while (fgets(buffer, MAXLEN, stdin) != NULL)
    {
        if (sendto(socket_desc , buffer , strlen(buffer) , 0, (SA*)&server, len) < 0)
        {
            printf("Send failed\n");
            return;
        }
    }
    */

    if (pthread_create(&snd_thread , NULL , sendUdpMsg, (void*) args) < 0)
    {
        printf("[Error:] Could not create listen thread\n");
        return;
    }

    pthread_join(snd_thread, NULL);
    pthread_join(listener_thread, NULL);
    close(socket_desc);
    free(args);
}

