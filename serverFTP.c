/*
 * serverFTP.c - A concurrent FTP server
 */

#include "csapp.h"

#define MAX_NAME_LEN 256
#define NMAX 10

int parent_pid;

void interprete(int connfd);

void handler(int sig) {
	kill(-parent_pid, sig);
    exit(0);
}

void init_pool(int port, int listenfd) {
    int  connfd;
    socklen_t clientlen;
    struct sockaddr_in clientaddr;
    char client_ip_string[INET_ADDRSTRLEN];
    char client_hostname[MAX_NAME_LEN];

    for (int i = 0; i < NMAX; i++) {
        if (Fork() == 0) { /* FILS */
            while (1) {
                clientlen = (socklen_t)sizeof(clientaddr);

                while((connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen)) < 0);

                /* determine the name of the client */
                Getnameinfo((SA *) &clientaddr, clientlen,
                            client_hostname, MAX_NAME_LEN, 0, 0, 0);

                /* determine the textual representation of the client's IP address */
                Inet_ntop(AF_INET, &clientaddr.sin_addr, client_ip_string,
                          INET_ADDRSTRLEN);

                printf("client connected to %s (%s) (pid:%d)\n", client_hostname,
                       client_ip_string, getpid());

                interprete(connfd);
                Close(connfd);
            }
        }
    }
}

/*
 * Note that this code only works with IPv4 addresses
 * (IPv6 is not supported)
 */
int main(int argc, char **argv)
{
    if (argc != 1) {
        fprintf(stderr, "usage: %s\n", argv[0]);
        exit(0);
    }
    parent_pid = getpid();

    int port = 2121;
    int listenfd = Open_listenfd(port);

    init_pool(port, listenfd);
    Signal(SIGINT, handler);

    while(1);

    exit(0);
}
