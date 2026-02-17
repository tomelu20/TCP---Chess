#include <stdio.h>
#include <string.h>
#include "game_logic.h"
//
// static void print_board(char b[64])
// {
//     printf("\n");
//     for (int y = 0; y < 8; ++y) {
//         printf("%d  ", 8 - y);
//         for (int x = 0; x < 8; ++x) {
//             char c = b[y * 8 + x];
//             if (c == '.') c = '.';
//             printf("%c ", c);
//         }
//         printf("\n");
//     }
//     printf("\n   a b c d e f g h\n\n");
// }
//
// int main(void)
// {
//     char board[64];
//     char move[32];
//     int white_turn = 1;
//
//     init_board(board);
//     print_board(board);
//
//     while (1)
//     {
//         printf("\nTurn: %s\n", white_turn ? "WHITE" : "BLACK");
//         printf("Enter move (e2e4, e7e8Q, castle e1g1) or 'q' to quit: ");
//         fflush(stdout);
//
//         if (!fgets(move, sizeof(move), stdin))
//             break;
//
//         // remove newline
//         move[strcspn(move, "\n")] = 0;
//
//         if (strcmp(move, "q") == 0)
//             break;
//
//         if (!validate_move_and_apply(board, move, white_turn)) {
//             printf("❌ Invalid move!\n");
//         } else {
//             printf("✅ Move applied!\n");
//             print_board(board);
//
//             if (is_in_check(board, !white_turn))
//                 printf("⚠️  CHECK!\n");
//
//             if (is_checkmate(board, !white_turn)) {
//                 printf("♟️  CHECKMATE! %s wins!\n",
//                        white_turn ? "WHITE" : "BLACK");
//                 break;
//             }
//
//             white_turn = !white_turn;
//         }
//     }
//
//     return 0;
// }
