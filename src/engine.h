#pragma once

#include <stddef.h>
#include <stdbool.h>

#define BLOCK_COLUMN_LENGTH 8
#define BOARD_TOKENS_LENGTH 64
#define ARENA_TOKENS_LENGTH 512
#define MOVES_BLOCKS_LENGTH 128

enum color {
        COLOR_NONE,
        COLOR_WHITE,
        COLOR_BLACK,
};

enum piece {
        PIECE_NONE,
        PIECE_KING,
        PIECE_QUEEN,
        PIECE_ROOK,
        PIECE_BISHOP,
        PIECE_KNIGHT,
        PIECE_PAWN,
};

struct token {
        enum color color;
        enum piece piece;
        bool       moved;
};

enum file {
        FILE_A,
        FILE_B,
        FILE_C,
        FILE_D,
        FILE_E,
        FILE_F,
        FILE_G,
        FILE_H,
};

enum rank {
        RANK_1,
        RANK_2,
        RANK_3,
        RANK_4,
        RANK_5,
        RANK_6,
        RANK_7,
        RANK_8,
};

enum tier {
        TIER_S,
        TIER_T,
        TIER_U,
        TIER_V,
        TIER_W,
        TIER_X,
        TIER_Y,
        TIER_Z,
};

struct block {
        enum file file;
        enum rank rank;
        enum tier tier;
};

struct board {
        struct token tokens[BOARD_TOKENS_LENGTH];
};

struct arena {
        struct token tokens[ARENA_TOKENS_LENGTH];
};

struct moves {
        struct block blocks[MOVES_BLOCKS_LENGTH];
        size_t       length;
};

enum color   invert_color(enum color color);

bool assert_valid_color(enum color color);
bool assert_valid_piece(enum piece piece);
bool assert_valid_token(struct token token);
bool assert_valid_file(enum file file);
bool assert_valid_rank(enum rank rank);
bool assert_valid_tier(enum tier tier);
bool assert_valid_block(struct block block);
bool assert_valid_moves(struct moves moves);

struct token create_token(enum color color, enum piece piece);
struct block create_block(enum file file, enum rank rank, enum tier tier);
struct board create_board(void);
struct arena create_arena(void);
struct moves create_moves(void);

size_t       obtain_block_index(struct block block);
enum color   obtain_block_color(struct block block);
struct token obtain_board_token(struct board board, struct block block);
struct token obtain_arena_token(struct arena arena, struct block block);
struct moves obtain_board_moves(struct board board, struct token token, struct block origin);
struct moves obtain_arena_moves(struct arena arena, struct token token, struct block origin);

struct token update_board_token(struct board *board, struct block block, struct token token);
struct token update_arena_token(struct arena *arena, struct block block, struct token token);


