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

struct block {
        enum rank rank;
        enum file file;
        enum tier tier;
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

int        obtain_piece_popcount(bitboard_t bitboard);
int        forward_scan_bitboard(bitboard_t bitboard);
int        reverse_scan_bitboard(bitboard_t bitboard);
int        general_scan_bitboard(bitboard_t bitboard, bool reverse);
bitboard_t flip_across_vert_line(bitboard_t bitboard);
bitboard_t flip_across_horz_line(bitboard_t bitboard);
bitboard_t flip_across_norm_diag(bitboard_t bitboard);
bitboard_t flip_across_anti_diag(bitboard_t bitboard);
bitboard_t general_flip_bitboard(bitboard_t bitboard, bool horz, bool vert);
bitboard_t rotl_by_45dg_bitboard(bitboard_t bitboard);
bitboard_t rotr_by_45dg_bitboard(bitboard_t bitboard);
bitboard_t rotl_by_90dg_bitboard(bitboard_t bitboard);
bitboard_t rotr_by_90dg_bitboard(bitboard_t bitboard);
bitboard_t rot_by_180dg_bitboard(bitboard_t bitboard);
bitboard_t general_turn_bitboard(bitboard_t bitboard, uint8_t deg, bool ccw);
bitboard_t obtain_color_bitboard(struct board board, enum color color);
bitboard_t obtain_piece_bitboard(struct board board, enum piece piece);
