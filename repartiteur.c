#include "csapp.h"
#include "interprete.h"


// MASTER
void repartiteur(int connfd, char* ip) {
    // Envoie de l'ip
    sendPacket(connfd, strlen(ip), ip);
    printf("client redirected to server %s\n", ip);
}
