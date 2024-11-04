#pragma once

#include <stdint.h>
#include <stdbool.h>

enum color {
        COLOR_WHITE = 1,
        COLOR_BLACK,
};

enum piece {
        PIECE_KING = 1,
        PIECE_QUEEN,
        PIECE_ROOK,
        PIECE_BISHOP,
        PIECE_KNIGHT,
        PIECE_PAWN,
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

typedef uint64_t bitboard_t;

struct board {
        bitboard_t pieces[8];
};

enum color invert_color(enum color color);
bool       decode_color(uint8_t bits, enum color *color);
bool       decode_piece(uint8_t bits, enum piece *piece);
uint8_t    encode_color(uint8_t bits, enum color color);
uint8_t    encode_piece(uint8_t bits, enum piece piece);
bitboard_t obtain_color_bitboard(struct board board, enum color color);
bitboard_t obtain_piece_bitboard(struct board board, enum piece piece);
bitboard_t obtain_board(struct board board, enum color *color, enum piece *piece);
