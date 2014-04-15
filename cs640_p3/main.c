#include "main.h"
#include "tcp.h"

int main (int argc, char* argv[])
{
    //printf("Chat Application \n");
    int index = 0, c = 0, lflag = 0, kflag = 0, sflag = 0, uflag = 0;
    char *saddr = NULL, *hname = NULL, *port = NULL;

    /*Parse the Options*/
    opterr = 0;
    while ((c = getopt(argc, argv, "+lkuhs:")) != -1)
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
                printf("internal error\n");
                return -1;
            default:
                abort();
        }
    }
    
    for (index = optind; index < argc; index++)
    {
        if ((argc - index) == 0 || (argc - index) > 2)
        {
            //printf("[Error:] Specify the hostname[optional] and port correctly \n ");
            printerror();
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

    printf(" [Debug:] lflag = %d, kflag = %d, sflag = %d, uflag = %d, saddr = %s hname = %s, port = %s\n", lflag, kflag, sflag, uflag, saddr,hname,port);

    if (lflag && !uflag) 	
        initTcpServer(hname, port, kflag); 
    else if (lflag && uflag)
        initUdpServer(hname, port, kflag);
    else if (!lflag && !uflag)
        initTcpClient(hname, port, saddr);
    else
        initUdpClient(hname, port, saddr);

    return 1;
	
	
}
