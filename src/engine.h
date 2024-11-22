#pragma once

#include <stddef.h>
#include <stdbool.h>

#define BLOCK_COLUMN_LENGTH 8
#define ARENA_TOKENS_LENGTH 512
#define MOVES_BLOCKS_LENGTH 256

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

struct moves {
        struct block blocks[MOVES_BLOCKS_LENGTH];
        size_t       length;
};

struct arena {
        struct token tokens[ARENA_TOKENS_LENGTH];
};

struct arena create_arena(void);
struct moves obtain_arena_moves(struct arena arena, enum file file, enum rank rank, enum tier tier);
struct token obtain_arena_token(struct arena arena, enum file file, enum rank rank, enum tier tier);
