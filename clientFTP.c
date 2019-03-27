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

char* rand_color() {
    int color = rand()%6 + 1;
    switch (color) {
        case 1:
            return RED;
        case 2:
            return GREEN;
        case 3:
            return YELLOW;
        case 4:
            return BLUE;
        case 5:
            return MAGENTA;
        case 6:
            return CYAN;
        default:
            return RESET;
    }
}

int main(int argc, char **argv)
{
    srand(time(NULL));

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
    printf("%sftp> " RESET, rand_color());

    Rio_readinitb(&rio, clientfd);

    while (Fgets(buf, MAXLINE, stdin) != NULL) {
        char* cmd = malloc(sizeof(char) * COMMAND_MAX_LENGTH);
        char* arg = malloc(sizeof(char) * COMMAND_MAX_LENGTH);
        char* buff = malloc(TAILLE_PAQUET);
        buff[0] = '\0';
        b_recv = 0;

        getCommand(buf, cmd, arg);

        // Envoie de la commande
        Rio_writen(clientfd, buf, strlen(buf));

        // Reception de confirmation de commande valide
        cmd_result = -1;
        Rio_readnb(&rio, &cmd_result, sizeof(int));

        if (cmd_result == -1) {
            printf("\"%s\" isn't a command\n", cmd);
        } else {

            /* GESTION DES COMMANDES */
            /* GET */
            if (!strcmp(cmd, "get")) {
                // Structures de donnée
                int f = 0;
                b_recv = 0;
                double start_time = time(NULL);

                // Envoie de la taille du fichier chez le client
                int f_length = file_length(arg);
                Rio_writen(clientfd, &f_length, sizeof(int));

                int same_length;
                Rio_readnb(&rio, &same_length, sizeof(int));

                if (f_length != 0 || !same_length) {
                    f = Open(arg, O_WRONLY | O_CREAT, 00400 | 00200);
                    if (!same_length) {
                        lseek(f, 0, SEEK_END);
                    }
                }

                // Recevoir taille du premier paquet
                int taille = -1;
                Rio_readnb(&rio, &taille, sizeof(int));

                // Message d'erreur si taille 0
                if (taille == -1) {
                    printf("The file %s don't exist\n", arg);
                } else {
                    // Recup x paquets
                    size_t n;
                    b_recv += taille;

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
            }
            
            /* BYE */
            else if (!strcmp(cmd, "bye")) {
                printf("Disconnected to %s\n", host);

                my_free(cmd, arg, buff);
                Close(clientfd);
                exit(0);
            }
            
            /* PWD */
            else if (!strcmp(cmd, "pwd")) {
                // Lire le chemin actuel du server
                int taille = recvPacket(&rio, buff);
                printf("Server working dir: %.*s\n", taille, buff);
            }
            
            /* LS */
            else if (!strcmp(cmd, "ls")) {
                // Lire le contenu du répertoire courant
                int taille = recvPacket(&rio, buff);
                printf("%.*s\n", taille, buff);
            }
            
            /* CD */
            else if (!strcmp(cmd, "cd")) {
                char res[1];
                recvPacket(&rio, res);
                if (res[0] == '0') {
                    printf("Folder don't exist\n");
                } else {
                    printf("-> moving to %s\n", arg);
                }
            }

            /* MKDIR */
            else if (!strcmp(cmd, "mkdir")) {
                char res[1];
                recvPacket(&rio, res);
                if (res[0] == '0') {
                    printf("Error while creating folder \"%s\"\n", arg);
                } else if (res[0] == '1') {
                    printf("-> folder \"%s\" created\n", arg);
                } else {
                    printf("%s\n", res);
                }
            }

            /* RM */
            else if (!strcmp(cmd, "rm")) {
                char res[1];
                recvPacket(&rio, res);

                if (arg[0] == '-') {
                    char *arg1 = malloc(sizeof(char) * 2);
                    char *arg2 = malloc(sizeof(char) * (strlen(arg)-3));
                    getCommand(arg, arg1, arg2);

                    if (res[0] == '0') {
                        printf("Error while removing folder \"%s\"\n", arg2);
                    } else if (res[0] == '1') {
                        printf("-> folder \"%s\" removed\n", arg2);
                    } else {
                        printf("Error: %s\n", res);
                    }

                    free(arg1);
                    free(arg2);
                } else {
                    if (res[0] == '0') {
                        printf("Error while removing file \"%s\"\n", arg);
                    } else if (res[0] == '1') {
                        printf("-> file \"%s\" removed\n", arg);
                    } else {
                        printf("Error: %s\n", res);
                    }
                }
            }
        }

        my_free(cmd, arg, buff);
        printf("%sftp> " RESET, rand_color());
    }

    Close(clientfd);
    exit(0);
}
