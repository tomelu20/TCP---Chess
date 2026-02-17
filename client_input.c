//cere numele jucatorului si il trimite la server prin socket

#include "client.h"
#include <stdio.h>
#include <string.h>
#include <ncurses.h>

void prompt_name_and_send(int sock)
{
    char name[64];
    printf("Nume jucator: ");
    fflush(stdout); //asigura ca promptul apare inainte de fgets
    if (!fgets(name, sizeof(name), stdin)) // se citeste numele jucatorului
        return;

    name[strcspn(name, "\n")] = 0; // remove newline
    char line[128]; // mesaj pentru server
    snprintf(line, sizeof(line)+6, "HELLO %s", name); //formateaza mesajul
    send_line_cli(sock, line); //trimite prin socket la server
}