#include "engine.h"

#include <stddef.h>

#define COLOR_BIT_MASK  0x18
#define PIECE_BIT_MASK  0x07
#define PIECE_BIT_WIDTH 3

enum color invert_color(enum color color)
{
        return (color == COLOR_WHITE) ? COLOR_BLACK : COLOR_WHITE;
}

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

bitboard_t obtain_color_bitboard(struct board board, enum color color)
{
        return board.pieces[(int)color - 1];
}

bitboard_t obtain_piece_bitboard(struct board board, enum piece piece)
{
        return board.pieces[(int)piece + 1];
}

bitboard_t obtain_board(struct board board, enum color *color, enum piece *piece)
{
        bitboard_t color_bb = 0;
        if (color != NULL)
                color_bb = obtain_color_bitboard(board, *color);

        bitboard_t piece_bb = 0;
        if (piece != NULL)
                piece_bb = obtain_piece_bitboard(board, *piece);

        return color_bb & piece_bb;
}
