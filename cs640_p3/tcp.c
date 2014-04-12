#include "tcp.h"
pthread_t send_thread, recv_thread;

void* recvMsg(void *socket_desc)
{
    int sock = *(int*)socket_desc;
    while (TRUE)
    {
        char server_reply[MAXLEN];
        if(recv(sock, server_reply , MAXLEN , 0)>0)
        {
            printf("Recv Msg: %s", server_reply);
        }
        else
        {
            pthread_cancel(send_thread);
            break;
        }
        memset(server_reply,0,MAXLEN);
    }
    return NULL;
}

void* sendMsg(void *socket_desc)
{
    char buffer[MAXLEN];

    int sock = *(int*)socket_desc;
    while (fgets(buffer, MAXLEN, stdin) != NULL)
    {
        //if(buffer[0] == '\n') continue;
        if( send(sock ,  buffer , strlen(buffer) , 0) < 0)
        {
            printf("[Error:] Send failed\n");
            return NULL;
        }
        memset(buffer,0,MAXLEN);       
    }
    pthread_cancel(recv_thread);
    return NULL;
}

void initTcpClient(char *hname, char *port, char *saddr)
{
    int socket_desc, *new_sock, i;
    struct sockaddr_in server, myaddr, addr;
    struct hostent *he;
    struct in_addr **addr_list;

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
    else
    {
        printf("[Error:] Why is Dst IP not given?\n");
        return;
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


    /* Assign a dummy address to recv message */
    memset((char *)&myaddr, 0, sizeof(myaddr));

    if (!saddr)
        myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    else
    {
        if (!GetIPfromString(saddr, &myaddr))
        {
             printf("[Error:] Invalid saddr provided\n");
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


    if (connect(socket_desc , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        printf("{Error:] connect error\n");
        return ;
    }
    puts("Connected");
    new_sock = malloc(1);
    *new_sock = socket_desc;

    if( pthread_create( &recv_thread , NULL , recvMsg , (void*) new_sock) < 0)
    {
       printf("[Error:] Could not create listen thread\n");
       return;
    }
 
    if( pthread_create( &send_thread , NULL , sendMsg , (void*) new_sock) < 0)
    {
       printf("[Error:] Could not create listen thread\n");
       return;
    }
  
    pthread_join(recv_thread, NULL);
    pthread_join(send_thread, NULL);


    /*
    while (!isTerminated && fgets(buffer, MAXLEN, stdin) != NULL)
    {
        if( send(socket_desc , buffer , strlen(buffer) , 0) < 0)
        {
            printf("Send failed\n");
            return;
        }
        //puts("Data Sent");
    }
    */
    //pthread_cancel(sniffer_thread);
    shutdown(socket_desc,2);
    close(socket_desc);
}

void initTcpServer(char *hname, char *port, int kflag)
{
    struct sockaddr_in addr;
    int i,socket_desc , new_socket , c, *new_sock;
    struct sockaddr_in server , client;
    struct hostent *he;
    struct in_addr **addr_list;

    if (hname)
    {
        printf("[hostname]: %s\n",hname);
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
        if( pthread_create( &recv_thread , NULL , recvMsg , (void*) new_sock) < 0)
        {
           printf("[Error:] Could not create listen thread\n");
           return;
        }

/*        while (fgets(buffer, MAXLEN, stdin) != NULL)
        {

            if( send(new_socket ,  buffer , strlen(buffer) , 0) < 0)
            {
                printf("[Error:] Send failed\n");
                return;
            }

        }

        pthread_cancel(sniffer_thread);
*/

        if( pthread_create( &send_thread , NULL , sendMsg , (void*) new_sock) < 0)
        {
           printf("[Error:] Could not create listen thread\n");
           return;
        }

        pthread_join(recv_thread, NULL);
        pthread_join(send_thread, NULL);
        close(new_socket);
        if (!kflag)
            break;
    }
    
    //close(socket_desc);
    shutdown(socket_desc,2);
    close(socket_desc);
}


