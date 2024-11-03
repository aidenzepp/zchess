#pragma once

#include <stdint.h>
#include <stdbool.h>

enum color {
        COLOR_WHITE = 1,
        COLOR_BLACK,
};

enum piece {
        PIECE_KING,
        PIECE_QUEEN,
        PIECE_ROOK,
        PIECE_BISHOP,
        PIECE_KNIGHT,
        PIECE_PAWN,
};

bool    decode_color(uint8_t bits, enum color *color);
bool    decode_piece(uint8_t bits, enum piece *piece);
uint8_t encode_color(uint8_t bits, enum color color);
uint8_t encode_piece(uint8_t bits, enum piece piece);
