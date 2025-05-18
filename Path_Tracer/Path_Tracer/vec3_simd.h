#pragma once

#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <immintrin.h> // For SIMD intrinsics

// Modified Vec3 structure to align with SIMD requirements
struct alignas(16) Vec3_simd {
    union {
        struct { float x, y, z; };
        __m128 simd; // SSE register that can hold 4 floats (we'll use only 3)
    };

    Vec3_simd() : simd(_mm_setzero_ps()) {}
    Vec3_simd(float x, float y, float z) : simd(_mm_set_ps(0.0f, z, y, x)) {}
    Vec3_simd(__m128 v) : simd(v) {}
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
    // Remove the final shuffle
    return Vec3_simd(result);
}

inline float dot(Vec3_simd a, Vec3_simd b) {
    __m128 dp = _mm_dp_ps(a.simd, b.simd, 0x77); // Mask 0x77 (x,y,z components)
    return _mm_cvtss_f32(dp);
}

inline float fast_mag(Vec3_simd a) {
    __m128 dp = _mm_dp_ps(a.simd, a.simd, 0x77);
    return _mm_cvtss_f32(_mm_sqrt_ps(dp));
}

inline Vec3_simd norm(Vec3_simd a) {
    __m128 len_sq = _mm_dp_ps(a.simd, a.simd, 0x77); // Correct mask
    __m128 mask = _mm_cmpgt_ps(len_sq, _mm_setzero_ps());
    __m128 len = _mm_sqrt_ps(len_sq);
    __m128 inv_len = _mm_div_ps(_mm_set1_ps(1.0f), len);
    inv_len = _mm_and_ps(inv_len, mask);
    return Vec3_simd(_mm_mul_ps(a.simd, inv_len));
}

// SIMD implementation of magnitude
inline float mag(Vec3_simd a) {
    return sqrt(dot(a, a));
}

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

#include <immintrin.h>
#include <xmmintrin.h>

// SIMD saturate (single float)
inline float saturate(float a) {
    // Branchless SSE implementation
    __m128 val = _mm_set_ss(a);
    val = _mm_max_ss(val, _mm_setzero_ps());
    val = _mm_min_ss(val, _mm_set_ss(1.0f));
    return _mm_cvtss_f32(val);
}

// SIMD saturate (Vec3)
inline Vec3_simd saturate(Vec3_simd a) {
    const __m128 zero = _mm_setzero_ps();
    const __m128 one = _mm_set1_ps(1.0f);
    __m128 result = a.simd;
    result = _mm_max_ps(result, zero);  // Clamp min 0
    result = _mm_min_ps(result, one);   // Clamp max 1
    return Vec3_simd(result);
}

// Random number generation (scalar remains same)
inline float randf() {
    return rand() / (RAND_MAX + 1.0f);
}

// SIMD random vector [-1, 1]
inline Vec3_simd randf3() {
    alignas(16) float vals[4];
    for (int i = 0; i < 3; ++i) {
        vals[i] = 2.0f * randf() - 1.0f;
    }
    vals[3] = 0.0f;  // unused component
    return Vec3_simd(_mm_load_ps(vals));
}

// SIMD random point in unit sphere
inline Vec3_simd rand_in_sphere() {
    Vec3_simd value = randf3();
    while (dot(value, value) > 1.0f) {  // Using squared length check
        value = randf3();
    }
    return value;
}