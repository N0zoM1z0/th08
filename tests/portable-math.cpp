// Golden values use 80-digit decimal trigonometry of the exact float inputs,
// multiplication at that precision, then one rounding to IEEE-754 float32.
// Retail 0x4286E0/0x428700 multiply FSINCOS results before FSTP stores.
#include "ZunMath.hpp"
#include <stdio.h>
#include <string.h>

static float FromBits(unsigned int bits)
{
    float value;
    memcpy(&value, &bits, sizeof(value));
    return value;
}

static unsigned int Bits(float value)
{
    unsigned int bits;
    memcpy(&bits, &value, sizeof(bits));
    return bits;
}

int main()
{
    const unsigned int cases[][4] = {
        {0xc017ae14, 0x41400000, 0xc109a034, 0xc105e099},
        {0x3dfcd680, 0x41400000, 0x413e89e7, 0x3fbd25a7},
        {0x3f9e0419, 0x41500000, 0x408946f2, 0x4144593a},
        {0x4039999a, 0x40500000, 0xc049f595, 0x3f470e2c},
        {0x3d91e109, 0x404ccccd, 0x404c47da, 0x3e69358b}
    };
    int failures = 0;
    for (unsigned int i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i)
    {
        float angle = FromBits(cases[i][0]), magnitude = FromBits(cases[i][1]);
        th08::Float3 vector;
        vector.FromAngleMagnitude(angle, magnitude);
        if (Bits(vector.x) != cases[i][2] || Bits(vector.y) != cases[i][3])
        {
            fprintf(stderr, "FAIL: FromAngleMagnitude case %u: %08x %08x\n", i, Bits(vector.x), Bits(vector.y));
            ++failures;
        }
        vector.FromRotatedVec2(angle, magnitude, magnitude);
        if (Bits(vector.x) != cases[i][2] || Bits(vector.y) != cases[i][3])
        {
            fprintf(stderr, "FAIL: FromRotatedVec2 case %u: %08x %08x\n", i, Bits(vector.x), Bits(vector.y));
            ++failures;
        }
    }
    if (!failures)
        fprintf(stdout, "Portable vector precision: all 10 golden contracts passed\n");
    return failures ? 1 : 0;
}
