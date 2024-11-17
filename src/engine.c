#include "engine.h"

#include <stdint.h>
#include <string.h>

#define BOARD_BLOCK_INDEX(file, rank) (file + rank * 8)
#define ARENA_BLOCK_INDEX(file, rank, tier) (BOARD_BLOCK_INDEX(file, rank) + tier * 64)

static const struct token DEFAULT_BOARD_TOKENS[BOARD_TOKENS_LENGTH] = {
        [BOARD_BLOCK_INDEX(FILE_A, RANK_1)] = {COLOR_WHITE, PIECE_ROOK,   false},
        [BOARD_BLOCK_INDEX(FILE_B, RANK_1)] = {COLOR_WHITE, PIECE_KNIGHT, false},
        [BOARD_BLOCK_INDEX(FILE_C, RANK_1)] = {COLOR_WHITE, PIECE_BISHOP, false},
        [BOARD_BLOCK_INDEX(FILE_D, RANK_1)] = {COLOR_WHITE, PIECE_QUEEN,  false},
        [BOARD_BLOCK_INDEX(FILE_E, RANK_1)] = {COLOR_WHITE, PIECE_KING,   false},
        [BOARD_BLOCK_INDEX(FILE_F, RANK_1)] = {COLOR_WHITE, PIECE_BISHOP, false},
        [BOARD_BLOCK_INDEX(FILE_G, RANK_1)] = {COLOR_WHITE, PIECE_KNIGHT, false},
        [BOARD_BLOCK_INDEX(FILE_H, RANK_1)] = {COLOR_WHITE, PIECE_ROOK,   false},
        [BOARD_BLOCK_INDEX(FILE_A, RANK_2)] = {COLOR_WHITE, PIECE_PAWN,   false},
        [BOARD_BLOCK_INDEX(FILE_B, RANK_2)] = {COLOR_WHITE, PIECE_PAWN,   false},
        [BOARD_BLOCK_INDEX(FILE_C, RANK_2)] = {COLOR_WHITE, PIECE_PAWN,   false},
        [BOARD_BLOCK_INDEX(FILE_D, RANK_2)] = {COLOR_WHITE, PIECE_PAWN,   false},
        [BOARD_BLOCK_INDEX(FILE_E, RANK_2)] = {COLOR_WHITE, PIECE_PAWN,   false},
        [BOARD_BLOCK_INDEX(FILE_F, RANK_2)] = {COLOR_WHITE, PIECE_PAWN,   false},
        [BOARD_BLOCK_INDEX(FILE_G, RANK_2)] = {COLOR_WHITE, PIECE_PAWN,   false},
        [BOARD_BLOCK_INDEX(FILE_H, RANK_2)] = {COLOR_WHITE, PIECE_PAWN,   false},
        [BOARD_BLOCK_INDEX(FILE_A, RANK_7)] = {COLOR_BLACK, PIECE_PAWN,   false},
        [BOARD_BLOCK_INDEX(FILE_B, RANK_7)] = {COLOR_BLACK, PIECE_PAWN,   false},
        [BOARD_BLOCK_INDEX(FILE_C, RANK_7)] = {COLOR_BLACK, PIECE_PAWN,   false},
        [BOARD_BLOCK_INDEX(FILE_D, RANK_7)] = {COLOR_BLACK, PIECE_PAWN,   false},
        [BOARD_BLOCK_INDEX(FILE_E, RANK_7)] = {COLOR_BLACK, PIECE_PAWN,   false},
        [BOARD_BLOCK_INDEX(FILE_F, RANK_7)] = {COLOR_BLACK, PIECE_PAWN,   false},
        [BOARD_BLOCK_INDEX(FILE_G, RANK_7)] = {COLOR_BLACK, PIECE_PAWN,   false},
        [BOARD_BLOCK_INDEX(FILE_H, RANK_7)] = {COLOR_BLACK, PIECE_PAWN,   false},
        [BOARD_BLOCK_INDEX(FILE_A, RANK_8)] = {COLOR_BLACK, PIECE_ROOK,   false},
        [BOARD_BLOCK_INDEX(FILE_B, RANK_8)] = {COLOR_BLACK, PIECE_KNIGHT, false},
        [BOARD_BLOCK_INDEX(FILE_C, RANK_8)] = {COLOR_BLACK, PIECE_BISHOP, false},
        [BOARD_BLOCK_INDEX(FILE_D, RANK_8)] = {COLOR_BLACK, PIECE_QUEEN,  false},
        [BOARD_BLOCK_INDEX(FILE_E, RANK_8)] = {COLOR_BLACK, PIECE_KING,   false},
        [BOARD_BLOCK_INDEX(FILE_F, RANK_8)] = {COLOR_BLACK, PIECE_BISHOP, false},
        [BOARD_BLOCK_INDEX(FILE_G, RANK_8)] = {COLOR_BLACK, PIECE_KNIGHT, false},
        [BOARD_BLOCK_INDEX(FILE_H, RANK_8)] = {COLOR_BLACK, PIECE_ROOK,   false},
};

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
        [ARENA_BLOCK_INDEX(FILE_A, RANK_8, TIER_Y)] = {COLOR_BLACK, PIECE_ROOK,   false},
        [ARENA_BLOCK_INDEX(FILE_B, RANK_8, TIER_Y)] = {COLOR_BLACK, PIECE_KNIGHT, false},
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

enum color invert_color(enum color color)
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

const char *color_letter(enum color color)
{
        switch (color) {
        case COLOR_WHITE:
                return "w";
        case COLOR_BLACK:
                return "b";
        default:
                return ".";
        }
}

const char *color_string(enum color color)
{
        switch (color) {
        case COLOR_WHITE:
                return "white";
        case COLOR_BLACK:
                return "black";
        default:
                return "empty";
        }
}

const char *token_letter(struct token token)
{
        switch (token.piece) {
        case PIECE_KING:
                return token.color == COLOR_WHITE ? "K" : "k";
        case PIECE_QUEEN:
                return token.color == COLOR_WHITE ? "Q" : "q";
        case PIECE_ROOK:
                return token.color == COLOR_WHITE ? "R" : "r";
        case PIECE_BISHOP:
                return token.color == COLOR_WHITE ? "B" : "b";
        case PIECE_KNIGHT:
                return token.color == COLOR_WHITE ? "N" : "n";
        case PIECE_PAWN:
                return token.color == COLOR_WHITE ? "P" : "p";
        default:
                return ".";
        }
}

const char *token_string(struct token token)
{
        switch (token.piece) {
        case PIECE_KING:
                return token.color == COLOR_WHITE ? "white king" : "black king";
        case PIECE_QUEEN:
                return token.color == COLOR_WHITE ? "white queen" : "black queen";
        case PIECE_ROOK:
                return token.color == COLOR_WHITE ? "white rook" : "black rook";
        case PIECE_BISHOP:
                return token.color == COLOR_WHITE ? "white bishop" : "black bishop";
        case PIECE_KNIGHT:
                return token.color == COLOR_WHITE ? "white knight" : "black knight";
        case PIECE_PAWN:
                return token.color == COLOR_WHITE ? "white pawn" : "black pawn";
        default:
                return "empty";
        }
}

const char *token_symbol(struct token token)
{
        switch (token.piece) {
        case PIECE_KING:
                return token.color == COLOR_WHITE ? "♔" : "♚";
        case PIECE_QUEEN:
                return token.color == COLOR_WHITE ? "♕" : "♛";
        case PIECE_ROOK:
                return token.color == COLOR_WHITE ? "♖" : "♜";
        case PIECE_BISHOP:
                return token.color == COLOR_WHITE ? "♗" : "♝";
        case PIECE_KNIGHT:
                return token.color == COLOR_WHITE ? "♘" : "♞";
        case PIECE_PAWN:
                return token.color == COLOR_WHITE ? "♙" : "♟︎";
        default:
                return " ";
        }
}

bool assert_valid_color(enum color color)
{
        return COLOR_NONE <= color && color <= COLOR_BLACK;
}

bool assert_valid_piece(enum piece piece)
{
        return PIECE_NONE <= piece && piece <= PIECE_PAWN;
}

bool assert_valid_token(struct token token)
{
        return assert_valid_color(token.color) && assert_valid_piece(token.piece);
}

bool assert_valid_file(enum file file)
{
        return FILE_A <= file && file <= FILE_H;
}

bool assert_valid_rank(enum rank rank)
{
        return RANK_1 <= rank && rank <= RANK_8;
}

bool assert_valid_tier(enum tier tier)
{
        return TIER_S <= tier && tier <= TIER_Z;
}

bool assert_valid_block(struct block block)
{
        return assert_valid_file(block.file) && assert_valid_rank(block.rank) && assert_valid_tier(block.tier);
}

bool assert_valid_moves(struct moves moves)
{
        return 0 <= moves.length && moves.length <= MOVES_BLOCKS_LENGTH;
}

struct token create_token(enum color color, enum piece piece)
{
        return (struct token){.color = color, .piece = piece, .moved = false};
}

struct block create_block(enum file file, enum rank rank, enum tier tier)
{
        return (struct block){.file = file, .rank = rank, .tier = tier};
}

struct board create_board(void)
{
        struct board board;
        memcpy(board.tokens, DEFAULT_BOARD_TOKENS, BOARD_TOKENS_LENGTH * sizeof(struct token));

        return board;
}

struct arena create_arena(void)
{
        struct arena arena;
        memcpy(arena.tokens, DEFAULT_ARENA_TOKENS, ARENA_TOKENS_LENGTH * sizeof(struct token));

        return arena;
}

struct moves create_moves(void)
{
        return (struct moves){.blocks = {0}, .length = 0};
}

size_t obtain_block_index(struct block block)
{
        return ARENA_BLOCK_INDEX(block.file, block.rank, block.tier);
}

enum color obtain_block_color(struct block block)
{
        if (!assert_valid_block(block))
                return COLOR_NONE;

        uint64_t black_blocks = 0xAA55AA55AA55AA55;
        if (block.tier % 2 != 0)
                black_blocks = ~black_blocks;

        return ((black_blocks >> BOARD_BLOCK_INDEX(block.file, block.rank)) & 1) == 0 ? COLOR_WHITE : COLOR_BLACK;
}

struct token obtain_board_token(struct board board, struct block block)
{
        return board.tokens[obtain_block_index(block)];
}

struct token obtain_arena_token(struct arena arena, struct block block)
{
        return arena.tokens[obtain_block_index(block)];
}

struct moves obtain_board_moves_piece_king(struct board board, struct token token, struct block origin)
{
        struct moves moves = create_moves();
        if (!assert_valid_block(origin) || token.piece != PIECE_KING)
                return moves;

        // TODO: In check?

        // Normal moves.
        for (size_t dr = -1; dr <= 1; dr++)
        for (size_t df = -1; df <= 1; df++) {
                if (dr == 0 && df == 0)
                        continue;

                struct block block = create_block(origin.file + df, origin.rank + dr, origin.tier);
                if (!assert_valid_block(block))
                        continue;

                if (obtain_board_token(board, block).color != token.color)
                        continue;

                moves.blocks[moves.length++] = block;
        }

        // Castling kingside. TODO: Ray march.
        struct token kside_token = obtain_board_token(board, create_block(FILE_A, origin.rank, origin.tier));
        if (!token.moved && kside_token.piece == PIECE_ROOK && kside_token.color == token.color && !kside_token.moved)
                moves.blocks[moves.length++] = create_block(FILE_B, origin.rank, origin.tier);

        // Castling queenside. TODO: Ray march.
        struct token qside_token = obtain_board_token(board, create_block(FILE_H, origin.rank, origin.tier));
        if (!token.moved && qside_token.piece == PIECE_ROOK && qside_token.color == token.color && !qside_token.moved)
                moves.blocks[moves.length++] = create_block(FILE_G, origin.rank, origin.tier);

        return moves;       
}

struct moves obtain_board_moves_piece_queen(struct board board, struct token token, struct block origin)
{
        struct moves moves = create_moves();
        if (!assert_valid_block(origin) || token.piece != PIECE_QUEEN)
                return moves;

        struct moves rook_moves = obtain_board_moves_piece_rook(board, token, origin);
        if (assert_valid_moves(rook_moves)) {
                memcpy(moves.blocks + moves.length, rook_moves.blocks, rook_moves.length * sizeof(struct block));
                moves.length += rook_moves.length;
        }

        struct moves bishop_moves = obtain_board_moves_piece_bishop(board, token, origin);
        if (assert_valid_moves(bishop_moves)) {
                memcpy(moves.blocks + moves.length, bishop_moves.blocks, bishop_moves.length * sizeof(struct block));
                moves.length += bishop_moves.length;
        }

        return moves;
}

struct moves obtain_board_moves_piece_rook(struct board board, struct token token, struct block origin)
{
        struct moves moves = create_moves();
        if (!assert_valid_block(origin) || token.piece != PIECE_ROOK)
                return moves;

        const size_t dr[4] = {-1,  0,  1,  0};
        const size_t df[4] = { 0,  1,  0, -1};
        for (size_t d = 0; d < 4; d++)
        for (size_t i = 1; i < BLOCK_COLUMN_LENGTH; i++) {
                struct block block = create_block(origin.file + df[d] * i, origin.rank + dr[d] * i, origin.tier);
                if (!assert_valid_block(block))
                        break;

                struct token other = obtain_board_token(board, block);
                if (other.color != token.color)
                        moves.blocks[moves.length++] = block;
                if (other.piece != PIECE_NONE)
                        break;
        }

        // TODO: Castling. (Is that even a rook move, or only a king move?)

        return moves;
}

struct moves obtain_board_moves_piece_bishop(struct board board, struct token token, struct block origin)
{
        struct moves moves = create_moves();
        if (!assert_valid_block(origin) || token.piece != PIECE_BISHOP)
                return moves;

        const size_t dr[4] = {-1, -1,  1,  1};
        const size_t df[4] = {-1,  1,  1, -1};
        for (size_t d = 0; d < 4; d++)
        for (size_t i = 1; i < BLOCK_COLUMN_LENGTH; i++) {
                struct block block = create_block(origin.file + df[d] * i, origin.rank + dr[d] * i, origin.tier);
                if (!assert_valid_block(block))
                        break;

                struct token other = obtain_board_token(board, block);
                if (other.color != token.color)
                        moves.blocks[moves.length++] = block;
                if (other.piece != PIECE_NONE)
                        break;
        }

        return moves;
}

struct moves obtain_board_moves_piece_knight(struct board board, struct token token, struct block origin)
{
        struct moves moves = create_moves();
        if (!assert_valid_block(origin) || token.piece != PIECE_KNIGHT)
                return moves;

        const size_t dr[8] = {-2, -1,  1,  2,  2,  1, -1, -2};
        const size_t df[8] = { 1,  2,  2,  1, -1, -2, -2, -1};
        for (size_t d = 0; d < 8; d++) {
                struct block block = create_block(origin.file + df[d], origin.rank + dr[d], origin.tier);
                if (!assert_valid_block(block))
                        continue;

                struct token other = obtain_board_token(board, block);
                if (other.color != token.color)
                        moves.blocks[moves.length++] = block;
        }

        return moves;
}

struct moves obtain_board_moves_piece_pawn(struct board board, struct token token, struct block origin)
{
        struct moves moves = create_moves();
        if (!assert_valid_block(origin) || token.piece != PIECE_PAWN)
                return moves;

        struct token other = create_token(COLOR_NONE, PIECE_NONE);
        struct block block = create_block(origin.file, origin.rank + 1, origin.tier);
        if (assert_valid_block(block) && (other = obtain_board_token(board, block)).piece == PIECE_NONE)
                moves.blocks[moves.length++] = block;

        block = create_block(origin.file - 1, origin.rank + 1, origin.tier);
        if (assert_valid_block(block) && (other = obtain_board_token(board, block)).color != token.color && other.piece != PIECE_NONE)
                moves.blocks[moves.length++] = block;

        block = create_block(origin.file + 1, origin.rank + 1, origin.tier);
        if (assert_valid_block(block) && (other = obtain_board_token(board, block)).color != token.color && other.piece != PIECE_NONE)
                moves.blocks[moves.length++] = block;

        // TODO: En passant.

        // TODO: Promotion.

        // Double.
        block = create_block(origin.file, origin.rank + 2, origin.tier);
        if (!token.moved && assert_valid_block(block) && (other = obtain_board_token(board, block)).piece == PIECE_NONE)
                moves.blocks[moves.length++] = block;

        return moves;
}

struct moves obtain_arena_moves_piece_king(struct arena arena, struct token token, struct block origin)
{
        struct moves moves = create_moves();
        if (!assert_valid_block(origin) || token.piece != PIECE_KING)
                return moves;

        return moves;
}

struct moves obtain_arena_moves_piece_queen(struct arena arena, struct token token, struct block origin)
{
        struct moves moves = create_moves();
        if (!assert_valid_block(origin) || token.piece != PIECE_QUEEN)
                return moves;

        return moves;
}

struct moves obtain_arena_moves_piece_rook(struct arena arena, struct token token, struct block origin)
{
        struct moves moves = create_moves();
        if (!assert_valid_block(origin) || token.piece != PIECE_ROOK)
                return moves;

        return moves;
}

struct moves obtain_arena_moves_piece_bishop(struct arena arena, struct token token, struct block origin)
{
        struct moves moves = create_moves();
        if (!assert_valid_block(origin) || token.piece != PIECE_BISHOP)
                return moves;

        return moves;
}

struct moves obtain_arena_moves_piece_knight(struct arena arena, struct token token, struct block origin)
{
        struct moves moves = create_moves();
        if (!assert_valid_block(origin) || token.piece != PIECE_KNIGHT)
                return moves;

        return moves;
}

struct moves obtain_arena_moves_piece_pawn(struct arena arena, struct token token, struct block origin)
{
        struct moves moves = create_moves();
        if (!assert_valid_block(origin) || token.piece != PIECE_PAWN)
                return moves;

        return moves;
}

struct moves obtain_board_moves(struct board board, struct token token, struct block origin)
{
        struct moves moves;
        switch (token.piece) {
        case PIECE_KING:
                moves = obtain_board_moves_piece_king(board, token, origin);
                break;
        case PIECE_QUEEN:
                moves = obtain_board_moves_piece_queen(board, token, origin);
                break;
        case PIECE_ROOK:
                moves = obtain_board_moves_piece_rook(board, token, origin);
                break;
        case PIECE_BISHOP:
                moves = obtain_board_moves_piece_bishop(board, token, origin);
                break;
        case PIECE_KNIGHT:
                moves = obtain_board_moves_piece_knight(board, token, origin);
                break;
        case PIECE_PAWN:
                moves = obtain_board_moves_piece_pawn(board, token, origin);
                break;
        default:
                moves = create_moves();
                break;
        }

        return moves;
}

struct moves obtain_arena_moves(struct arena arena, struct token token, struct block origin)
{
        struct moves moves;
        switch (token.piece) {
        case PIECE_KING:
                moves = obtain_arena_moves_piece_king(arena, token, origin);
                break;
        case PIECE_QUEEN:
                moves = obtain_arena_moves_piece_queen(arena, token, origin);
                break;
        case PIECE_ROOK:
                moves = obtain_arena_moves_piece_rook(arena, token, origin);
                break;
        case PIECE_BISHOP:
                moves = obtain_arena_moves_piece_bishop(arena, token, origin);
                break;
        case PIECE_KNIGHT:
                moves = obtain_arena_moves_piece_knight(arena, token, origin);
                break;
        case PIECE_PAWN:
                moves = obtain_arena_moves_piece_pawn(arena, token, origin);
                break;
        default:
                moves = create_moves();
                break;
        }

        return moves;
}

struct board obtain_arena_board_axis_file(struct arena arena, enum file file)
{
        struct board board;

        size_t index = 0;
        for (enum tier tier = TIER_S; tier <= TIER_Z; tier++)
        for (enum rank rank = RANK_1; rank <= RANK_8; rank++)
                board.tokens[index++] = arena.tokens[obtain_block_index(create_block(file, rank, tier))];

        return board;
}

struct board obtain_arena_board_axis_rank(struct arena arena, enum rank rank)
{
        struct board board;

        size_t index = 0;
        for (enum tier tier = TIER_S; tier <= TIER_Z; tier++)
        for (enum file file = FILE_A; file <= FILE_H; file++)
                board.tokens[index++] = arena.tokens[obtain_block_index(create_block(file, rank, tier))];

        return board;
}

struct board obtain_arena_board_axis_tier(struct arena arena, enum tier tier)
{
        struct board board;

        size_t index = 0;
        for (enum rank rank = RANK_1; rank <= RANK_8; rank++)
        for (enum file file = FILE_A; file <= FILE_H; file++)
                board.tokens[index++] = arena.tokens[obtain_block_index(create_block(file, rank, tier))];

        return board;
}

struct token update_board_token(struct board *board, struct block block, struct token token)
{
        struct token prev = board->tokens[obtain_block_index(block)];
        board->tokens[obtain_block_index(block)] = token;

        return prev;
}

struct token update_arena_token(struct arena *arena, struct block block, struct token token)
{
        struct token prev = arena->tokens[obtain_block_index(block)];
        arena->tokens[obtain_block_index(block)] = token;

        return prev;
}
