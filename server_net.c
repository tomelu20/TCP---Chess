//acceptă conexiuni, gestionează meciuri
#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int server_init(int port) //initializează serverul pe portul dat
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0); //crează socket TCP
    if (sockfd < 0)
    {
        perror("socket");
        exit(1);
    }

    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)); //pentru a reutiliza adresa facem setare SO_REUSEADDR

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port); //setează portul
    addr.sin_addr.s_addr = INADDR_ANY; //ascultă pe toate interfețele

    if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) //leagă socket-ul de port
    {
        perror("bind");
        exit(1);
    }
    if (listen(sockfd, 32) < 0) //ascultă conexiuni
    {
        perror("listen");
        exit(1);
    }
    return sockfd;
}

int accept_player(int server_sock) //acceptă conexiuni de la clienți
{
    struct sockaddr_in client; //informații despre client
    socklen_t len = sizeof(client);
    int s = accept(server_sock, (struct sockaddr*)&client, &len); //acceptă conexiunea de la client la server_sock
    if (s < 0)
    {
        perror("accept");
        return -1;
    }
    return s;
}

int send_line(int sock, const char* line) //trimite mesaje la client cu /n la final
{
    size_t L = strlen(line);
    size_t sent = 0;
    while (sent < L)
    {
        ssize_t r = send(sock, line + sent, L - sent, 0);
        if (r <= 0) return -1;
        sent += r;
    }
    send(sock, "\n", 1, 0);
    return 0;
}


int recv_line(int sock, char* buf, int maxlen) //primeste mesaje de la cilent pana la /n sau maxlen
{
    int idx = 0;
    while (idx < maxlen - 1)
    {
        char c;
        ssize_t r = recv(sock, &c, 1, 0);
        if (r <= 0)
            return -1;
        if (c == '\r') //ignora \r
            continue;
        if (c == '\n') //dacă e \n, termină citirea
            break;
        buf[idx++] = c;
    }
    buf[idx] = 0;
    return idx; //return bytes sau -1
}
