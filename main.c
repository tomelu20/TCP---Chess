#include "server.h"
#include "game_logic.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct pair_args
{
    int p1; // player 1 socket
    int p2; // player 2 socket
    char name1[64]; // player 1 name
    char name2[64]; // player 2 name
}; // parametri pt thread de joc ( game_thread)

void* game_thread(void* arg) // thread pt fiecare joc
{
    struct pair_args* a = (struct pair_args*)arg; // preiau parametrii
    int p1 = a->p1, p2 = a->p2; // socketii jucatorilor
    char name1[64], name2[64]; // numele jucatorilor
    strncpy(name1, a->name1, 64); strncpy(name2, a->name2, 64); // copiez numele
    free(a); // eliberez memoria alocata pt parametri


    char board[64];
    init_board(board); // initializez tabla
    int white_turn = 1; // player1 = white
    send_line(p1, "START WHITE"); // anunt jucator ca e white
    send_line(p2, "START BLACK"); // anunt jucator ca e black

    char bstr[128]; // board string
    char tmp[128]; // buffer temporar pt mesaje

    board_to_string(board, bstr); // convertesc tabla in string
    snprintf(tmp, sizeof(tmp), "BOARD %s", bstr); // trimit tabla initiala la ambii jucatori
    send_line(p1, tmp); // trimit mesaj la clientul fiecarui player sa deseneze tabla lui
    send_line(p2, tmp);

    int game_over = 0; // flag pt terminare joc
    while (!game_over)
    {
        int cur = white_turn ? p1 : p2; // jucatorul curent
        int other = white_turn ? p2 : p1; // jucatorul advers
        send_line(cur, "TURN"); // anunt jucatorul curent ca e randul lui
        send_line(other, "WAIT"); // anunt jucatorul advers sa astepte

        char line[256];
        if (recv_line(cur, line, sizeof(line)) <= 0) // primesc mutarea de la jucatorul curent
        {
            send_line(other, "GAMEOVER OPPONENT_DISCONNECTED");
            close(p1); close(p2);
            return NULL;
        }
        if (strncmp(line, "MOVE ", 5) == 0) // verific daca e mutare
        {
            const char* mv = line + 5; // extrag mutarea, + 5 pt a sari peste "MOVE "

            if (validate_move_and_apply(board, mv, white_turn)) // verific si aplic mutarea
            {
                board_to_string(board, bstr); // convertesc tabla in string
                snprintf(tmp, sizeof(tmp), "BOARD %s", bstr); // pregatesc mesajul cu tabla actualizata
                send_line(p1, tmp); // trimit tabla actualizata la ambii jucatori
                send_line(p2, tmp);

                // check for check / checkmate
                int opponent = !white_turn;
                if (is_checkmate(board, opponent))
                {
                    send_line(cur, "GAMEOVER WIN");
                    send_line(other, "GAMEOVER LOSE");
                    game_over = 1;
                } else if (is_in_check(board, opponent))
                {
                    send_line(other, "MESSAGE CHECK");
                }
                white_turn = !white_turn; // schimb randul
            } else
            {
                send_line(cur, "INVALID"); // mutare invalida
            }
        } else
        {
            send_line(cur, "UNKNOWN"); // comanda necunoscuta
        }
    }
    close(p1); close(p2); // inchid socketii jucatorilor
    return NULL;
}

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        printf("Usage: %s <port>\n", argv[0]);
        return 1;
    }
    int port = atoi(argv[1]);
    int server_sock = server_init(port);
    printf("Server listening on port %d\n", port);

    while (1)
    {
        int p1 = accept_player(server_sock); // astept primul jucator
        if (p1 < 0)
            continue;
        char buf[256];
        if (recv_line(p1, buf, sizeof(buf)) <= 0) // primesc mesajul de la jucator = numele lui
        {
            close(p1);
            continue;
        }
        char name1[64] = "Player1"; // numele jucatorului 1
        if (strncmp(buf, "HELLO ", 6) == 0) // extrag numele jucatorului 1
            strncpy(name1, buf + 6, 63);

        send_line(p1, "WAITING"); // anunt jucatorul 1 ca asteapta adversar

        int p2 = accept_player(server_sock); // accept al doilea jucator
        if (p2 < 0)
        {
            close(p1);
            continue;
        }
        if (recv_line(p2, buf, sizeof(buf)) <= 0) // primesc mesajul de la jucatorul 2 = numele lui
        {
            close(p1);
            close(p2);
            continue;
        }
        char name2[64] = "Player2";
        if (strncmp(buf, "HELLO ", 6) == 0) // extrag numele jucatorului 2
            strncpy(name2, buf + 6, 63);

        // create thread with pair
        struct pair_args* a = malloc(sizeof(*a));
        a->p1 = p1; // socket jucator 1
        a->p2 = p2; // socket jucator 2
        strncpy(a->name1, name1, 64);
        strncpy(a->name2, name2, 64);

        pthread_t th;
        pthread_create(&th, NULL, game_thread, a); // pornesc thread-ul de joc
        pthread_detach(th); // dezatasez thread-ul
        printf("Paired %s and %s\n", name1, name2);
    }
    close(server_sock);
    return 0;
}
