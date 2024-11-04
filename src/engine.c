#include "engine.h"

#include <stddef.h>

#define COLOR_BIT_MASK  0x18
#define PIECE_BIT_MASK  0x07
#define PIECE_BIT_WIDTH 3
#define BITBOARD_NIL    ((bitboard_t)0)
#define BITBOARD_ALL    ((bitboard_t)0xFFFFFFFFFFFFFFFF)

#define BITBOARD

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
        if (piece_bits < 1 || piece_bits > 6)
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

int obtain_piece_popcount(bitboard_t bitboard)
{
        // https://www.chessprogramming.org/Population_Count
        const bitboard_t K1 = (bitboard_t)0x5555555555555555;
        const bitboard_t K2 = (bitboard_t)0x3333333333333333;
        const bitboard_t K4 = (bitboard_t)0x0f0f0f0f0f0f0f0f;
        const bitboard_t KF = (bitboard_t)0x0101010101010101;

        bitboard = (bitboard - ((bitboard >> 1) & K1);
        bitboard = (bitboard & k2) + ((bitboard >> 2) & K2);
        bitboard = (bitboard + (bitboard >> 4)) & K4;
        bitboard = (bitboard * KF) >> 56;

        return (int)bitboard;
}

int forward_scan_bitboard(bitboard_t bitboard)
{
        if (bitboard == 0)
                return -1;

        return obtain_piece_popcount((bitboard & -bitboard) - 1);
}

int reverse_scan_bitboard(bitboard_t bitboard)
{
        int ret = 0;
        if (bitboard > 0xFFFFFFFF) {
                bitboard >>= 32;
                ret = 32;
        }
        if (bitboard > 0xFFFF) {
                bitboard >>= 16;
                ret += 16;
        }
        if (bitboard > 0xFF) {
                bitboard >>= 8;
                ret += 8;
        }

        // TODO: Implement ms1b_table (https://www.talkchess.com/forum/viewtopic.php?t=38777).
        return ret + ms1b_table[bitboard];
}

int general_scan_bitboard(bitboard_t bitboard, bool reverse)
{
        if (bitboard == 0)
                return -1;

        bitboard_t rmask = -(bitboard_t)reverse;
        bitboard &= -bitboard | rmask;

        return reverse_scan_bitboard(bitboard);
}

bitboard_t obtain_color_bitboard(struct board board, enum color color)
{
        return board.pieces[(int)color - 1];
}

bitboard_t obtain_piece_bitboard(struct board board, enum piece piece)
{
        return board.pieces[(int)piece + 1];
}
