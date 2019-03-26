/*
 * clientFTP.c - A concurrent FTP client
 */
#include "csapp.h"
#include "interprete.h"

void message(int b_recv, double start_time) {
    double curr_time = time(NULL);
    int diff = (curr_time != start_time)
            ? curr_time - start_time
            : 1;
    int speed = b_recv/(diff*1000);

    printf("Transfer successfully complete.\n");
    printf("%d bytes received in %d seconds (%d Kbytes/s)\n", b_recv, diff, speed);
}

void ecrireFichier(int f, char* buf, int n) {
    //printf("BUFF: [%s] | TAILLE: %d\n", buf, n);
    Write(f, buf, n);
}

void my_free(char* cmd, char* arg, char* buff) {
    free(cmd);
    free(arg);
    free(buff);
}

int main(int argc, char **argv)
{
    int clientfd, port;
    char *host, buf[MAXLINE];
    rio_t rio;
    int cmd_result;
    int b_recv;

    if (argc != 2) {
        fprintf(stderr, "usage: %s <host>\n", argv[0]);
        exit(0);
    }
    host = argv[1];
    port = 2121;

    clientfd = Open_clientfd(host, port);

    printf("Connected to %s\n", host);
    printf("ftp> ");

    Rio_readinitb(&rio, clientfd);

    char* cmd = malloc(sizeof(char) * COMMAND_MAX_LENGTH);
    char* arg = malloc(sizeof(char) * COMMAND_MAX_LENGTH);
    char* buff = malloc(TAILLE_PAQUET);

    while (Fgets(buf, MAXLINE, stdin) != NULL) {
        b_recv = 0;

        getCommand(buf, cmd, arg);

        // Envoie de la commande
        Rio_writen(clientfd, buf, strlen(buf));

        // Reception de confirmation de commande valide
        cmd_result = -1;
        Rio_readnb(&rio, &cmd_result, sizeof(int));

        if (cmd_result == -1) {
            printf("\"%s\" n'est pas une commande\n", cmd);
        } else {
            /* GESTION DES COMMANDES */
            if (!strcmp(cmd, "get")) {
                // Structures de donnée
                int f = 0;
                b_recv = 0;
                double start_time = time(NULL);

                // Recevoir taille du premier paquet
                int taille = -1;
                Rio_readnb(&rio, &taille, sizeof(int));

                // Message d'erreur si taille 0
                if (taille == -1) {
                    printf("le fichier %s n'existe pas\n", arg);
                } else {
                    // Recup x paquets
                    size_t n;
                    b_recv += taille;
                    char* arg2 = malloc(strlen(arg) + 1);
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
            } else if (!strcmp(cmd, "bye")) {
                printf("Disconnected to %s\n", host);

                my_free(cmd, arg, buff);
                Close(clientfd);
                exit(0);
            } else if (!strcmp(cmd, "pwd")) {
                // Lire le chemin actuel du server
                int taille;
                Rio_readnb(&rio, &taille, sizeof(int));
                Rio_readnb(&rio, buff, taille);

                printf("Server working dir: %s\n", buff);
            } else if (!strcmp(cmd, "ls")) {
                // Lire le contenu du répertoire courant
                int taille;
                Rio_readnb(&rio, &taille, sizeof(int));
                Rio_readnb(&rio, buff, taille);
            } else if (!strcmp(cmd, "cd")) {
                
                int taille;
                char res[1];
                Rio_readnb(&rio, &taille, sizeof(int));
                Rio_readnb(&rio, res, taille);
                if (res[0] == '0') {
                    printf("Folder don't exist\n");
                } else {
                    printf("-> cd %s\n", arg);
                }
            }
        }

        printf("ftp> ");
    }

    my_free(cmd, arg, buff);
    Close(clientfd);
    exit(0);
}
