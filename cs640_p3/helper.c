#include "helper.h"

void printhelp()
{
     printf("Usage: nc [-l] [-k] [-s source_ip_address] [-u] [hostname] [port] \n l : Used to specify that netcat should listen for an incoming connection rather than initiate a connection. In other words, specifies that netcat should behalf as a server rather than a client. \n k: Forces netcat to stay listening for another connection after its current connection is completed. This option can only be used when the -l option is also given. \n s: Specifies the IPv4 address in dotted decimal notation (e.g., 128.105.112.1) for the interface which should be used to send packets. This option can only be used when the -l option is not given.\n u: Indicates that UDP should be used instead of TCP. \n hostname: If the -l option is not given, then hostname specifies the IPv4 address in dotted decimal notation (e.g., 128.105.112.1) or the hostname (e.g., mumble-01.cs.wisc.edu) that should be used by the client instance to contact the server instance. Otherwise, hostname specifies the IPv4 address or the hostname on which the server instance should listen for packets/connections. If the -l option is not given, the hostname argument is required; otherwise it is optional. The hostname argument must be the second to last argument, if it is included.\n port: If the -l option is not given, then port specifies the numerical port number (e.g., 9999) on which the client instance should contact the server instance. Otherwise, port specifies the port on which the server instance should listen for packets/connections. The port argument is always required; it should be the last argument.\n ");
}

void printerror()
{
    printf("invalid or missing options\nusage: snc [-k] [-l] [-u] [-s source_ip_address] [hostname] port\n");
}

void printinterror()
{
    printf("internal error\n");
}

int validateArgs(int l, int k, int s, int u, char *hname, char *port)
{
    char *error = "invalid or missing options\nusage: snc [-k] [-l] [-u] [-s source_ip_address] [hostname] port\n";
    int portno = atoi(port);

    if (k && !l)
    {
        printf("%s",error);
        return 0;
    }
    
    if (s && l)
    {
        printf("%s",error);
        return 0;
    }
    if (!l && !hname)
    {
        printf("%s",error);
        return 0;
    }
    if (!port)
    {
        printf("%s",error);
        return 0;   
    }
    /* Validate Ports */
    if (portno < 1025 || portno > 65535)
    {
        printerror();
        return 0;
    }
    return 1;
}

int GetIPfromString(char *ip, struct sockaddr_in *addr)
{
    int result = inet_pton(AF_INET, ip, (&(addr->sin_addr)));
    return (result != 0);
}

