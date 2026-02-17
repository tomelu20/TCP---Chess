#include <ncurses.h>
#include <string.h>
#include <ctype.h>

static WINDOW* board_win;

void ui_init() { // initialize ncurses UI
    initscr();              // initialize ncurses
    cbreak();               // read input immediately
    noecho();               // don't show typed chars
    keypad(stdscr, TRUE);   // enable arrow keys
    refresh();

    int width = 3 + 8*4;    // left margin
    int height = 12;        // top margin
    board_win = newwin(height, width, 1, 1); // create window for board
    box(board_win, 0, 0); // draw border
    wrefresh(board_win); // refresh window
}

void ui_end() { // end ncurses UI
    delwin(board_win); // delete board window
    endwin(); // end ncurses mode
}


/* board string is 64 chars a8..h1 */
/* board string is 64 chars a8..h1 */
void ui_draw_board(const char* board, int white_bottom) {
    werase(board_win); // clear window
    box(board_win, 0, 0); // redraw border

    start_color();
    init_color(COLOR_BLACK, 125, 125, 115); // define gray color

    // define colors
    init_pair(1, COLOR_RED, COLOR_WHITE);   // white piece on white square
    init_pair(2, COLOR_RED, COLOR_BLACK);   // white piece on dark square
    init_pair(3, COLOR_BLUE, COLOR_WHITE);     // black piece on white square
    init_pair(4, COLOR_BLUE, COLOR_BLACK);     // black piece on dark square
    init_pair(5, COLOR_BLACK, COLOR_WHITE);    // empty white square
    init_pair(6, COLOR_BLACK, COLOR_BLACK);    // empty dark square

    for (int rank = 0; rank < 8; ++rank) // deseneaza tabla
    {
        int draw_rank = white_bottom ? rank : 7 - rank; // schimba tabla pt juc negru
        int label_rank = 8 - draw_rank;
        mvwprintw(board_win, 1 + rank, 1, "%d ", label_rank); // deseneaza eticheta randului

        for (int file = 0; file < 8; ++file) {
            int draw_file = white_bottom ? file : 7 - file; // schimba tabla pt juc negru
            int idx = draw_rank * 8 + draw_file;
            char c = board[idx];

            int is_white_square = (draw_rank + draw_file) % 2 == 0;
            int pair = 0;

            if (c == '.') {
                pair = is_white_square ? 5 : 6; // empty square
            } else if (isupper(c)) { // white piece
                pair = is_white_square ? 1 : 2;
            } else { // black piece
                pair = is_white_square ? 3 : 4;
            }

            wattron(board_win, COLOR_PAIR(pair) | A_BOLD);
            mvwprintw(board_win, 1 + rank, 3 + file * 4, " %c ", c);
            wattroff(board_win, COLOR_PAIR(pair) | A_BOLD);
        }
    }

    // draw file labels
    if (white_bottom)
        mvwprintw(board_win, 9, 3, " a   b   c   d   e   f   g   h");
    else
        mvwprintw(board_win, 9, 3, " h   g   f   e   d   c   b   a");

    wrefresh(board_win);
}



// read move input from user
int ui_get_move_input(char* out, int maxlen)
{
    mvprintw(14, 1, "Introdu mutare (ex: e2e4) sau 'quit': ");
    clrtoeol(); // clear to end of line
    echo(); // enable echo for user input
    char buf[64];
    getnstr(buf, sizeof(buf)-1); // read input
    noecho(); // disable echo
    if (strcmp(buf, "quit") == 0) // check for quit command
        return 0;
    strncpy(out, buf, maxlen-1); // copy input to output
    out[maxlen-1] = 0; // ensure null termination
    return 1;
}
