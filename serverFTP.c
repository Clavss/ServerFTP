/*
 * serverFTP.c - A concurrent FTP server
 */

#include "csapp.h"

#define MAX_NAME_LEN 256
#define NMAX 10

int parent_pid;

void interprete(int connfd);
void repartiteur(int connfd, char* ip);

void handler(int sig) {
	kill(-parent_pid, sig);
    exit(0);
}

void init_pool(int port, int listenfd, int is_master, char** ips, int nb_ip) {
    int  connfd;
    socklen_t clientlen;
    struct sockaddr_in clientaddr;
    char client_ip_string[INET_ADDRSTRLEN];
    char client_hostname[MAX_NAME_LEN];

    for (int i = 0; i < NMAX; i++) {
        if (Fork() == 0) { /* FILS */
            int current_ip = i;

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

                if (is_master) {
                    // Incrementer l'ip dans le processus père
                    current_ip = (current_ip+1) %nb_ip;
                    repartiteur(connfd, ips[current_ip]);
                } else {
                    interprete(connfd);
                }

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
    //
    if (argc < 2) {
        fprintf(stderr, "usage: %s <slave:0>\n", argv[0]);
        fprintf(stderr, "or\n");
        fprintf(stderr, "usage: %s <master:1> <ip_slave1> <ip_slave2> ...\n", argv[0]);
        exit(0);
    }
    parent_pid = getpid();

    int is_master = atoi(argv[1]);
    int port = 2121;
    int listenfd = Open_listenfd(port);

    char** ips = NULL;
    if (is_master) {
        ips = malloc(sizeof(char*) * (argc-2));
        for (int i = 0; i < argc-2; i++) {
            ips[i] = argv[i+2];
        }
    }

    init_pool(port, listenfd, is_master, ips, argc-2);
    Signal(SIGINT, handler);

    while(1);

    exit(0);
}
