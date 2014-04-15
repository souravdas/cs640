#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

void printinterror();
void printerror();
void printhelp();
int validateArgs(int l, int k, int s, int u, char *hname, char *port);
int GetIPfromString(char *ip, struct sockaddr_in *addr);


