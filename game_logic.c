// server/game_logic.c
// Logica jocului de șah (validare mutări, detectare șah/mat, rocada, promovare).
// Design: tabla are 64 caractere, indexare: board[0] = a8, board[63] = h1
// Indicii: index = rank*8 + file, cu rank 0 = a8..h8; file 0 = a..h
//
// Modificări cheie:
// - parse_move acceptă mutări de forma "e2e4" sau "e7e8Q" (ultimul char = promovare)
// - rocada: permisă doar dacă regele și turnul sunt la pozițiile inițiale și
//   pătratele intermediare sunt libere și nu sunt atacate; se verifică și că regele
//   nu este în șah și nu trece prin pătrat atacat.
// - promovare: dacă mutarea conține un caracter 5 (ex: 'Q','R','B','N') îl folosim,
//   altfel promovăm automat la damă (Q/q).

#include "game_logic.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

/* returnează index = rank * 8 + file */
static int pos_index(int file, int rank) { return rank * 8 + file; }

/* INITIAL BOARD:
   board[0] = a8 ... board[7] = h8
   ...
   board[56] = a1 ... board[63] = h1
*/
void init_board(char board[64])
{
    const char* initial =
        "rnbqkbnr"  // 8
        "pppppppp"  // 7
        "........"  // 6
        "........"  // 5
        "........"  // 4
        "........"  // 3
        "PPPPPPPP"  // 2
        "RNBQKBNR"; // 1
    memcpy(board, initial, 64);
}

void board_to_string(char board[64], char out[65]) //face o copie a tablei într-un string
{
    for (int i = 0; i < 64; ++i)
        out[i] = board[i];
    out[64] = 0;
}

//parse_move: parsează "e2e4" sau "e7e8Q"
static int parse_move(const char* m, int* from, int* to, char* promo)
{
    if (!m) return 0;
    int len = strlen(m);
    if (len < 4) return 0;

    // primele 4 caractere trebuie să fie litera file - cifra rank
    int fx = m[0] - 'a';
    int fy = m[1] - '1'; // '1' => 0 (rank1)
    int tx = m[2] - 'a';
    int ty = m[3] - '1';
    if (fx < 0 || fx > 7 || tx < 0 || tx > 7 || fy < 0 || fy > 7 || ty < 0 || ty > 7)
        return 0;

    // convertim la indexing intern (0 = a8)
    int from_idx = (7 - fy) * 8 + fx;
    int to_idx   = (7 - ty) * 8 + tx;
    *from = from_idx;
    *to   = to_idx;

    // promo opțional (ex: e7e8Q sau e7e8q)
    if (len >= 5) {
        char c = m[4];
        // acceptăm Q/R/B/N (majuscule sau minuscule)
        if (c == 'Q' || c == 'R' || c == 'B' || c == 'N' ||
            c == 'q' || c == 'r' || c == 'b' || c == 'n') {
            *promo = c;
        } else {
            *promo = 0;
        }
    } else {
        *promo = 0;
    }
    return 1;
}

/* attacked_by: întoarce 1 dacă pătratul 'sq' este atacat de side 'by_white' */
static int attacked_by(char board[64], int sq, int by_white)
{
    int fx = sq % 8;
    int fy = sq / 8;

    for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 8; ++x) {
            int i = y*8 + x;
            char p = board[i];
            if (p == '.') continue;
            if (by_white && !isupper((unsigned char)p)) continue;
            if (!by_white && !islower((unsigned char)p)) continue;
            char lower = tolower((unsigned char)p);

            int dx = fx - x;
            int dy = fy - y;
            int adx = dx < 0 ? -dx : dx;
            int ady = dy < 0 ? -dy : dy;

            if (lower == 'p') {
                // white pawns attack "up" (towards rank 0), black pawns attack "down"
                if (by_white) {
                    if (dy == -1 && adx == 1) return 1;
                } else {
                    if (dy == 1 && adx == 1) return 1;
                }
            } else if (lower == 'n') {
                if ((adx==1 && ady==2) || (adx==2 && ady==1)) return 1;
            } else if (lower == 'b' || lower == 'q') {
                if (adx == ady && adx > 0) {
                    int sx = (dx > 0) ? 1 : -1;
                    int sy = (dy > 0) ? 1 : -1;
                    int cx = x + sx, cy = y + sy;
                    int ok = 1;
                    while (cx != fx || cy != fy) {
                        if (board[cy*8 + cx] != '.') { ok = 0; break; }
                        cx += sx; cy += sy;
                    }
                    if (ok) return 1;
                }
            }

            if (lower == 'r' || lower == 'q') {
                if ((dx == 0 && dy != 0) || (dy == 0 && dx != 0)) {
                    int sx = (dx==0)?0:((dx>0)?1:-1);
                    int sy = (dy==0)?0:((dy>0)?1:-1);
                    int cx = x + sx, cy = y + sy;
                    int ok = 1;
                    while (cx != fx || cy != fy) {
                        if (board[cy*8 + cx] != '.') { ok = 0; break; }
                        cx += sx; cy += sy;
                    }
                    if (ok) return 1;
                }
            }

            if (lower == 'k') {
                if (adx <= 1 && ady <= 1) return 1;
            }
        }
    }
    return 0;
}

/* is_in_check: returnează 1 dacă partea 'white' este în șah */
int is_in_check(char board[64], int white)
{
    char king = white ? 'K' : 'k';
    int ksq = -1;
    for (int i = 0; i < 64; ++i) if (board[i] == king) { ksq = i; break; }
    if (ksq < 0) return 1; // no king -> treat as check (invalid)
    return attacked_by(board, ksq, !white);
}

/* Generează mutări pentru piesa de la indexul i; callback-ul cb(from,to,ctx) e apelat pentru fiecare mutare posibilă.
   Notă: gen_piece_moves generează mutări "normale" (fără a ține cont de șah) */
typedef void (*move_cb)(int from, int to, void* ctx);

static void gen_piece_moves(char board[64], int i, move_cb cb, void* ctx)
{
    char p = board[i];
    if (p == '.') return;
    int white = isupper((unsigned char)p);
    char lower = tolower((unsigned char)p);
    int x = i % 8, y = i / 8;

    if (lower == 'p') {
        int dir = white ? -1 : 1;
        int ny = y + dir;
        if (ny >= 0 && ny < 8) {
            int fwd = ny*8 + x;
            if (board[fwd] == '.') {
                cb(i, fwd, ctx);
                // double step
                if ((white && y == 6) || (!white && y == 1)) {
                    int ny2 = y + 2*dir;
                    int fwd2 = ny2*8 + x;
                    if (board[fwd2] == '.') cb(i, fwd2, ctx);
                }
            }
            // captures
            for (int dx = -1; dx <= 1; dx += 2) {
                int nx = x + dx;
                if (nx < 0 || nx > 7) continue;
                int cap = ny*8 + nx;
                if (cap < 0 || cap >= 64) continue;
                if (board[cap] != '.' && (isupper((unsigned char)board[cap]) != white)) {
                    cb(i, cap, ctx);
                }
            }
        }
    } else if (lower == 'n') {
        int moves[8][2] = {{1,2},{2,1},{-1,2},{-2,1},{1,-2},{2,-1},{-1,-2},{-2,-1}};
        for (int k=0;k<8;k++){
            int nx = x + moves[k][0], ny = y + moves[k][1];
            if (nx<0||nx>7||ny<0||ny>7) continue;
            int t = ny*8 + nx;
            if (board[t]=='.' || (isupper((unsigned char)board[t]) != isupper((unsigned char)p))) cb(i,t,ctx);
        }
    } else if (lower == 'b' || lower == 'r' || lower == 'q') {
        int dirs[8][2] = {{1,0},{-1,0},{0,1},{0,-1},{1,1},{1,-1},{-1,1},{-1,-1}};
        int start = (lower=='b')?4: (lower=='r')?0:0;
        int end = (lower=='b')?8: (lower=='r')?4:8;
        for (int d = start; d < end; ++d) {
            int sx = dirs[d][0], sy = dirs[d][1];
            int nx = x + sx, ny = y + sy;
            while (nx>=0 && nx<8 && ny>=0 && ny<8) {
                int t = ny*8 + nx;
                if (board[t] == '.') {
                    cb(i, t, ctx);
                } else {
                    if (isupper((unsigned char)board[t]) != isupper((unsigned char)p)) cb(i, t, ctx);
                    break;
                }
                nx += sx; ny += sy;
            }
        }
    } else if (lower == 'k') {
        for (int dx=-1;dx<=1;dx++) for (int dy=-1;dy<=1;dy++){
            if (dx==0 && dy==0) continue;
            int nx = x + dx, ny = y + dy;
            if (nx<0||nx>7||ny<0||ny>7) continue;
            int t = ny*8 + nx;
            if (board[t]=='.' || (isupper((unsigned char)board[t]) != isupper((unsigned char)p))) cb(i,t,ctx);
        }
    }
}

/* apply_move_internal: mută piesa din f în t (fără verificări suplimentare).
   Nu face promovare aici; promovarea se tratează în validate_move_and_apply. */
static void apply_move_internal(char b[64], int f, int t)
{
    b[t] = b[f];
    b[f] = '.';
}

/* side_has_any_move: verifică dacă partea 'white' are vreună mutare legală (folosind gen_piece_moves + simulare) */
static int side_has_any_move(char board[64], int white)
{
    int found = 0;
    struct Ctx { char b[64]; int white; int *found; } ctx;
    memcpy(ctx.b, board, 64);
    ctx.white = white;
    ctx.found = &found;

    void cb(int from, int to, void* vctx) {
        struct Ctx* c = (struct Ctx*)vctx;
        char copyb[64];
        memcpy(copyb, c->b, 64);
        apply_move_internal(copyb, from, to);
        if (!is_in_check(copyb, c->white)) {
            *(c->found) = 1;
        }
    }

    for (int i=0;i<64 && !found;i++){
        char p = board[i];
        if (p=='.') continue;
        if (isupper((unsigned char)p) != white) continue;
        gen_piece_moves(board, i, cb, &ctx);
    }
    return found;
}

/* is_checkmate: true dacă side 'white' e în check și nu are mutări legale */
int is_checkmate(char board[64], int white)
{
    if (!is_in_check(board, white)) return 0;
    if (side_has_any_move(board, white)) return 0;
    return 1;
}

/* validate_move_and_apply:
   - parsează mutarea (opțional cu promovare: e7e8Q)
   - verifică piesa sursă și culoarea
   - verifică că nu capturează propria piesă
   - suport pentru rocada: dacă regele mută 2 coloane, se tratează rocada (dacă e legal)
   - verifică legalitatea mutării (gen_piece_moves) și dacă mutarea lasă regele în șah
   - aplică mutarea și gestionează promovarea (folosind char promo dacă este furnizat)
   Return: 1 = aplicată, 0 = invalid
*/
int validate_move_and_apply(char board[64], const char* move, int white_turn)
{
    int f, t; // from, to
    char promo = 0;
    if (!parse_move(move, &f, &t, &promo)) {
        printf("[DEBUG] parse_move FAILED for move='%s'\n", move ? move : "(null)");
        return 0;
    }

    char p = board[f]; // piece at from
    if (p == '.') {
        printf("[DEBUG] No piece at source square for move='%s' (from=%d)\n", move, f);
        return 0;
    }

    // verify piece color matches side to move (use unsigned char cast)
    int piece_white = isupper((unsigned char)p) ? 1 : 0;
    if (piece_white != white_turn) {
        printf("[DEBUG] Piece color mismatch for move='%s' piece='%c' at %d white_turn=%d\n",
               move, p, f, white_turn);
        return 0;
    }


    // cannot capture own piece
    if (board[t] != '.' && (isupper((unsigned char)board[t]) == isupper((unsigned char)p))) {
        printf("[DEBUG] Attempt to capture own piece for move='%s' target_piece='%c' at %d\n",
               move, board[t], t);
        return 0;
    }

    // --- Castling detection & handling ---
    // For white: king initial e1 index = 60; rooks at a1=56 and h1=63
    // For black: king initial e8 index = 4; rooks at a8=0 and h8=7
    int is_castle = 0;
    int castle_kingside = 0;
    if ((p == 'K' && f == 60 && (t == 62 || t == 58)) ||
        (p == 'k' && f == 4  && (t == 6  || t == 2))) {
        // attempt to castle
        int white = isupper((unsigned char)p);
        int king_from = f;
        int king_to = t;
        int rook_from = -1, rook_to = -1;

        if (white) {
            if (t == 62) { // O-O (king to g1)
                rook_from = 63; rook_to = 61; castle_kingside = 1;
            } else if (t == 58) { // O-O-O (king to c1)
                rook_from = 56; rook_to = 59; castle_kingside = 0;
            }
        } else {
            if (t == 6) { // O-O black: king to g8
                rook_from = 7; rook_to = 5; castle_kingside = 1;
            } else if (t == 2) { // O-O-O black: king to c8
                rook_from = 0; rook_to = 3; castle_kingside = 0;
            }
        }

        // verify king & rook are in their original positions (unmoved)
        if (rook_from < 0) {
            // invalid target for castling
            return 0;
        }
        if (board[rook_from] == '.' ) {
            // no rook
            return 0;
        }
        // rook must be correct color
        if (isupper((unsigned char)board[rook_from]) != isupper((unsigned char)p)) return 0;

        // squares between king and rook must be empty
        int step = (rook_from > king_from) ? 1 : -1;
        for (int x = king_from + step; x != rook_from; x += step) {
            if (x == rook_from) break;
            if (board[x] != '.') {
                // path blocked
                return 0;
            }
        }

        // king must not be in check now
        if (is_in_check(board, white)) return 0;

        // and king cannot pass through or land on attacked squares
        // find squares king passes: from king_from to king_to inclusive
        int pass1 = (king_from + king_to) / 2; // middle square when moving two steps
        int pass2 = king_to;
        if (attacked_by(board, pass1, !white) || attacked_by(board, pass2, !white)) return 0;

        // all checks passed — perform castling: move king and rook
        apply_move_internal(board, king_from, king_to);
        apply_move_internal(board, rook_from, rook_to);
        printf("[DEBUG] Castling applied for %s side (king %d->%d rook %d->%d)\n",
               white ? "WHITE":"BLACK", king_from, king_to, rook_from, rook_to);
        return 1;
    }

    // --- Normal move flow ---
    // Check that piece can generate this move (ignoring check)
    int legal = 0;
    void cb_check(int from, int to, void* ctx) {
        if (from == f && to == t) *(int*)ctx = 1;
    }
    gen_piece_moves(board, f, cb_check, &legal);
    if (!legal) {
        printf("[DEBUG] Piece %c at %d cannot move to %d (not generated) for move='%s'\n",
               p, f, t, move);
        return 0;
    }

    // simulate move
    char copyb[64];
    memcpy(copyb, board, 64);
    apply_move_internal(copyb, f, t);

    // handle promotion in simulation if pawn reaches last rank
    if (tolower((unsigned char)p) == 'p') {
        if ( (isupper((unsigned char)p) && (t / 8 == 0)) ||
             (islower((unsigned char)p) && (t / 8 == 7)) ) {
            // promotion in simulation: if promo char provided, use it; else default to Q/q
            char chosen = 0;
            if (promo) chosen = promo;
            else chosen = isupper((unsigned char)p) ? 'Q' : 'q';
            // normalize chosen to correct case
            if (isupper((unsigned char)p)) chosen = (char)toupper((unsigned char)chosen);
            else chosen = (char)tolower((unsigned char)chosen);
            copyb[t] = chosen;
        }
    }

    // if move leaves own king in check -> invalid
    if (is_in_check(copyb, white_turn)) {
        printf("[DEBUG] Move '%s' would leave king in check\n", move);
        return 0;
    }

    // apply to real board
    apply_move_internal(board, f, t);

    // apply promotion (if pawn reached last rank)
    if (tolower((unsigned char)p) == 'p') {
        if ( (isupper((unsigned char)p) && (t / 8 == 0)) ||
             (islower((unsigned char)p) && (t / 8 == 7)) ) {
            // choose promotion piece
            char chosen = 0;
            if (promo) chosen = promo;
            else chosen = isupper((unsigned char)p) ? 'Q' : 'q';
            // normalize
            if (isupper((unsigned char)p)) chosen = (char)toupper((unsigned char)chosen);
            else chosen = (char)tolower((unsigned char)chosen);
            board[t] = chosen;
            printf("[DEBUG] Pawn promoted at %d to %c (move='%s')\n", t, board[t], move);
        }
    }

    printf("[DEBUG] Move '%s' applied: %c from %d to %d\n", move, p, f, t);
    return 1;
}
