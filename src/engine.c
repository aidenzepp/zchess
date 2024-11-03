#include "engine.h"

#include <stddef.h>

#define COLOR_BIT_MASK  0x18
#define PIECE_BIT_MASK  0x0F
#define PIECE_BIT_WIDTH 3

bool decode_color(uint8_t bits, enum color *color)
{
        uint8_t color_bits = (bits & COLOR_BIT_MASK) >> PIECE_BIT_WIDTH;
        if (color_bits == 0)
                return false;

        *color = (enum color)color_bits;
        return true;
}

bool decode_piece(uint8_t bits, enum piece *piece)
{
        uint8_t piece_bits = (bits & PIECE_BIT_MASK);
        if (piece_bits == 0)
                return false;

        *piece = (enum piece)piece_bits;
        return true;
}

uint8_t encode_color(uint8_t bits, enum color color)
{
        return bits | ((uint8_t)color << PIECE_BIT_WIDTH);
}

uint8_t encode_piece(uint8_t bits, enum piece piece)
{
        return bits | (uint8_t)piece;
}
