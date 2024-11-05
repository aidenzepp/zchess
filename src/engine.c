#include "engine.h"

#include <stddef.h>

#define COLOR_BIT_MASK  0x18
#define PIECE_BIT_MASK  0x07
#define PIECE_BIT_WIDTH 3

#define BITBOARD_NIL        ((bitboard_t)0)
#define BITBOARD_ALL        ((bitboard_t)0xFFFFFFFFFFFFFFFF)
#define BITBOARD_NOT_A_FILE ((bitboard_t)0xFEFEFEFEFEFEFEFE)
#define BITBOARD_NOT_H_FILE ((bitboard_t)0x7f7f7f7f7f7f7f7f)

static bitboard_t nort_one(bitboard_t bitboard)
{
        return bitboard << 8;
}

static bitboard_t sout_one(bitboard_t bitboard)
{
        return bitboard >> 8;
}

static bitboard_t west_one(bitboard_t bitboard)
{
        return (bitboard & BITBOARD_NOT_A_FILE) >> 1;
}

static bitboard_t nowe_one(bitboard_t bitboard)
{
        return (bitboard & BITBOARD_NOT_A_FILE) << 7;
}

static bitboard_t sowe_one(bitboard_t bitboard)
{
        return (bitbaord & BITBOARD_NOT_A_FILE) >> 9;
}

static bitboard_t east_one(bitboard_t bitboard)
{
        return (bitboard & BITBOARD_NOT_H_FILE) << 1;
}

static bitboard_t noea_one(bitboard_t bitboard)
{
        return (bitboard & BITBOARD_NOT_H_FILE) << 9;
}

static bitboard_t soea_one(bitboard_t bitboard)
{
        return (bitboard & BITBOARD_NOT_H_FILE) >> 7;
}

static bitboard_t rotl(bitboard_t bitboard, int shift)
{
        return (bitboard << shift) | (bitboard >> (64 - shift));
}

static bitboard_t rotr(bitboard_t bitboard, int shift)
{
        return (bitboard >> shift) | (bitboard << (64 - shift));
}

static bitboard_t general_shift_bitboard(bitboard_t bitboard, int shift)
{
        return (s > 0) ? (bitboard << shift) : (bitboard >> -shift);
}

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

/*
 * For some stupid reason (or reason I don't understand, who knows), the Chess Wiki
 * calls "flip_across_vert_line" and "flip_across_horz_line" as "flipVertical" and
 * "mirrorHorizontal", respectively. Unsure why they use "flip" and "mirror" instead
 * of just "flip". Whatever.
 */

bitboard_t flip_across_vert_line(bitboard_t bitboard)
{
        const bitboard_t K1 = (bitboard_t)0x5555555555555555;
        const bitboard_t K2 = (bitboard_t)0x3333333333333333;
        const bitboard_t K4 = (bitboard_t)0x0f0f0f0f0f0f0f0f;

        bitboard = ((bitboard >> 1) & K1) | ((bitboard & K1) << 1);
        bitboard = ((bitboard >> 2) & K2) | ((bitboard & K2) << 2);
        bitboard = ((bitboard >> 4) & K4) | ((bitboard & K4) << 4);

        return bitboard;
}

bitboard_t flip_across_horz_line(bitboard_t bitboard)
{
        const bitboard_t K1 = (bitboard_t)0x00FF00FF00FF00FF;
        const bitboard_t K2 = (bitboard_t)0x0000FFFF0000FFFF;

        bitboard = ((bitboard >>  8) & K1) | ((bitboard & K1) << 8);
        bitboard = ((bitboard >> 16) & K2) | ((bitboard & K2) << 16);
        bitboard = (bitboard >> 32) | (bitboard << 32);

        return bitboard;
}

// A1 - H8
bitboard_t flip_across_norm_diag(bitboard_t bitboard)
{
        const bitboard_t K1 = (bitboard_t)0x5500550055005500;
        const bitboard_t K2 = (bitboard_t)0x3333000033330000;
        const bitboard_t K4 = (bitboard_t)0x0f0f0f0f00000000;

        bitboard_t tmp;
        tmp = K4 & (bitboard ^ (bitboard << 28));
        bitboard ^= tmp ^ (tmp >> 28);
        tmp = K2 & (bitboard ^ (bitboard << 14));
        bitboard ^= tmp ^ (tmp >> 14);
        tmp = K1 & (bitboard ^ (bitboard <<  7));
        bitboard ^= tmp ^ (tmp >>  7);

        return bitboard;
}

// A8 - H1
bitboard_t flip_across_anti_diag(bitboard_t bitboard)
{
        const bitboard_t K1 = (bitboard_t)0xaa00aa00aa00aa00;
        const bitboard_t K2 = (bitboard_t)0xcccc0000cccc0000;
        const bitboard_t K4 = (bitboard_t)0xf0f0f0f00f0f0f0f;

        bitboard_t tmp;
        tmp = bitboard ^ (bitboard << 36);
        bitboard ^= K4 & (tmp ^ (bitboard >> 36));
        tmp = K2 & (bitboard ^ (bitboard << 18));
        bitboard ^= tmp ^ (tmp >> 18);
        tmp = K1 & (bitboard ^ (bitboard <<  9));
        bitboard ^= tmp ^ (tmp >>  9);

        return ret;
}

bitboard_t general_flip_bitboard(bitboard_t bitboard, bool horz, bool vert)
{
        for (uint32_t i = 3 * (1 - vert); i < 3 * (1 + horz); i++) {
                int s = 1 << i;
                uint64_t f = (uint64_t)1 << s;
                uint64_t k = (uint64_t)(-1) / (f + 1);
                bitboard = ((bitboard >> s) & k) + (f * (bitboard & k));
        }

        return bitboard;
}

bitboard_t rotl_by_45dg_bitboard(bitboard_t bitboard)
{
        const bitboard_t K1 = (bitboard_t)0x5555555555555555;
        const bitboard_t K2 = (bitboard_t)0x3333333333333333;
        const bitboard_t K4 = (bitboard_t)0x0f0f0f0f0f0f0f0f;

        bitboard ^= K1 & (bitboard ^ rotr(bitboard, 8));
        bitboard ^= K2 & (bitboard ^ rotr(bitboard, 16));
        bitboard ^= K4 & (bitboard ^ rotr(bitboard, 32));

        return bitboard;
}

bitboard_t rotr_by_45dg_bitboard(bitboard_t bitboard)
{
        const bitboard_t K1 = (bitboard_t)0xAAAAAAAAAAAAAAAA;
        const bitboard_t K2 = (bitboard_t)0xCCCCCCCCCCCCCCCC;
        const bitboard_t K4 = (bitboard_t)0xF0F0F0F0F0F0F0F0;

        bitboard ^= K1 & (bitboard ^ rotr(bitboard, 8));
        bitboard ^= K2 & (bitboard ^ rotr(bitboard, 16));
        bitboard ^= K4 & (bitboard ^ rotr(bitboard, 32));

        return bitboard;
}

bitboard_t rotl_by_90dg_bitboard(bitboard_t bitboard)
{
        return flip_across_horz_line(flip_across_anti_diag(bitboard));
}

bitboard_t rotr_by_90dg_bitbaord(bitboard_t bitboard)
{
        return flip_across_horz_line(flip_across_norm_diag(bitboard));
}

bitboard_t rot_by_180dg_bitboard(bitboard_t bitboard)
{
        return flip_across_vert_line(flip_across_horz_line(bitboard));
}

bitboard_t obtain_color_bitboard(struct board board, enum color color)
{
        return board.pieces[(size_t)color - 1];
}

bitboard_t obtain_piece_bitboard(struct board board, enum piece piece)
{
        return board.pieces[(size_t)piece + 1];
}
