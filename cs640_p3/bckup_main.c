#include "main.h"

int main (int argc, char* argv[])
{
    printf("Chat Application \n");
    int index = 0, c = 0, lflag = 0, kflag = 0, sflag = 0, uflag = 0;
    char *saddr = NULL, *hname = NULL, *port = NULL;

    /*Parse the Options*/
    opterr = 0;
    while ((c = getopt(argc, argv, "lkuhs:")) != -1)
    {
        switch (c)
        {
            case 'l':
                lflag = 1;
                break;
            case 'k':
                kflag = 1;
                break;
            case 's':
                sflag = 1;
                saddr = optarg;
                break;
            case 'u':
                uflag = 1;
                break;
            case 'h':
                printhelp();
                return 1;
            case '?':
                if (optopt == 's')
                    printf("[Error:] Option -%c requires an argument.\n", optopt);                 
                else if (isprint(optopt))
                    printf("[Error:] Unknown Option '-%c'.\n", optopt);
                else
                    printf("[Error:] Unknown Option Character '\\x%x'.\n", optopt);
                return -1;
            default:
                abort();
        }
    }
    
    for (index = optind; index < argc; index++)
    {
        if ((argc - index) == 0 || (argc - index) > 2)
        {
            printf("[Error:] Specify the hostname[optional] and port correctly \n ");
            return -1;
        }
        if ((argc - index) == 2)
            hname = argv[index];
        else
            port = argv[index];   
    }

    if (!validateArgs(lflag, kflag, sflag, uflag, hname, port))
    {
       return -1;
    }

    printf(" [Debug:] lflag = %d, kflag = %d, sflag = %d, uflag = %d, saddr = %s \n", lflag, kflag, sflag, uflag, saddr);

    if (lflag && !uflag)
        initTcpServer(hname, port, kflag); 
    else if (lflag && uflag)
        initUdpServer(hname, port, kflag);
    else if (!lflag && !uflag)
        initTcpClient(hname, port);
    else
        initUdpClient(hname, port);

    return 1;
	
	
}

void initTcpServer(char *hname, char *port, int kflag)
{
    struct sockaddr_in addr;
    int i,socket_desc , new_socket , c, *new_sock;
    struct sockaddr_in server , client;
    pthread_t sniffer_thread;
    char buffer[2000];
    struct hostent *he;
    struct in_addr **addr_list;

    if (hname)
    {
        printf("[hostname]: %s\n",hname);
        if (!GetIPfromString(hname, &addr))
        {
            if ( (he = gethostbyname( hname ) ) == NULL) 
            {
                //gethostbyname failed
                printf("gethostbyname failed \n");
                return;
            }  
            addr_list = (struct in_addr **) he->h_addr_list;
            for(i = 0; addr_list[i] != NULL; i++) 
            {
                addr.sin_addr = *addr_list[i];
            }
 
        }
        else
            printf("[debug:] GetIPfromString - IPAdress is %s\n", inet_ntoa(addr.sin_addr));
        server.sin_addr = addr.sin_addr;
    }
    else
        server.sin_addr.s_addr = INADDR_ANY;


    /*Create socket*/
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printf("[Error:] Could not create socket\n");
    }

    /*Prepare the sockaddr_in structure*/
    server.sin_family = AF_INET;
    server.sin_port = htons( atoi(port) );
    
    /*Bind*/
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        printf("[Error:] Bind failed\n");
        return;
    }
    printf("Bind done\n");

    /*listen*/
    listen(socket_desc , 3);
    
    /*Accept and incoming connection*/
    printf("Waiting for incoming connections...\n");
    c = sizeof(struct sockaddr_in);
    while( (new_socket = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) )
    {
        printf("Connection accepted\n");
        new_sock = malloc(1);
        *new_sock = new_socket;
        if( pthread_create( &sniffer_thread , NULL , recvMsg , (void*) new_sock) < 0)
        {
           printf("[Error:] Could not create listen thread\n");
           return;
        }
        while (fgets(buffer, 2000, stdin) != NULL)
        {
            
            if( send(new_socket ,  buffer , strlen(buffer) , 0) < 0)
            {
                printf("[Error:] Send failed\n");
                return;
            }
            //puts("Data Send\n");
        }

        pthread_cancel(sniffer_thread);
        
        if (!kflag)
            return;
    }  
}

void initTcpClient(char *hname, char *port)
{
    int socket_desc, *new_sock, i;
    struct sockaddr_in server;
    char buffer[2000];

    struct sockaddr_in addr;
    struct hostent *he;
    struct in_addr **addr_list;
        
    pthread_t sniffer_thread;

    if (hname)
    {
        printf("[Debug]: Hostname %s\n",hname);
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
        printf("Connecting to Dst IPAdress: %s, Port: %d \n", inet_ntoa(addr.sin_addr), (atoi(port)));
        server.sin_addr = addr.sin_addr;
    }

    /*Create Socket*/
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printf("[Error:] Could not create socket");
    }

    server.sin_addr = addr.sin_addr ;
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(port) );

    
    if (connect(socket_desc , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        printf("{Error:] connect error\n");
        return ;
    }
    
    puts("Connected\n");
    new_sock = malloc(1);
    *new_sock = socket_desc;
    if( pthread_create( &sniffer_thread , NULL , recvMsg , (void*) new_sock) < 0)
    {
       printf("[Error:] Could not create listen thread\n");
       return;
    }

    while (fgets(buffer, 2000, stdin) != NULL)
    {
        if( send(socket_desc , buffer , strlen(buffer) , 0) < 0)
        {
            printf("Send failed\n");
            return;
        }
        //puts("Data Sent");
    }
    
    pthread_cancel(sniffer_thread);
    
}

void initUdpClient(char *hname, char *port)
{
    char buffer[MAXLEN];
    int *new_sock, socket_desc, i;
    struct sockaddr_in server, addr, myaddr;
    struct hostent *he;
    struct in_addr **addr_list;
    pthread_t listener_thread;
    recvUdpMsgparams *params;

    socklen_t len = sizeof(server);
    memset((char *)&server,sizeof(struct sockaddr_in),0);

    if (hname)
    {
        printf("[Debug]: Hostname %s\n",hname);
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
        printf("Connecting to Dst IPAdress: %s, Port: %d \n", inet_ntoa(addr.sin_addr), atoi(port));
        server.sin_addr = addr.sin_addr;
    }
    
    /*Create Socket*/
    socket_desc = socket(AF_INET ,SOCK_DGRAM , 0);
    if (socket_desc == -1)
    {
        printf("[Error:] Could not create socket");
    }
    else
        printf("Udp Client Socket Created Successfully\n");

    
    /*server.sin_addr = addr.sin_addr ;*/
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(port));

    memset((char *)&myaddr, 0, sizeof(myaddr)); 
    myaddr.sin_family = AF_INET; 
    myaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    myaddr.sin_port = htons(0);

    if (bind(socket_desc, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) 
    { 
        printf("bind failed\n"); 
        return 0; 
    }
    
    new_sock = malloc(1);
    *(new_sock) = socket_desc;
   
    if (pthread_create(&listener_thread , NULL , recvUdpMsg , (void*) new_sock) < 0)
    {
        printf("[Error:] Could not create listen thread\n");
        return;
    }

    while (fgets(buffer, 2000, stdin) != NULL)
    {
        if (sendto(socket_desc , buffer , strlen(buffer) , 0, (SA*)&server, len) < 0)
        {
            printf("Send failed\n");
            return;
        }
        //puts("Data Sent");
    }
   
    pthread_cancel(listener_thread);
       
}

void initUdpServer(char *hname, char *port, int kflag)
{
    char buffer[MAXLEN];
    int *new_sock, socket_desc, i;
    struct sockaddr_in server, addr, remaddr;
    struct hostent *he;
    struct in_addr **addr_list;
    pthread_t listener_thread;
    recvUdpMsgparams *params;

    socklen_t len = sizeof(server);    
    memset(&server,sizeof(struct sockaddr_in),0);

    if (hname)
    {
        printf("[Debug]: Hostname %s\n",hname);
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
        printf("Connecting to Dst IPAdress: %s, Port: %d \n", inet_ntoa(addr.sin_addr), htons(atoi(port)));
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
    else
        printf("Udp Client Socket Created Successfully\n");


    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(port));

    params = (recvUdpMsgparams*) malloc(sizeof(recvUdpMsgparams));
    params->server = (struct sockaddr_in*)malloc(sizeof(struct sockaddr_in));
    new_sock = malloc(1);
   
    
    /*Bind*/
    if( bind(socket_desc,(SA *)&server , sizeof(server)) < 0)
    {
        printf("[Error:] Bind failed\n");
        return;
    }
    printf("Bind done\n");
    printf("Waiting for the first packet\n");
    
    *new_sock = socket_desc;
   
    while (1)
    { 
        if(recvfrom(socket_desc ,buffer , MAXLEN, 0, (SA*)&remaddr, &len)>0)
        {
            printf("Recv Udp Msg: %s", buffer);
       
            if (pthread_create(&listener_thread , NULL , recvUdpMsg, (void*) new_sock) < 0)
            {
                printf("[Error:] Could not create listen thread\n");
                return;
            }
        }

        while (fgets(buffer, 2000, stdin) != NULL)
        {
            if (sendto(socket_desc , buffer , strlen(buffer) , 0, (SA*)&remaddr, len) < 0)
            { 
               printf("Send failed\n");
               return;
            }
        //puts("Data Sent");
        }
        pthread_cancel(listener_thread);
    }
    close(socket_desc);
}

void* recvMsg(void *socket_desc)
{
    int sock = *(int*)socket_desc;
    while (1)
    {
        char server_reply[2000];
        if(recv(sock, server_reply , 2000 , 0)>0)
        {
            printf("Recv Msg: %s", server_reply);
        }
    }
}

void* recvUdpMsg(void *socket_desc)
{
    int sock = *(int*)socket_desc;
    while (1)
    {
        char server_reply[MAXLEN];
        if(recvfrom(sock, server_reply , MAXLEN,0, NULL, NULL)>0)
        {
            printf("Recv Udp Msg: %s", server_reply);
        }
    }
}

