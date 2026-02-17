//trimite/primește mesaje TCP

#include "client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int net_connect_to(const char* host, int port) //face conexiune la server
{
    int s = socket(AF_INET, SOCK_STREAM, 0); //creem socket client
    if (s < 0) //verificam daca s-a creat socketul
    {
        perror("socket");
        return -1;
    }

    struct sockaddr_in addr = {0}; //structura cu adresa serverului
    addr.sin_family = AF_INET; //familia de adrese IPv4
    addr.sin_port = htons(port); //setam portul (serverului)
    if (inet_pton(AF_INET, host, &addr.sin_addr) <= 0) //convertim adresa IP in binar - stocat in addr.sin_addr
    {
        perror("inet_pton");
        close(s);
        return -1;
    }
    if (connect(s, (struct sockaddr*)&addr, sizeof(addr)) < 0) //conencteaza client -> server , addr = adresa serverului
    {
        perror("connect");
        close(s);
        return -1;
    }
    return s;
}

int send_line_cli(int sock, const char* line) //mesaj text catre server, sock = socketul client
{
    size_t L = strlen(line); //lungimea mesajului
    size_t sent = 0; //cate bytes trimite
    while (sent < L) //trimite pe bucati mesajul
    {
        ssize_t r = send(sock, line + sent, L - sent, 0);
        if (r <= 0)
            return -1;
        sent += r;
    }
    if (send(sock, "\n", 1, 0) <= 0) //trimite \n la final
        return -1;
    return 0;
}

int recv_line_cli(int sock, char* buf, int maxlen) //primeste mesaj de la server pana la \n
{
    int idx = 0; //indice in buffer
    while (idx < maxlen - 1) //pana se termina dim de buff = maxlen
    {
        char c;
        ssize_t r = recv(sock, &c, 1, 0); //citeste un carcacter pe rand
        if (r <= 0) //daca conexiunea s-a inchis sau eroare
            return -1;
        if (c == '\n') //daca e \n se termina citirea
            break;
        buf[idx++] = c;
    }
    buf[idx] = 0; //inchide stringul cu null
    return idx;
}
