#include "csapp.h"
#include "interprete.h"
#define TAILLE_MAX 1000000
void interprete(int connfd)
{
    size_t n;
    char buf[MAXLINE];
    rio_t rio;

    Rio_readinitb(&rio, connfd);
    while ((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0) {
        //printf("server received %u bytes:\n", (unsigned int)n);
        //printf("%s", buf);

        char* cmd = malloc(sizeof(char) * COMMAND_MAX_LENGTH);
        char* arg = malloc(sizeof(char) * COMMAND_MAX_LENGTH);
        getCommand(buf, cmd, arg);

        if (!strcmp(cmd, "get")) {
            //printf("Le fichier %s est de taille : %d\n et contient: %s\n", arg, size,out);

            // MODIFIER CE TRUC
            if(arg[strlen(arg)-1] == '\n')arg[strlen(arg)-1] = '\0';

            //declaration des structures de données
            int f;
            int compte = 0;

            //envoie des paquets successif
            if ((f = open(arg, O_RDONLY, 0)) > 0) {
                char b[TAILLE_PAQUET];

                while ((compte = Read(f, b, TAILLE_PAQUET)) > 0) {
                    envoiePaquet(connfd, compte, b);
                }
                Close(f);
            }
            //envoyer -1 au client, stop ou fichier inexistant
            compte = -1;
            Rio_writen(connfd, &compte, sizeof(int));

        } else if (!strcmp(cmd, "echo")) {
            Rio_writen(connfd, arg, n);
        } else {
            printf("unknow command received: %s\n", cmd);
        }

        free(cmd);
        free(arg);
    }
}
void getCommand(char buf[COMMAND_MAX_LENGTH], char* cmd, char* arg) {
    int i = 0;

    // Remplir cmd
    while ((buf[i] != ' ') && (buf[i] != '\n')) {
        cmd[i] = buf[i];
        i++;
    }
    cmd[i++] = '\0';

    int j = 0;
    // Remplir arg
    while (buf[i] != '\0') {
        arg[j] = buf[i];
        i++;
        j++;
    }
    arg[j] = '\0';
}

int envoiePaquet(int connfd, int compte, char* buf){
    //printf("ENVOIE: [%s]\n", buf);

    //envoie de la taille du paquet
    Rio_writen(connfd, &compte, sizeof(int));

    //envoie du paquets
    Rio_writen(connfd, buf, compte);
    return 0;
}
