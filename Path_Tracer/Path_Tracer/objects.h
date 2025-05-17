#pragma once
#include "vec3_simd.h"  // Use SIMD-optimized Vec3

// Sphere with SIMD-aligned members
struct alignas(16) Sphere {
    Vec3_simd pos;      // Center (16-byte aligned)
    float radius;       // Sphere radius
    Vec3_simd color;    // Color (16-byte aligned)
    float roughness;    // 0.0 (smooth) to 0.9 (rough)
    float _pad[2];      // Padding to maintain alignment (optional)
};

// Plane with SIMD-aligned members
struct alignas(16) Plane {
    Vec3_simd normal;   // Surface normal (16-byte aligned)
    float distance;     // Distance from origin
    Vec3_simd color;    // Color (16-byte aligned)
    float roughness;    // 0.0 (smooth) to 0.9 (rough)
};

// Ray with SIMD-aligned members
struct alignas(16) Ray {
    Vec3_simd pos;      // Origin (16-byte aligned)
    Vec3_simd dir;      // Direction (16-byte aligned)
    float _pad[2];      // Padding for potential AVX (optional)
};

// Hit record with SIMD-aligned members
struct alignas(16) Hit {
    Vec3_simd pos;      // Intersection point (16-byte aligned)
    float distance;     // Ray distance to hit
    Vec3_simd normal;   // Surface normal (16-byte aligned)
    Vec3_simd color;    // Color (16-byte aligned)
    float roughness;    // 0.0 (smooth) to 0.9 (rough)
};

// Scene containing SIMD-aligned objects
struct alignas(16) Scene {
    Sphere s1, s2, s3;  // 3 SIMD-optimized spheres
    Plane p1;           // 1 SIMD-optimized plane
};
