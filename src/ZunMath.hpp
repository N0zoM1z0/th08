#pragma once
#include "inttypes.hpp"
#ifdef TH08_MODERN_PORT
#include <math.h>
#endif

#define ZUN_MIN(x, y) ((x) < (y) ? (x) : (y))
#define ZUN_MAX(x, y) ((x) > (y) ? (x) : (y))
#define ZUN_PI ((f32)(3.14159265358979323846))
#define ZUN_2PI ((f32)(ZUN_PI * 2.0f))

namespace th08
{

inline void IncrementIfBelow(u32 *value, u32 threshold)
{
    if (*value < threshold)
    {
        (*value)++;
    }
}

/* ZUN name: FVector */
struct Float3
{
    Float3()
    {
    }

    Float3(float x, float y, float z);

    void FromAngleMagnitude(float angle, float magnitude)
    {
#ifdef TH08_MODERN_PORT
        this->x = cosf(angle) * magnitude;
        this->y = sinf(angle) * magnitude;
#else
        __asm
        {
            mov eax, this
            fld angle
            fsincos
            fmul [magnitude]
            fstp [eax] /* this->x */
            fmul [magnitude]
            fstp [eax + 4] /* this->y */
        }
#endif
    }

    void FromRotatedVec2(float angle, float vecX, float vecY)
    {
#ifdef TH08_MODERN_PORT
        this->x = cosf(angle) * vecX;
        this->y = sinf(angle) * vecY;
#else
        __asm
        {
            mov eax, this
            fld angle
            fsincos
            fmul [vecX]
            fstp [eax] /* this->x */
            fmul [vecY]
            fstp [eax + 4] /* this->y */
        }
#endif
    }

    // FUNCTION: th08 0x40b460 FOLDED
    operator float *()
    {
        return (float *)this;
    }

    Float3 operator+(const Float3 &other) const;
    Float3 operator-(const Float3 &other) const;
    Float3 operator*(f32 scalar) const;
    Float3 operator/(f32 scalar) const;
    Float3 operator-() const;
    Float3 *operator*=(f32 scalar);
    Float3 *operator/=(f32 scalar);

    Float3 *operator+=(const Float3 &other)
    {
        this->x += other.x;
        this->y += other.y;
        this->z += other.z;

        return this;
    }

    Float3 *operator-=(const Float3 &other)
    {
        this->x -= other.x;
        this->y -= other.y;
        this->z -= other.z;

        return this;
    }

    float x;
    float y;
    float z;
};

/* ZUN name: FVector2 */
struct Float2
{
    float x;
    float y;
};

struct ZunRect
{
    f32 left;
    f32 top;
    f32 right;
    f32 bottom;
};

f32 AddNormalizeAngle(f32 a, f32 b);
f32 VectorAngle(f32 y, f32 x);
void Rotate(Float3 *outVector, Float3 *point, f32 angle);

} // namespace th08

#ifdef TH08_MODERN_PORT
#define sincos(in, out_sine, out_cosine)                                                                               \
    {                                                                                                                  \
        out_sine = sinf(in);                                                                                           \
        out_cosine = cosf(in);                                                                                         \
    }
#else
#define sincos(in, out_sine, out_cosine)                                                                               \
    {                                                                                                                  \
        __asm { \
        __asm fld in \
        __asm fsincos \
        __asm fstp out_cosine \
        __asm fstp out_sine }                                            \
    }
#endif
