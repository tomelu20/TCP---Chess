#ifndef SERVER_H
#define SERVER_H
#include <netinet/in.h>

int server_init(int port); //initializare server
int accept_player(int server_sock); //conecteaza 2 playeri intr-un joc
int send_line(int sock, const char* line); //trimite mesaje la client
int recv_line(int sock, char* buf, int maxlen);//primeste mesaje de la client

#endif
