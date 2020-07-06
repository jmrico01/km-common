#pragma once

#include <math.h>

#include "km_defines.h"

#define PI_F 3.14159265f
#define E_F  2.71828182f

inline int RoundToPowerOfTwo(int n, int powerOfTwo) {
    DEBUG_ASSERTF(powerOfTwo && ((powerOfTwo & (powerOfTwo - 1)) == 0), "%d should be power of 2\n",
                  powerOfTwo);
    return (n + powerOfTwo - 1) & -powerOfTwo;
}

inline uint32 RoundUpToPowerOfTwo(int n)
{
    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n++;

    return n;
}

inline int AbsInt(int n) {
    return n >= 0 ? n : -n;
}
inline float32 AbsFloat32(float32 f) {
    return f < 0.0f ? -f : f;
}

inline int RandInt(int max)
{
	DEBUG_ASSERT(max > 0);
	return rand() % max;
}
inline int RandInt(int min, int max)
{
	DEBUG_ASSERT(max > min);
	return rand() % (max - min) + min;
}

inline int MinInt(int a, int b) {
    return a < b ? a : b;
}
inline int MaxInt(int a, int b) {
    return a > b ? a : b;
}
inline int ClampInt(int a, int min, int max) {
    return MinInt(MaxInt(a, min), max);
}

inline int RandUInt32(uint32 max)
{
	DEBUG_ASSERT(max > 0);
	return rand() % max;
}
inline int RandUInt32(uint32 min, uint32 max)
{
	DEBUG_ASSERT(max > min);
	return rand() % (max - min) + min;
}

inline uint32 MinUInt32(uint32 a, uint32 b) {
    return a < b ? a : b;
}
inline uint32 MaxUInt32(uint32 a, uint32 b) {
    return a > b ? a : b;
}
inline uint32 ClampUInt32(uint32 a, uint32 min, uint32 max) {
    return MinUInt32(MaxUInt32(a, min), max);
}

inline uint64 MinUInt64(uint64 a, uint64 b) {
    return a < b ? a : b;
}
inline uint64 MaxUInt64(uint64 a, uint64 b) {
    return a > b ? a : b;
}
inline uint64 ClampUInt64(uint64 a, uint64 min, uint64 max) {
    return MinUInt64(MaxUInt64(a, min), max);
}

int ToIntOrTruncate(uint64 n)
{
    if (n > INT_MAX) {
        return INT_MAX;
    }
    else {
        return (int)n;
    }
}

uint32 SafeTruncateUInt64(uint64 value)
{
    DEBUG_ASSERT(value <= 0xFFFFFFFF);
    uint32 result = (uint32)value;
    return result;
}


inline float32 RandFloat32()
{
	return (float32)rand() / RAND_MAX;
}
inline float32 RandFloat32(float32 min, float32 max)
{
	DEBUG_ASSERT(max > min);
	return RandFloat32() * (max - min) + min;
}

inline float32 MinFloat32(float32 a, float32 b) {
    return a < b ? a : b;
}
inline float32 MaxFloat32(float32 a, float32 b) {
    return a > b ? a : b;
}
inline float32 ClampFloat32(float32 a, float32 min, float32 max) {
    return MinFloat32(MaxFloat32(a, min), max);
}

float32 ModFloat32(float32 n, float32 mod)
{
    return (float32)fmod(n, mod);
}

// TODO quick and dirty round implementation
inline int RoundFloat32Fast(float32 a) {
    if (a < 0.0) {
        return (int)(a - 0.5);
    }
    else {
        return (int)(a + 0.5);
    }
}

float32 Lerp(float32 a, float32 b, float32 t)
{
    return a + (b - a) * t;
}
int Lerp(int a, int b, float32 t)
{
    return (int)((float32)a + (float32)(b - a) * t);
}

bool IsPrime(uint32 n)
{
    for (uint32 i = 2; i < n / 2; i++) {
        if (n % i == 0) {
            return false;
        }
    }

    return true;
}

uint32 NextPrime(uint32 n)
{
    while (!IsPrime(n)) {
        n++;
    }

    return n;
}

// ========== MATH TYPES ==========

union Vec2
{
    const static Vec2 zero;
    const static Vec2 one;
    const static Vec2 unitX;
    const static Vec2 unitY;

    struct
    {
        float32 x, y;
    };
    float32 e[2];
};

const Vec2 Vec2::zero  = { 0.0f, 0.0f };
const Vec2 Vec2::one   = { 1.0f, 1.0f };
const Vec2 Vec2::unitX = { 1.0f, 0.0f };
const Vec2 Vec2::unitY = { 0.0f, 1.0f };

union Vec2Int
{
    const static Vec2Int zero;
    const static Vec2Int unitX;
    const static Vec2Int unitY;

    struct
    {
        int x, y;
    };
    int e[2];
};

const Vec2Int Vec2Int::zero  = { 0, 0 };
const Vec2Int Vec2Int::unitX = { 1, 0 };
const Vec2Int Vec2Int::unitY = { 0, 1 };

union Vec3
{
    const static Vec3 zero;
    const static Vec3 one;
    const static Vec3 unitX;
    const static Vec3 unitY;
    const static Vec3 unitZ;

    struct
    {
        float32 x, y, z;
    };
    struct
    {
        float32 r, g, b;
    };
    float32 e[3];
};

const Vec3 Vec3::zero  = { 0.0f, 0.0f, 0.0f };
const Vec3 Vec3::one   = { 1.0f, 1.0f, 1.0f };
const Vec3 Vec3::unitX = { 1.0f, 0.0f, 0.0f };
const Vec3 Vec3::unitY = { 0.0f, 1.0f, 0.0f };
const Vec3 Vec3::unitZ = { 0.0f, 0.0f, 1.0f };

union Vec4
{
    const static Vec4 zero;
    const static Vec4 one;
    const static Vec4 unitX;
    const static Vec4 unitY;
    const static Vec4 unitZ;
    const static Vec4 unitW;
    const static Vec4 white;
    const static Vec4 black;

    struct
    {
        float32 x, y, z, w;
    };
    struct
    {
        float32 r, g, b, a;
    };
    float32 e[4];
};

const Vec4 Vec4::zero  = { 0.0f, 0.0f, 0.0f, 0.0f };
const Vec4 Vec4::one   = { 1.0f, 1.0f, 1.0f, 1.0f };
const Vec4 Vec4::unitX = { 1.0f, 0.0f, 0.0f, 0.0f };
const Vec4 Vec4::unitY = { 0.0f, 1.0f, 0.0f, 0.0f };
const Vec4 Vec4::unitZ = { 0.0f, 0.0f, 1.0f, 0.0f };
const Vec4 Vec4::unitW = { 0.0f, 0.0f, 0.0f, 1.0f };
const Vec4 Vec4::white = { 1.0f, 1.0f, 1.0f, 1.0f };
const Vec4 Vec4::black = { 0.0f, 0.0f, 0.0f, 1.0f };

struct Rect
{
    Vec2 min, max;
};

struct RectInt
{
    Vec2Int min, max;
};

// Column-major 4x4 matrix (columns stored contiguously)
// ORDER IS OPPOSITE OF NORMAL MATH
/*
| e[0][0]  e[1][0]  e[2][0]  e[3][0] |
| e[0][1]  e[1][1]  e[2][1]  e[3][1] |
| e[0][2]  e[1][2]  e[2][2]  e[3][2] |
| e[0][3]  e[1][3]  e[2][3]  e[3][3] |
*/
struct Mat4
{
    const static Mat4 zero;
    const static Mat4 one;

    float32 e[4][4];
};

// Should always be unit quaternions
struct Quat
{
    const static Quat one;

    // You should NOT be changing
    // these values manually
    float32 x, y, z, w;
};

// ========== OPERATORS & FUNCTIONS ==========

// -------------------- Vec2 --------------------
inline Vec2Int ToVec2Int(Vec2 v)
{
    return Vec2Int { (int)v.x, (int)v.y };
}

inline Vec3 ToVec3(Vec2 v, float32 z)
{
    return Vec3 { v.x, v.y, z };
}

inline Vec4 ToVec4(Vec2 v, float32 z, float32 w)
{
    return Vec4 { v.x, v.y, z, w };
}

inline Vec2 operator-(Vec2 v)
{
    Vec2 result;
    result.x = -v.x;
    result.y = -v.y;
    return result;
}

inline Vec2 operator+(Vec2 v1, Vec2 v2)
{
    Vec2 result;
    result.x = v1.x + v2.x;
    result.y = v1.y + v2.y;
    return result;
}
inline Vec2& operator+=(Vec2& v1, Vec2 v2)
{
    v1 = v1 + v2;
    return v1;
}

inline Vec2 operator-(Vec2 v1, Vec2 v2)
{
    Vec2 result;
    result.x = v1.x - v2.x;
    result.y = v1.y - v2.y;
    return result;
}
inline Vec2& operator-=(Vec2& v1, Vec2 v2)
{
    v1 = v1 - v2;
    return v1;
}

inline Vec2 operator*(float32 s, Vec2 v)
{
    Vec2 result;
    result.x = s * v.x;
    result.y = s * v.y;
    return result;
}
inline Vec2 operator*(Vec2 v, float32 s)
{
    return s * v;
}
inline Vec2& operator*=(Vec2& v, float32 s)
{
    v = s * v;
    return v;
}

inline Vec2 operator/(Vec2 v, float32 s)
{
    Vec2 result;
    result.x = v.x / s;
    result.y = v.y / s;
    return result;
}
inline Vec2& operator/=(Vec2& v, float32 s)
{
    v = v / s;
    return v;
}

inline bool operator==(const Vec2& v1, const Vec2& v2)
{
    return v1.x == v2.x && v1.y == v2.y;
}

Vec2 Lerp(Vec2 v1, Vec2 v2, float t)
{
    Vec2 result = {
        Lerp(v1.x, v2.x, t),
        Lerp(v1.y, v2.y, t)
    };
    return result;
}

inline float32 Dot(Vec2 v1, Vec2 v2)
{
    return v1.x * v2.x + v1.y * v2.y;
}

inline float32 MagSq(Vec2 v)
{
    return v.x*v.x + v.y*v.y;
}
inline float32 Mag(Vec2 v)
{
    return sqrtf(v.x*v.x + v.y*v.y);
}
inline Vec2 Normalize(Vec2 v)
{
    return v / Mag(v);
}

float32 AngleBetween(Vec2 v1, Vec2 v2)
{
    return (float32)(atan2(v2.y, v2.x) - atan2(v1.y, v1.x));
}

// ------------------ Vec2Int -------------------
inline Vec2 ToVec2(Vec2Int v)
{
    return Vec2 { (float32)v.x, (float32)v.y };
}

inline Vec2Int operator-(Vec2Int v)
{
    Vec2Int result;
    result.x = -v.x;
    result.y = -v.y;
    return result;
}

inline Vec2Int operator+(Vec2Int v1, Vec2Int v2)
{
    Vec2Int result;
    result.x = v1.x + v2.x;
    result.y = v1.y + v2.y;
    return result;
}
inline Vec2Int& operator+=(Vec2Int& v1, Vec2Int v2)
{
    v1 = v1 + v2;
    return v1;
}

inline Vec2Int operator-(Vec2Int v1, Vec2Int v2)
{
    Vec2Int result;
    result.x = v1.x - v2.x;
    result.y = v1.y - v2.y;
    return result;
}
inline Vec2Int& operator-=(Vec2Int& v1, Vec2Int v2)
{
    v1 = v1 - v2;
    return v1;
}

inline Vec2Int operator*(int s, Vec2Int v)
{
    Vec2Int result;
    result.x = s * v.x;
    result.y = s * v.y;
    return result;
}
inline Vec2Int operator*(Vec2Int v, int s)
{
    return s * v;
}
inline Vec2Int& operator*=(Vec2Int& v, int s)
{
    v = s * v;
    return v;
}

inline Vec2Int operator/(Vec2Int v, int s)
{
    Vec2Int result;
    result.x = v.x / s;
    result.y = v.y / s;
    return result;
}
inline Vec2Int& operator/=(Vec2Int& v, int s)
{
    v = v / s;
    return v;
}

/*inline Vec2Int operator*(float32 s, Vec2Int v)
{
    Vec2Int result;
    result.x = (int)(s * (float32)v.x);
    result.y = (int)(s * (float32)v.y);
    return result;
}
inline Vec2Int operator*(Vec2Int v, float32 s)
{
    return s * v;
}
inline Vec2Int& operator*=(Vec2Int& v, float32 s)
{
    v = s * v;
    return v;
}*/

inline Vec2Int MultiplyVec2IntFloat32(Vec2Int v, float32 f)
{
    return Vec2Int { (int)((float32)v.x * f), (int)((float32)v.y * f) };
}

inline bool operator==(const Vec2Int& v1, const Vec2Int& v2)
{
    return v1.x == v2.x && v1.y == v2.y;
}

inline bool operator!=(const Vec2Int& v1, const Vec2Int& v2)
{
    return v1.x != v2.x || v1.y != v2.y;
}

Vec2Int Lerp(Vec2Int v1, Vec2Int v2, float32 t)
{
    Vec2Int result = {
        Lerp(v1.x, v2.x, t),
        Lerp(v1.y, v2.y, t)
    };
    return result;
}

inline int MagSq(Vec2Int v)
{
    return v.x*v.x + v.y*v.y;
}
inline int Mag(Vec2Int v)
{
    return (int)sqrtf((float32)v.x*v.x + v.y*v.y);
}

// -------------------- Vec3 --------------------
inline Vec2 ToVec2(Vec3 v)
{
    Vec2 result;
    result.x = v.x;
    result.y = v.y;
    return result;
}
inline Vec4 ToVec4(Vec3 v, float32 w)
{
    Vec4 result;
    result.x = v.x;
    result.y = v.y;
    result.z = v.z;
    result.w = w;
    return result;
}

inline Vec3 operator-(Vec3 v)
{
    Vec3 result;
    result.x = -v.x;
    result.y = -v.y;
    result.z = -v.z;
    return result;
}

inline Vec3 operator+(Vec3 v1, Vec3 v2)
{
    Vec3 result;
    result.x = v1.x + v2.x;
    result.y = v1.y + v2.y;
    result.z = v1.z + v2.z;
    return result;
}
inline Vec3& operator+=(Vec3& v1, Vec3 v2)
{
    v1 = v1 + v2;
    return v1;
}

inline Vec3 operator-(Vec3 v1, Vec3 v2)
{
    Vec3 result;
    result.x = v1.x - v2.x;
    result.y = v1.y - v2.y;
    result.z = v1.z - v2.z;
    return result;
}
inline Vec3& operator-=(Vec3& v1, Vec3 v2)
{
    v1 = v1 - v2;
    return v1;
}

inline Vec3 operator*(float32 s, Vec3 v)
{
    Vec3 result;
    result.x = s * v.x;
    result.y = s * v.y;
    result.z = s * v.z;
    return result;
}
inline Vec3 operator*(Vec3 v, float32 s)
{
    return s * v;
}
inline Vec3& operator*=(Vec3& v, float32 s)
{
    v = s * v;
    return v;
}

inline Vec3 operator/(Vec3 v, float32 s)
{
    Vec3 result;
    result.x = v.x / s;
    result.y = v.y / s;
    result.z = v.z / s;
    return result;
}
inline Vec3& operator/=(Vec3& v, float32 s)
{
    v = v / s;
    return v;
}

inline bool operator==(const Vec3& v1, const Vec3& v2)
{
    return v1.x == v2.x && v1.y == v2.y && v1.z == v2.z;
}

inline Vec3 Lerp(Vec3 v1, Vec3 v2, float t)
{
    Vec3 result = {
        Lerp(v1.x, v2.x, t),
        Lerp(v1.y, v2.y, t),
        Lerp(v1.z, v2.z, t)
    };
    return result;
}

inline float32 Dot(Vec3 v1, Vec3 v2)
{
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

inline Vec3 Cross(Vec3 v1, Vec3 v2)
{
    return Vec3 {
        v1.y * v2.z - v1.z * v2.y,
        v1.z * v2.x - v1.x * v2.z,
        v1.x * v2.y - v1.y * v2.x
    };
}

inline float32 MagSq(Vec3 v)
{
    return v.x*v.x + v.y*v.y + v.z*v.z;
}
inline float32 Mag(Vec3 v)
{
    return sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);
}
inline Vec3 Normalize(Vec3 v)
{
    return v / Mag(v);
}

inline Vec3 GetPerpendicular(Vec3 v)
{
    return v.z < v.x ? Vec3 { v.y, -v.x, 0.0f } : Vec3 { 0.0f, -v.z, v.y };
}

// -------------------- Vec4 --------------------
inline Vec4 operator-(Vec4 v)
{
    Vec4 result;
    result.x = -v.x;
    result.y = -v.y;
    result.z = -v.z;
    result.w = -v.w;
    return result;
}

inline Vec4 operator+(Vec4 v1, Vec4 v2)
{
    Vec4 result;
    result.x = v1.x + v2.x;
    result.y = v1.y + v2.y;
    result.z = v1.z + v2.z;
    result.w = v1.w + v2.w;
    return result;
}
inline Vec4& operator+=(Vec4& v1, Vec4 v2)
{
    v1 = v1 + v2;
    return v1;
}

inline Vec4 operator-(Vec4 v1, Vec4 v2)
{
    Vec4 result;
    result.x = v1.x - v2.x;
    result.y = v1.y - v2.y;
    result.z = v1.z - v2.z;
    result.w = v1.w - v2.w;
    return result;
}
inline Vec4& operator-=(Vec4& v1, Vec4 v2)
{
    v1 = v1 - v2;
    return v1;
}

inline Vec4 operator*(float32 s, Vec4 v)
{
    Vec4 result;
    result.x = s * v.x;
    result.y = s * v.y;
    result.z = s * v.z;
    result.w = s * v.w;
    return result;
}
inline Vec4 operator*(Vec4 v, float32 s)
{
    return s * v;
}
inline Vec4& operator*=(Vec4& v, float32 s)
{
    v = s * v;
    return v;
}

inline Vec4 operator/(Vec4 v, float32 s)
{
    Vec4 result;
    result.x = v.x / s;
    result.y = v.y / s;
    result.z = v.z / s;
    result.w = v.w / s;
    return result;
}
inline Vec4& operator/=(Vec4& v, float32 s)
{
    v = v / s;
    return v;
}

inline bool operator==(const Vec4& v1, const Vec4& v2)
{
    return v1.x == v2.x && v1.y == v2.y && v1.z == v2.z && v1.w == v2.w;
}

inline Vec4 Lerp(Vec4 v1, Vec4 v2, float t)
{
    Vec4 result = {
        Lerp(v1.x, v2.x, t),
        Lerp(v1.y, v2.y, t),
        Lerp(v1.z, v2.z, t),
        Lerp(v1.w, v2.w, t)
    };
    return result;
}

// -------------------- Rect --------------------
bool IsInside(Vec2 v, Rect rect)
{
    return rect.min.x <= v.x && v.x < rect.max.x && rect.min.y <= v.y && v.y < rect.max.y;
}

// ------------------ RectInt -------------------
bool IsInside(Vec2Int v, RectInt rect)
{
    return rect.min.x <= v.x && v.x < rect.max.x && rect.min.y <= v.y && v.y < rect.max.y;
}

// -------------------- Mat4 --------------------
// TODO these functions might be better off not inlined
// though they are used very infrequently
const Mat4 Mat4::one =
{
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1
};
const Mat4 Mat4::zero =
{
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0
};

inline Mat4 operator+(Mat4 m1, Mat4 m2)
{
    Mat4 result;
    for (int col = 0; col < 4; col++) {
        for (int row = 0; row < 4; row++) {
            result.e[col][row] = m1.e[col][row] + m2.e[col][row];
        }
    }

    return result;
}
inline Mat4& operator+=(Mat4& m1, Mat4 m2)
{
    m1 = m1 + m2;
    return m1;
}

inline Mat4 operator-(Mat4 m1, Mat4 m2)
{
    Mat4 result;
    for (int col = 0; col < 4; col++) {
        for (int row = 0; row < 4; row++) {
            result.e[col][row] = m1.e[col][row] - m2.e[col][row];
        }
    }

    return result;
}
inline Mat4& operator-=(Mat4& m1, Mat4 m2)
{
    m1 = m1 - m2;
    return m1;
}

inline Mat4 operator*(Mat4 m1, Mat4 m2)
{
    Mat4 result = Mat4::zero;

    // I really thought hard about this
    // Make it as cache-efficient as possible
    // Probably doesn't matter at all...
    for (int colM2 = 0; colM2 < 4; colM2++) {
        for (int colM1 = 0; colM1 < 4; colM1++) {
            for (int rowM1 = 0; rowM1 < 4; rowM1++) {
                result.e[colM2][rowM1] +=
                    m2.e[colM2][colM1] * m1.e[colM1][rowM1];
            }
        }
    }

    return result;
}

inline Vec4 operator*(Mat4 m, Vec4 v)
{
    Vec4 result = Vec4::zero;

    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 4; col++) {
            result.e[row] += m.e[col][row] * v.e[col];
        }
    }

    return result;
}

Mat4 Translate(Vec3 v)
{
    Mat4 result = Mat4::one;

    result.e[3][0] = v.x;
    result.e[3][1] = v.y;
    result.e[3][2] = v.z;

    return result;
}

Mat4 Scale(float32 s)
{
    Mat4 result = {};

    result.e[0][0] = s;
    result.e[1][1] = s;
    result.e[2][2] = s;
    result.e[3][3] = 1.0f;

    return result;
}

Mat4 Scale(Vec3 v)
{
    Mat4 result = {};

    result.e[0][0] = v.x;
    result.e[1][1] = v.y;
    result.e[2][2] = v.z;
    result.e[3][3] = 1.0f;

    return result;
}

Mat4 Rotate(Vec3 r)
{
    Mat4 rx = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, cosf(r.x), -sinf(r.x), 0.0f,
        0.0f, sinf(r.x), cosf(r.x), 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    Mat4 ry = {
        cosf(r.y), 0.0f, sinf(r.y), 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        -sinf(r.y), 0.0f, cosf(r.y), 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    Mat4 rz = {
        cosf(r.z), -sinf(r.z), 0.0f, 0.0f,
        sinf(r.z), cosf(r.z), 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };

    // Order: <- yaw <- pitch <- roll <-
    return rz * ry * rx;
}

Mat4 Perspective(float32 fovRadians, float32 aspect, float32 nearZ, float32 farZ)
{
    const float32 yScale = 1.0f / tanf(fovRadians / 2.0f);
    const float32 xScale = yScale / aspect;
    const float32 farMinusNear = farZ - nearZ;
    return {
        xScale, 0.0f, 0.0f, 0.0f,
        0.0f, yScale, 0.0f, 0.0f,
        0.0f, 0.0f, farZ / farMinusNear, 1.0f,
        0.0f, 0.0f, -farZ*nearZ / farMinusNear, 0.0f
    };
}

// -------------------- Quat --------------------
const Quat Quat::one = {
    0.0f, 0.0f, 0.0f, 1.0f
};

// Compounds two rotations q1 and q2 (like matrices: q2 first, then q1)
inline Quat operator*(Quat q1, Quat q2)
{
    Quat result;
    result.x = q1.w*q2.x + q1.x*q2.w + q1.y*q2.z - q1.z*q2.y;
    result.y = q1.w*q2.y + q1.y*q2.w + q1.z*q2.x - q1.x*q2.z;
    result.z = q1.w*q2.z + q1.z*q2.w + q1.x*q2.y - q1.y*q2.x;
    result.w = q1.w*q2.w - q1.x*q2.x - q1.y*q2.y - q1.z*q2.z;
    return result;
}

inline float32 MagSq(Quat q)
{
    return q.x*q.x + q.y*q.y + q.z*q.z + q.w*q.w;
}
inline float32 Mag(Quat q)
{
    return sqrtf(MagSq(q));
}
inline Quat Normalize(Quat q)
{
    Quat result = q;
    float32 mag = Mag(q);
    result.x /= mag;
    result.y /= mag;
    result.z /= mag;
    result.w /= mag;
    return result;
}

inline Quat Lerp(Quat q1, Quat q2, float32 t)
{
    Quat q;
    q.x = Lerp(q1.x, q2.x, t);
    q.y = Lerp(q1.y, q2.y, t);
    q.z = Lerp(q1.z, q2.z, t);
    q.w = Lerp(q1.w, q2.w, t);
    return q;
}

// Returns a new quaternion qInv such that q * qInv = Quat::one
inline Quat Inverse(Quat q)
{
    Quat inv;
    inv.x = -q.x;
    inv.y = -q.y;
    inv.z = -q.z;
    inv.w = q.w;
    return inv;
}

// Rotates vector v by quaternion q
inline Vec3 operator*(Quat q, Vec3 v)
{
    // Treat v as a quaternion with w = 0
    Quat vQuat = { v.x, v.y, v.z, 0.0f };
    // TODO Quat multiply with baked in w=0 would be faster, obviously
    /*qv.x = q.w*v.x + q.y*v.z - q.z*v.y;
    qv.y = q.w*v.y + q.z*v.x - q.x*v.z;
    qv.z = q.w*v.z + q.x*v.y - q.y*v.x;
    qv.w = -q.x*v.x - q.y*v.y - q.z*v.z;*/
    Quat qv = q * vQuat;

    Quat qInv = Inverse(q);
    Quat qvqInv = qv * qInv;

    Vec3 result = { qvqInv.x, qvqInv.y, qvqInv.z };
    return result;
}

// Axis should be a unit vector
Quat QuatFromAngleUnitAxis(float32 angle, Vec3 axis)
{
    const float32 cosHalfAngle = cosf(angle / 2.0f);
    const float32 sinHalfAngle = sinf(angle / 2.0f);

    Quat quat;
    quat.x = axis.x * sinHalfAngle;
    quat.y = axis.y * sinHalfAngle;
    quat.z = axis.z * sinHalfAngle;
    quat.w = cosHalfAngle;
    return quat;
}

// TODO is this working?
Quat QuatFromEulerAngles(Vec3 euler)
{
    Quat quat = QuatFromAngleUnitAxis(euler.x, Vec3 { 1.0f, 0.0f, 0.0f });
    quat = QuatFromAngleUnitAxis(euler.y, Vec3 { 0.0f, 1.0f, 0.0f }) * quat;
    quat = QuatFromAngleUnitAxis(euler.z, Vec3 { 0.0f, 0.0f, 1.0f }) * quat;
    return quat;
}

Quat QuatRotBetweenVectors(Vec3 v1, Vec3 v2)
{
    float32 dot = Dot(v1, v2);
    if (dot > 0.99999f) {
        return Quat::one;
    }
    else if (dot < -0.99999f) {
        // TODO 180 degree rotation about any perpendicular axis
        // hardcoded PI_F could be cheaper to calculate some other way
        const Vec3 axis = Normalize(GetPerpendicular(v1));
        return QuatFromAngleUnitAxis(PI_F, axis);
    }

    const Vec3 axis = Cross(v1, v2);
    const float32 angle = (float32)sqrt(MagSq(v1) * MagSq(v2)) + dot;
    return QuatFromAngleUnitAxis(angle, Normalize(axis));
}

// q, as always, must be a unit quaternion
Mat4 UnitQuatToMat4(Quat q)
{
    Mat4 result;
    result.e[0][0] = 1.0f - 2.0f * (q.y*q.y + q.z*q.z);
    result.e[0][1] = 2.0f * (q.x*q.y + q.w*q.z);
    result.e[0][2] = 2.0f * (q.x*q.z - q.w*q.y);
    result.e[0][3] = 0.0f;

    result.e[1][0] = 2.0f * (q.x*q.y - q.w*q.z);
    result.e[1][1] = 1.0f - 2.0f * (q.x*q.x + q.z*q.z);
    result.e[1][2] = 2.0f * (q.y*q.z + q.w*q.x);
    result.e[1][3] = 0.0f;

    result.e[2][0] = 2.0f * (q.x*q.z + q.w*q.y);
    result.e[2][1] = 2.0f * (q.y*q.z - q.w*q.x);
    result.e[2][2] = 1.0f - 2.0f * (q.x*q.x + q.y*q.y);
    result.e[2][3] = 0.0f;

    result.e[3][0] = 0.0f;
    result.e[3][1] = 0.0f;
    result.e[3][2] = 0.0f;
    result.e[3][3] = 1.0f;
    return result;
}

// Misc, higher level geometry functions

float32 SmoothStep(float32 edge0, float32 edge1, float32 x)
{
    x = ClampFloat32((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    return x * x * (3.0f - 2.0f * x);
}

bool RayPlaneIntersection(Vec3 rayOrigin, Vec3 rayDir, Vec3 planeOrigin, Vec3 planeNormal, float32* t)
{
    float32 dotDirNormal = Dot(rayDir, planeNormal);
    if (dotDirNormal == 0.0f) {
        // Ray direction is perpendicular to plane normal. There is no intersection.
        return false;
    }

    *t = Dot(planeOrigin - rayOrigin, planeNormal) / dotDirNormal;
    return true;
}

bool RayAxisAlignedBoxIntersection(Vec3 rayOrigin, Vec3 rayDirInv, Vec3 boxMin, Vec3 boxMax, float32* t)
{
    float32 tMin = -INFINITY;
    float32 tMax = INFINITY;

    const float32 tX1 = (boxMin.x - rayOrigin.x) * rayDirInv.x;
    const float32 tX2 = (boxMax.x - rayOrigin.x) * rayDirInv.x;
    tMin = MaxFloat32(tMin, MinFloat32(tX1, tX2));
    tMax = MinFloat32(tMax, MaxFloat32(tX1, tX2));

    const float32 tY1 = (boxMin.y - rayOrigin.y) * rayDirInv.y;
    const float32 tY2 = (boxMax.y - rayOrigin.y) * rayDirInv.y;
    tMin = MaxFloat32(tMin, MinFloat32(tY1, tY2));
    tMax = MinFloat32(tMax, MaxFloat32(tY1, tY2));

    const float32 tZ1 = (boxMin.z - rayOrigin.z) * rayDirInv.z;
    const float32 tZ2 = (boxMax.z - rayOrigin.z) * rayDirInv.z;
    tMin = MaxFloat32(tMin, MinFloat32(tZ1, tZ2));
    tMax = MinFloat32(tMax, MaxFloat32(tZ1, tZ2));

    // *point = 
    *t = tMin;

    return tMax >= tMin;
}

float32 TriangleArea(Vec3 v1, Vec3 v2, Vec3 v3)
{
    return Mag(Cross(v3 - v1, v2 - v1)) / 2.0f;
}

// Returns triangle normal out of the front face, where v1 -> v2 -> v3 are ordered clockwise
Vec3 CalculateTriangleUnitNormal(Vec3 v1, Vec3 v2, Vec3 v3)
{
    return Normalize(Cross(v3 - v1, v2 - v1));
}

Vec3 BarycentricCoordinates(Vec2 p, Vec2 a, Vec2 b, Vec2 c)
{
    const Vec2 v0 = b - a;
    const Vec2 v1 = c - a;
    const Vec2 v2 = p - a;
    const float32 d00 = Dot(v0, v0);
    const float32 d01 = Dot(v0, v1);
    const float32 d11 = Dot(v1, v1);
    const float32 d20 = Dot(v2, v0);
    const float32 d21 = Dot(v2, v1);
    const float32 denom = d00 * d11 - d01 * d01;

    Vec3 result;
    result.y = (d11 * d20 - d01 * d21) / denom;
    result.z = (d00 * d21 - d01 * d20) / denom;
    result.x = 1.0f - result.y - result.z;
    return result;
}

bool BarycentricCoordinates(Vec3 rayOrigin, Vec3 rayDir, Vec3 a, Vec3 b, Vec3 c, Vec3* bCoords)
{
    const float32 EPSILON = 0.000001f;

    const Vec3 ab = b - a;
    const Vec3 ac = c - a;
    const Vec3 h = Cross(rayDir, ac);
    const float32 x = Dot(ab, h);
    if (x > -EPSILON && x < EPSILON) {
        // Ray is parallel to triangle
        return false;
    }

    const float32 f = 1.0f / x;
    const Vec3 s = rayOrigin - a;
    const float32 u = f * Dot(s, h);
    if (u < 0.0f || u > 1.0f) {
        return false;
    }

    const Vec3 q = Cross(s, ab);
    const float32 v = f * Dot(rayDir, q);
    const float32 sumUV = u + v;
    if (v < 0.0f || sumUV > 1.0f) {
        return false;
    }

    bCoords->x = u;
    bCoords->y = v;
    bCoords->z = 1.0f - sumUV;
    return true;
}

bool RayTriangleIntersection(Vec3 rayOrigin, Vec3 rayDir, Vec3 a, Vec3 b, Vec3 c, float32* t)
{
    const float32 EPSILON = 0.000001f;

    const Vec3 ab = b - a;
    const Vec3 ac = c - a;
    const Vec3 h = Cross(rayDir, ac);
    const float32 x = Dot(ab, h);
    if (x > -EPSILON && x < EPSILON) {
        // Ray is parallel to triangle
        return false;
    }

    const float32 f = 1.0f / x;
    const Vec3 s = rayOrigin - a;
    const float32 u = f * Dot(s, h);
    if (u < 0.0f || u > 1.0f) {
        return false;
    }

    const Vec3 q = Cross(s, ab);
    const float32 v = f * Dot(rayDir, q);
    if (v < 0.0f || u + v > 1.0f) {
        return false;
    }

    *t = f * Dot(ac, q);
    // NOTE if a is 0, intersection is a line (I think)
    return true;
}
