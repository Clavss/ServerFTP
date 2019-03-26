#ifndef __CLIENTFTP_H__
#define __CLIENTFTP_H__

#define COMMAND_MAX_LENGTH 256
#define TAILLE_PAQUET 65536

void getCommand(char buf[COMMAND_MAX_LENGTH], char* cmd, char* arg);
int envoiePaquet(int connfd,int compte,char* buf);

#endif
