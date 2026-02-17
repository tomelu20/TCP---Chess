#ifndef CLIENT_H
#define CLIENT_H
int net_connect_to(const char* host, int port); //face conexiune la server
int send_line_cli(int sock, const char* line); // trimite mesaje la server
int recv_line_cli(int sock, char* buf, int maxlen); // primeste mesaje de la server
void ui_init(); //pornească interfața
void ui_draw_board(const char* board, int white_bottom);
void ui_end(); //inchide interfața
int ui_get_move_input(char* out, int maxlen); //citeste mutarea jucatorului de la tastatura
#endif
