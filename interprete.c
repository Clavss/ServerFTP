#include "csapp.h"
#include "interprete.h"

#define TAILLE_MAX 1000000

void interprete(int connfd) {
    size_t n;
    char buf[MAXLINE];
    rio_t rio;
    int cmd_result;

    Rio_readinitb(&rio, connfd);
    while ((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0) {
        //printf("server received %u bytes: [%s]", (unsigned int)n, buf);

        char* cmd = malloc(sizeof(char) * COMMAND_MAX_LENGTH);
        char* arg = malloc(sizeof(char) * COMMAND_MAX_LENGTH);
        getCommand(buf, cmd, arg);

        cmd_result = -1;

        /* GESTION DES COMMANDES */
        /* GET */
        if (!strcmp(cmd, "get")) {
            cmd_result = 1;
            Rio_writen(connfd, &cmd_result, sizeof(int));

            // Declaration des structures de données
            int f;
            int compte = 0;

            // Récupère la taille du fichier
            int f_length_client;
            Rio_readnb(&rio, &f_length_client, sizeof(int));

            int f_length_server = file_length(arg);
            int same_length = (f_length_client == f_length_server);
            Rio_writen(connfd, &same_length, sizeof(int));

            // Envoie des paquets successif
            if ((f = open(arg, O_RDONLY, 0)) > 0) {
                char b[TAILLE_PAQUET];
                if (!same_length) {
                    lseek(f, f_length_client, SEEK_CUR);
                }

                while ((compte = Read(f, b, TAILLE_PAQUET)) > 0) {
                    envoiePaquet(connfd, compte, b);
                }
                Close(f);
            }
            // Envoyer -1 au client, stop ou fichier inexistant
            compte = -1;
            Rio_writen(connfd, &compte, sizeof(int));
        }

        /* PWD */
        else if (!strcmp(cmd, "pwd")) {
            cmd_result = 2;
            Rio_writen(connfd, &cmd_result, sizeof(int));

            char cwd[1024];
            // Récupère le chemin actuel
            getcwd(cwd, sizeof(cwd));
            // Envoie du chemin
            envoiePaquet(connfd, sizeof(cwd), cwd);

        }

        /* BYE */
        else if (!strcmp(cmd, "bye")) {
            cmd_result = 3;
            Rio_writen(connfd, &cmd_result, sizeof(int));

            printf("a client has disconnected\n");
        }
        
        /* CD */
        else if (!strcmp(cmd, "cd")) {
            cmd_result = 4;
            Rio_writen(connfd, &cmd_result, sizeof(int));
            
            char res = '1';
            if (chdir(arg) != 0) {
                res = '0';
            } else {
                printf("-> cd %s\n", arg);
            }
            envoiePaquet(connfd, sizeof(char), &res);
        }
        
        /* LS */
        else if (!strcmp(cmd, "ls")) {
            cmd_result = 5;
            Rio_writen(connfd, &cmd_result, sizeof(int));

            FILE *fp;
            char path[PATH_MAX];
            char* concat = malloc(sizeof(char) *PATH_MAX*PATH_MAX);
            int length_total = 0;

            fp = popen("ls *", "r");
            if (fp == NULL) {
                printf("error\n");
            }
            while (fgets(path, PATH_MAX, fp) != NULL) {
                strcat(concat, path);
                length_total += strlen(path);
            }
            pclose(fp);

            // Envoie le résultat de ls
            envoiePaquet(connfd, length_total, concat);
            free(concat);
        }
        
        /* AUTRES */
        else {
            printf("unknow command received: \"%s\"\n", cmd);
            cmd_result = -1;
            Rio_writen(connfd, &cmd_result, sizeof(int));
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

    // Peut être traité au dessus
    if (arg[strlen(arg)-1] == '\n') {
        arg[strlen(arg)-1] = '\0';
    }
}

int envoiePaquet(int connfd, int compte, char* buf) {
    //printf("ENVOIE: [%s]\n", buf);

    // Envoie de la taille du paquet
    Rio_writen(connfd, &compte, sizeof(int));

    // Envoie du paquets
    Rio_writen(connfd, buf, compte);
    return 0;
}

int file_length(char *filename) {
    struct stat buffer;   
    if (stat(filename, &buffer) == 0) {
        return buffer.st_size;
    } else {
        return 0;
    }
}