#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H

void init_board(char board[64]);
void board_to_string(char board[64], char out[65]); // copiaza tabla in string

int validate_move_and_apply(char board[64], const char* move, int white_turn); // valideaza si aplica mutarea
int is_in_check(char board[64], int white); //verifica daca regele e in sah

int is_checkmate(char board[64], int white); //verifica daca e sah mat

#endif
