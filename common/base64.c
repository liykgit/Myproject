
#include <string.h>
#include "base64.h"


static char base64_index[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

int base64_encode(unsigned char *in, int length, char *out)
{
	unsigned char *end;
	int count = 0;

    if ((in == NULL)
		|| (length < 1)
		|| (out == NULL))
		return 0;

    end = in + length;
    *out = '\0';
    // 0x30 -> 00110000
    // 0x3C -> 00111100
    // 0x3F -> 00111111
    while (end - in >= 3) {
        *out++ = base64_index[( in[0] >> 2 )];
        *out++ = base64_index[( (in[0] << 4) & 0x30 ) | ( in[1] >> 4 )];
        *out++ = base64_index[( (in[1] << 2) & 0x3C ) | ( in[2] >> 6 )];
        *out++ = base64_index[in[2] & 0x3F];
        in += 3;
		count += 4;
    }

    if (end - in > 0) {
        *out++ = base64_index[(in[0] >> 2)];
        if (end - in == 2) {
            *out++ = base64_index[( (in[0] << 4) & 0x30 ) | (in[1] >> 4 )];
            *out++ = base64_index[(in[1] << 2) & 0x3C];
            *out++ = '=';
			count += 3;
        } else if (end - in == 1) {
//            *out++ = base64_index[(in[1] << 4) & 0x30];
			*out++ = base64_index[(in[0] << 4) & 0x30];
            *out++ = '=';
            *out++ = '=';
			count += 3;
        }
    }

    *out = '\0';
    return count;
}

// 16 * 16
static unsigned char base64_decode_map[256] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 0   - 15
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 16  - 31
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63, // 32  - 47
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1, // 48  - 63
    -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, // 64  - 79
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1, // 80  - 95
    -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, // 96  - 111
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1  // 112 - 127
};

int base64_decode(char* in, unsigned char *out)
{
	int len = 0, count = 0;
	unsigned char *p, *p_end;

    if ((in == NULL)
		|| (out == NULL))
        return 0;

    len = strlen(in);
    if ((len < 4)
		|| (len % 4 != 0))
        return 0;

	out[0] = '\0';

    // 0xFC -> 11111100
    // 0x03 -> 00000011
    // 0xF0 -> 11110000
    // 0x0F -> 00001111
    // 0xC0 -> 11000000
    p = (unsigned char *)in;
    p_end = (unsigned char *)(in + len);
    for (; p < p_end; p += 4)
	{
        *out++ = ((base64_decode_map[p[0]] << 2) & 0xFC) | ((base64_decode_map[p[1]] >> 4) & 0x03);
        *out++ = ((base64_decode_map[p[1]] << 4) & 0xF0) | ((base64_decode_map[p[2]] >> 2) & 0x0F);
        *out++ = ((base64_decode_map[p[2]] << 6) & 0xC0) | (base64_decode_map[p[3]]);
		count += 3;
    }

    if (*(in + len - 2) == '=')
	{
        *(out - 2) = '\0';
		count -= 2;
    }
	else if (*(in + len - 1) == '=')
	{
        *(out - 1) = '\0';
		count -= 1;
    }

    return count;
}
