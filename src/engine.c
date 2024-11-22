#include "engine.h"

#include <stdint.h>
#include <string.h>

#define ARENA_BLOCK_INDEX(file, rank, tier) (file + rank * 8 + tier * 64)

static const struct token DEFAULT_ARENA_TOKENS[ARENA_TOKENS_LENGTH] = {
        [ARENA_BLOCK_INDEX(FILE_A, RANK_1, TIER_S)] = {COLOR_WHITE, PIECE_ROOK,   false},
        [ARENA_BLOCK_INDEX(FILE_B, RANK_1, TIER_S)] = {COLOR_WHITE, PIECE_KNIGHT, false},
        [ARENA_BLOCK_INDEX(FILE_C, RANK_1, TIER_S)] = {COLOR_WHITE, PIECE_BISHOP, false},
        [ARENA_BLOCK_INDEX(FILE_D, RANK_1, TIER_S)] = {COLOR_WHITE, PIECE_QUEEN,  false},
        [ARENA_BLOCK_INDEX(FILE_E, RANK_1, TIER_S)] = {COLOR_WHITE, PIECE_KING,   false},
        [ARENA_BLOCK_INDEX(FILE_F, RANK_1, TIER_S)] = {COLOR_WHITE, PIECE_BISHOP, false},
        [ARENA_BLOCK_INDEX(FILE_G, RANK_1, TIER_S)] = {COLOR_WHITE, PIECE_KNIGHT, false},
        [ARENA_BLOCK_INDEX(FILE_H, RANK_1, TIER_S)] = {COLOR_WHITE, PIECE_ROOK,   false},
        [ARENA_BLOCK_INDEX(FILE_A, RANK_1, TIER_T)] = {COLOR_WHITE, PIECE_PAWN,   false},
        [ARENA_BLOCK_INDEX(FILE_B, RANK_1, TIER_T)] = {COLOR_WHITE, PIECE_PAWN,   false},
        [ARENA_BLOCK_INDEX(FILE_C, RANK_1, TIER_T)] = {COLOR_WHITE, PIECE_PAWN,   false},
        [ARENA_BLOCK_INDEX(FILE_D, RANK_1, TIER_T)] = {COLOR_WHITE, PIECE_PAWN,   false},
        [ARENA_BLOCK_INDEX(FILE_E, RANK_1, TIER_T)] = {COLOR_WHITE, PIECE_PAWN,   false},
        [ARENA_BLOCK_INDEX(FILE_F, RANK_1, TIER_T)] = {COLOR_WHITE, PIECE_PAWN,   false},
        [ARENA_BLOCK_INDEX(FILE_G, RANK_1, TIER_T)] = {COLOR_WHITE, PIECE_PAWN,   false},
        [ARENA_BLOCK_INDEX(FILE_H, RANK_1, TIER_T)] = {COLOR_WHITE, PIECE_PAWN,   false},
        [ARENA_BLOCK_INDEX(FILE_A, RANK_2, TIER_T)] = {COLOR_WHITE, PIECE_PAWN,   false},
        [ARENA_BLOCK_INDEX(FILE_B, RANK_2, TIER_T)] = {COLOR_WHITE, PIECE_PAWN,   false},
        [ARENA_BLOCK_INDEX(FILE_C, RANK_2, TIER_T)] = {COLOR_WHITE, PIECE_PAWN,   false},
        [ARENA_BLOCK_INDEX(FILE_D, RANK_2, TIER_T)] = {COLOR_WHITE, PIECE_PAWN,   false},
        [ARENA_BLOCK_INDEX(FILE_E, RANK_2, TIER_T)] = {COLOR_WHITE, PIECE_PAWN,   false},
        [ARENA_BLOCK_INDEX(FILE_F, RANK_2, TIER_T)] = {COLOR_WHITE, PIECE_PAWN,   false},
        [ARENA_BLOCK_INDEX(FILE_G, RANK_2, TIER_T)] = {COLOR_WHITE, PIECE_PAWN,   false},
        [ARENA_BLOCK_INDEX(FILE_H, RANK_2, TIER_T)] = {COLOR_WHITE, PIECE_PAWN,   false},
        [ARENA_BLOCK_INDEX(FILE_A, RANK_7, TIER_Y)] = {COLOR_BLACK, PIECE_PAWN,   false},
        [ARENA_BLOCK_INDEX(FILE_B, RANK_7, TIER_Y)] = {COLOR_BLACK, PIECE_PAWN,   false},
        [ARENA_BLOCK_INDEX(FILE_C, RANK_7, TIER_Y)] = {COLOR_BLACK, PIECE_PAWN,   false},
        [ARENA_BLOCK_INDEX(FILE_D, RANK_7, TIER_Y)] = {COLOR_BLACK, PIECE_PAWN,   false},
        [ARENA_BLOCK_INDEX(FILE_E, RANK_7, TIER_Y)] = {COLOR_BLACK, PIECE_PAWN,   false},
        [ARENA_BLOCK_INDEX(FILE_F, RANK_7, TIER_Y)] = {COLOR_BLACK, PIECE_PAWN,   false},
        [ARENA_BLOCK_INDEX(FILE_G, RANK_7, TIER_Y)] = {COLOR_BLACK, PIECE_PAWN,   false},
        [ARENA_BLOCK_INDEX(FILE_H, RANK_7, TIER_Y)] = {COLOR_BLACK, PIECE_PAWN,   false},
        [ARENA_BLOCK_INDEX(FILE_A, RANK_8, TIER_Y)] = {COLOR_BLACK, PIECE_PAWN,   false},
        [ARENA_BLOCK_INDEX(FILE_B, RANK_8, TIER_Y)] = {COLOR_BLACK, PIECE_PAWN,   false},
        [ARENA_BLOCK_INDEX(FILE_C, RANK_8, TIER_Y)] = {COLOR_BLACK, PIECE_PAWN,   false},
        [ARENA_BLOCK_INDEX(FILE_D, RANK_8, TIER_Y)] = {COLOR_BLACK, PIECE_PAWN,   false},
        [ARENA_BLOCK_INDEX(FILE_E, RANK_8, TIER_Y)] = {COLOR_BLACK, PIECE_PAWN,   false},
        [ARENA_BLOCK_INDEX(FILE_F, RANK_8, TIER_Y)] = {COLOR_BLACK, PIECE_PAWN,   false},
        [ARENA_BLOCK_INDEX(FILE_G, RANK_8, TIER_Y)] = {COLOR_BLACK, PIECE_PAWN,   false},
        [ARENA_BLOCK_INDEX(FILE_H, RANK_8, TIER_Y)] = {COLOR_BLACK, PIECE_PAWN,   false},
        [ARENA_BLOCK_INDEX(FILE_A, RANK_8, TIER_Z)] = {COLOR_BLACK, PIECE_ROOK,   false},
        [ARENA_BLOCK_INDEX(FILE_B, RANK_8, TIER_Z)] = {COLOR_BLACK, PIECE_KNIGHT, false},
        [ARENA_BLOCK_INDEX(FILE_C, RANK_8, TIER_Z)] = {COLOR_BLACK, PIECE_BISHOP, false},
        [ARENA_BLOCK_INDEX(FILE_D, RANK_8, TIER_Z)] = {COLOR_BLACK, PIECE_QUEEN,  false},
        [ARENA_BLOCK_INDEX(FILE_E, RANK_8, TIER_Z)] = {COLOR_BLACK, PIECE_KING,   false},
        [ARENA_BLOCK_INDEX(FILE_F, RANK_8, TIER_Z)] = {COLOR_BLACK, PIECE_BISHOP, false},
        [ARENA_BLOCK_INDEX(FILE_G, RANK_8, TIER_Z)] = {COLOR_BLACK, PIECE_KNIGHT, false},
        [ARENA_BLOCK_INDEX(FILE_H, RANK_8, TIER_Z)] = {COLOR_BLACK, PIECE_ROOK,   false},
};

static bool assert_valid_block(enum file file, enum rank rank, enum tier tier)
{
        return FILE_A <= file && file <= FILE_H &&
               RANK_1 <= rank && rank <= RANK_8 &&
               TIER_S <= tier && tier <= TIER_Z;
}

static bool assert_valid_token(struct token token)
{
        return PIECE_NONE <= token.piece && token.piece <= PIECE_PAWN &&
               COLOR_NONE <= token.color && token.color <= COLOR_BLACK;
}

static struct block create_block(enum file file, enum rank rank, enum tier tier)
{
        return (struct block){ .file = file, .rank = rank, .tier = tier };
}

static struct moves create_moves(void)
{
        return (struct moves){ .blocks = {0}, .length = 0 };
}

static enum color invert_color(enum color color)
{
        switch (color) {
        case COLOR_WHITE: 
                return COLOR_BLACK;
        case COLOR_BLACK: 
                return COLOR_WHITE;
        default: 
                return COLOR_NONE;
        }
}

static enum color obtain_block_color(enum file file, enum rank rank, enum tier tier)
{
        enum color color = COLOR_BLACK;
        if (file % 2 != 0)
                color = invert_color(color);
        if (rank % 2 != 0)
                color = invert_color(color);
        if (tier % 2 != 0)
                color = invert_color(color);

        return color;
}

static void obtain_moves_piece_king(struct arena arena, enum file file, enum rank rank, enum tier tier, struct token token, struct moves *moves)
{
        // TODO: Create map of checked blocks.

        for (int dt = -1; dt <= 1; dt++)
        for (int dr = -1; dr <= 1; dr++)
        for (int df = -1; df <= 1; df++) {
                enum file f = file + df;
                enum rank r = rank + dr;
                enum tier t = tier + dt;
                if (!assert_valid_block(f, r, t))
                        continue;

                struct token other = obtain_arena_token(arena, f, r, t);
                if (other.color == token.color)
                        continue;

                moves->blocks[moves->length++] = create_block(f, r, t);
        }

        if (!token.moved) {
                struct token qside_token = obtain_arena_token(arena, FILE_A, RANK_1, TIER_S);
                if (qside_token.color == token.color && qside_token.piece == PIECE_KING)
                        moves->blocks[moves->length++] = create_block(FILE_B, RANK_1, TIER_S);

                struct token kside_token = obtain_arena_token(arena, FILE_H, RANK_1, TIER_S);
                if (kside_token.color == token.color && kside_token.piece == PIECE_KING)
                        moves->blocks[moves->length++] = create_block(FILE_G, RANK_1, TIER_S);
        }
}

static void obtain_moves_piece_rook(struct arena arena, enum file file, enum rank rank, enum tier tier, struct token token, struct moves *moves)
{
        const int directions[6][3] = {
                { 0,  1,  0}, // North
                { 0, -1,  0}, // South
                { 1,  0,  0}, // East
                {-1,  0,  0}, // West
                { 0,  0,  1}, // Up
                { 0,  0, -1}  // Down
        };
        for (int d = 0; d < 6; d++) {
                int df = directions[d][0];
                int dr = directions[d][1]; 
                int dt = directions[d][2];

                enum file f = file;
                enum rank r = rank;
                enum tier t = tier;
                for (;;) {
                        f += df;
                        r += dr;
                        t += dt;
                        if (!assert_valid_block(f, r, t))
                                break;

                        struct token other = obtain_arena_token(arena, f, r, t);
                        if (other.color != token.color)
                                moves->blocks[moves->length++] = create_block(f, r, t);
                        if (other.piece != PIECE_NONE)
                                break;
                }
        }

        // TODO: Castling.
}

static void obtain_moves_piece_bishop(struct arena arena, enum file file, enum rank rank, enum tier tier, struct token token, struct moves *moves)
{
        const int directions[4][3] = {
                { 1,  1,  0}, // North-East
                {-1,  1,  0}, // North-West
                { 1, -1,  0}, // South-East
                {-1, -1,  0}  // South-West
        };
        for (int d = 0; d < 4; d++) {
                int df = directions[d][0];
                int dr = directions[d][1];
                int dt = directions[d][2];

                enum file f = file;
                enum rank r = rank;
                enum tier t = tier;
                for (;;) {
                        f += df;
                        r += dr;
                        t += dt;
                        if (!assert_valid_block(f, r, t))
                                break;

                        struct token other = obtain_arena_token(arena, f, r, t);
                        if (other.color != token.color)
                                moves->blocks[moves->length++] = create_block(f, r, t);
                        if (other.piece != PIECE_NONE)
                                break;
                }
        }
}

static void obtain_moves_piece_queen(struct arena arena, enum file file, enum rank rank, enum tier tier, struct token token, struct moves *moves)
{
        obtain_moves_piece_bishop(arena, file, rank, tier, token, moves);
        obtain_moves_piece_rook(arena, file, rank, tier, token, moves);
}

static void obtain_moves_piece_knight(struct arena arena, enum file file, enum rank rank, enum tier tier, struct token token, struct moves *moves)
{
        const int directions[12][3] = {
                { 2,  1,  0}, // North-East
                { 2, -1,  0}, // North-West
                {-2,  1,  0}, // South-East
                {-2, -1,  0}, // South-West
                { 1,  2,  0}, // North-East
                { 1, -2,  0}, // North-West
                {-1,  2,  0}, // South-East
                {-1, -2,  0}, // South-West
                { 0,  2,  1}, // Up-West
                { 0, -2,  1}, // Up-East
                { 0,  2, -1}, // Down-West
                { 0, -2, -1}  // Down-East
        };
        for (int d = 0; d < 12; d++) {
                int df = directions[d][0];
                int dr = directions[d][1];
                int dt = directions[d][2];

                enum file f = file + df;
                enum rank r = rank + dr;
                enum tier t = tier + dt;
                if (!assert_valid_block(f, r, t))
                        continue;

                struct token other = obtain_arena_token(arena, f, r, t);
                if (other.color != token.color)
                        moves->blocks[moves->length++] = create_block(f, r, t);
        }
}

static void obtain_moves_piece_pawn(struct arena arena, enum file file, enum rank rank, enum tier tier, struct token token, struct moves *moves)
{

}

struct arena create_arena(void)
{
        struct arena arena;
        memcpy(arena.tokens, DEFAULT_ARENA_TOKENS, ARENA_TOKENS_LENGTH * sizeof(struct token));

        return arena;
}

struct moves obtain_arena_moves(struct arena arena, enum file file, enum rank rank, enum tier tier)
{
        struct moves moves = create_moves();
        if (!assert_valid_block(file, rank, tier))
                return moves;

        struct token token = obtain_arena_token(arena, file, rank, tier);
        if (!assert_valid_token(token) || token.color == COLOR_NONE || token.piece == PIECE_NONE)
                return moves;

        switch (token.piece) {
        case PIECE_KING:
                obtain_moves_piece_king(arena, file, rank, tier, token, &moves);
                break;
        case PIECE_QUEEN:
                obtain_moves_piece_queen(arena, file, rank, tier, token, &moves);
                break;
        case PIECE_ROOK:
                obtain_moves_piece_rook(arena, file, rank, tier, token, &moves);
                break;
        case PIECE_BISHOP:
                obtain_moves_piece_bishop(arena, file, rank, tier, token, &moves);
                break;
        case PIECE_KNIGHT:
                obtain_moves_piece_knight(arena, file, rank, tier, token, &moves);
                break;
        case PIECE_PAWN:
                obtain_moves_piece_pawn(arena, file, rank, tier, token, &moves);
                break;
        default:
                break;
        }

        return moves;
}

struct token obtain_arena_token(struct arena arena, enum file file, enum rank rank, enum tier tier)
{
        return arena.tokens[ARENA_BLOCK_INDEX(file, rank, tier)];
}
