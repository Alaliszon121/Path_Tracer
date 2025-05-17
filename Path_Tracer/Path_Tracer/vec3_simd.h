#pragma once

#include <math.h>
#include "customVector.h"
#include <immintrin.h> // For SIMD intrinsics

// Modified Vec3 structure to align with SIMD requirements
struct alignas(16) Vec3_simd {
    union {
        struct { float x, y, z; };
        __m128 simd; // SSE register that can hold 4 floats (we'll use only 3)
    };

    Vec3_simd() : x(0), y(0), z(0) {}
    Vec3_simd(float x, float y, float z) : x(x), y(y), z(z) {}
    Vec3_simd(__m128 v) : simd(v) {}

    Vec3_simd(const Vec3& v) : simd(_mm_set_ps(0.f, v.z, v.y, v.x)) {}
    operator Vec3() const { return Vec3{ x, y, z }; }
};


// SIMD implementation of vector subtraction
inline Vec3_simd sub(Vec3_simd a, Vec3_simd b) {
    return _mm_sub_ps(a.simd, b.simd);
}

// SIMD implementation of vector addition
inline Vec3_simd add(Vec3_simd a, Vec3_simd b) {
    return _mm_add_ps(a.simd, b.simd);
}

// SIMD implementation of component-wise multiplication
inline Vec3_simd mul(Vec3_simd a, Vec3_simd b) {
    return _mm_mul_ps(a.simd, b.simd);
}

// SIMD implementation of scalar multiplication
inline Vec3_simd mul(Vec3_simd a, float s) {
    __m128 scalar = _mm_set1_ps(s);
    return _mm_mul_ps(a.simd, scalar);
}

// SIMD implementation of cross product
inline Vec3_simd cross(Vec3_simd a, Vec3_simd b) {
    __m128 a_yzx = _mm_shuffle_ps(a.simd, a.simd, _MM_SHUFFLE(3, 0, 2, 1));
    __m128 b_yzx = _mm_shuffle_ps(b.simd, b.simd, _MM_SHUFFLE(3, 0, 2, 1));
    __m128 a_zxy = _mm_shuffle_ps(a.simd, a.simd, _MM_SHUFFLE(3, 1, 0, 2));
    __m128 b_zxy = _mm_shuffle_ps(b.simd, b.simd, _MM_SHUFFLE(3, 1, 0, 2));

    __m128 result = _mm_sub_ps(
        _mm_mul_ps(a_yzx, b_zxy),
        _mm_mul_ps(a_zxy, b_yzx)
    );

    // Shuffle back to original order (x, y, z)
    return _mm_shuffle_ps(result, result, _MM_SHUFFLE(3, 0, 2, 1));
}

// SIMD implementation of dot product
inline float dot(Vec3_simd a, Vec3_simd b) {
    __m128 mul = _mm_mul_ps(a.simd, b.simd);
    __m128 shuf = _mm_shuffle_ps(mul, mul, _MM_SHUFFLE(2, 3, 0, 1));
    __m128 sums = _mm_add_ps(mul, shuf);
    shuf = _mm_movehl_ps(shuf, sums);
    sums = _mm_add_ss(sums, shuf);
    return _mm_cvtss_f32(sums);
}

// SIMD implementation of magnitude
inline float mag(Vec3_simd a) {
    return sqrt(dot(a, a));
}

inline float fast_mag(Vec3_simd a) {
    return _mm_cvtss_f32(_mm_sqrt_ss(_mm_dp_ps(a.simd, a.simd, 0x71)));
}

// SIMD implementation of normalization
inline Vec3_simd norm(Vec3_simd a) {
    float length = mag(a);
    if (length > 0.0f) {
        __m128 inv_len = _mm_set1_ps(1.0f / length);
        return _mm_mul_ps(a.simd, inv_len);
    }
    return a;
}

// SIMD implementation of reflection
inline Vec3_simd reflect(Vec3_simd a, Vec3_simd n) {
    float d = dot(a, n);
    __m128 two_d = _mm_set1_ps(2.0f * d);
    __m128 n_two_d = _mm_mul_ps(n.simd, two_d);
    return _mm_sub_ps(a.simd, n_two_d);
}

// Set all components to the same value
inline Vec3_simd splat(float v) {
    return _mm_set1_ps(v);
}

// Zero vector
inline Vec3_simd zero() {
    return _mm_setzero_ps();
}

