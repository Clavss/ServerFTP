/*
 * clientFTP.c - A concurrent FTP client
 */
#include "csapp.h"
#include "interprete.h"

void message(int b_recv, double start_time) {
    double curr_time = time(NULL);
    int diff = curr_time - start_time;
    if(diff == 0)diff = 1;
    int speed = b_recv/(diff*1000);

    printf("Transfer successfully complete.\n");
    printf("%d bytes received in %d seconds (%d Kbytes/s)\n", b_recv, diff, speed);
}

void ecrireFichier(int f, char* buf, int n) {
    //printf("BUFF: [%s] | TAILLE: %d\n", buf, n);
    Write(f, buf, n);
}

int main(int argc, char **argv)
{
    int clientfd, port;
    char *host, buf[MAXLINE];
    rio_t rio;

    if (argc != 2) {
        fprintf(stderr, "usage: %s <host>\n", argv[0]);
        exit(0);
    }
    host = argv[1];
    port = 2121;

    /*
     * Note that the 'host' can be a name or an IP address.
     * If necessary, Open_clientfd will perform the name resolution
     * to obtain the IP address.
     */
    clientfd = Open_clientfd(host, port);

    /*
     * At this stage, the connection is established between the client
     * and the server OS ... but it is possible that the server application
     * has not yet called "Accept" for this connection
     */
    printf("Connected to %s\n", host);
    printf("ftp> ");

    Rio_readinitb(&rio, clientfd);

    while (Fgets(buf, MAXLINE, stdin) != NULL) {
        char* cmd = malloc(sizeof(char) * COMMAND_MAX_LENGTH);
        char* arg = malloc(sizeof(char) * COMMAND_MAX_LENGTH);
        char* buff = malloc(TAILLE_PAQUET);
        int b_recv = 0;
        getCommand(buf, cmd, arg);

        // MODIFIER CE TRUC
        if(arg[strlen(arg)-1] == '\n')arg[strlen(arg)-1] = '\0';

        if (!strcmp(cmd, "get")) {
            //structures de donnée
            int f = 0;
            b_recv = 0;
            double start_time = time(NULL);

            //envoie de la commande
            Rio_writen(clientfd, buf, strlen(buf));

            //recevoir taille du premier paquet
            int taille = -1;
            Rio_readnb(&rio, &taille, sizeof(int));

            // Message d'erreur si taille 0
            if (taille == -1) {
                printf("le fichier %s n'existe pas\n", arg);
            } else {
                // Recup x paquets
                size_t n;
                b_recv += taille;
                char* arg2 = malloc(strlen(arg)+1);
                strcpy(arg2, "_");
                strcat(arg2, arg);

                f = Open(arg2, O_WRONLY | O_CREAT, 00400 | 00200);

                // Recup premier paquet
                Rio_readnb(&rio, buff, taille);
                ecrireFichier(f, buff, taille);

                // Recuperer tous les paquets
                // On obtient la taille du paquet à la volée
                int fin = 0;
                while (!fin && (n = Rio_readnb(&rio, &taille, sizeof(int))) != 0) {
                    if (taille == -1) {
                        fin = 1;
                    } else {
                        Rio_readnb(&rio, buff, taille);
                        ecrireFichier(f, buff, taille);
                        b_recv += taille;
                    }
                }
                Close(f);
                message(b_recv, start_time);
            }
        } else if (!strcmp(cmd, "echo")) {
            Rio_writen(clientfd, buf, strlen(buf));
            if (Rio_readlineb(&rio, buf, MAXLINE) > 0) {
                Fputs(buf, stdout);
            } else { /* the server has prematurely closed the connection */
                break;
            }
        } else {
            printf("\"%s\" n'est pas une commande\n", cmd);
        }

        printf("ftp> ");
    }
    Close(clientfd);
    exit(0);
}
