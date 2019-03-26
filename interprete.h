#ifndef __CLIENTFTP_H__
#define __CLIENTFTP_H__

#define COMMAND_MAX_LENGTH 256
#define TAILLE_PAQUET 65536
#define PATH_MAX 256

#define RESET "\033[0m"
#define RED "\x1B[31m"
#define GREEN  "\x1B[32m"
#define YELLOW  "\x1B[33m"
#define BLUE  "\x1B[34m"
#define MAGENTA  "\x1B[35m"
#define CYAN  "\x1B[36m"

void getCommand(char buf[COMMAND_MAX_LENGTH], char* cmd, char* arg);
int envoiePaquet(int connfd, int compte, char* buf);
int file_length(char *filename);

#endif
