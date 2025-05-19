#include "intersections.h"
#include <math.h>
#include <limits>
#include <immintrin.h>

bool Sphere::intersect(const Ray& ray, Hit& hit) const {
    // Vector from ray origin to sphere center
    Vec3_simd c = sub(pos, ray.pos);

    // Calculate dot products using SIMD
    float t1 = dot(ray.dir, c);
    float c_sq = dot(c, c);
    float radius_sq = radius * radius;

    // Early rejection tests using SIMD comparisons
    __m128 cmp1 = _mm_cmplt_ss(_mm_set_ss(t1), _mm_setzero_ps());
    __m128 cmp2 = _mm_cmpgt_ss(_mm_set_ss(c_sq - t1 * t1), _mm_set_ss(radius_sq));

    if (_mm_movemask_ps(_mm_or_ps(cmp1, cmp2)) & 1) {
        return false;
    }

    // Calculate intersection distance
    float t2 = sqrt(radius_sq - (c_sq - t1 * t1));
    hit.distance = t1 - t2;

    // Calculate hit position
    hit.pos = add(ray.pos, mul(ray.dir, hit.distance));

    // Calculate normal (with backface check)
    hit.normal = norm(sub(hit.pos, pos));
    __m128 normal_dot = _mm_dp_ps(ray.dir.simd, hit.normal.simd, 0x71);
    __m128 mask = _mm_cmpgt_ss(normal_dot, _mm_setzero_ps());
    hit.normal.simd = _mm_xor_ps(hit.normal.simd,
        _mm_and_ps(mask, _mm_set1_ps(-0.0f))); // Flip if needed

    hit.color = color;
    hit.roughness = roughness;
    return true;
}

bool Plane::intersect(const Ray& ray, Hit& hit) const {
    // Calculate denominator using SIMD dot product
    float denom = dot(normal, ray.dir);

    // Early rejection with absolute value comparison
    __m128 abs_denom = _mm_andnot_ps(_mm_set1_ps(-0.0f), _mm_set_ss(denom));
    if (_mm_comile_ss(abs_denom, _mm_set_ss(1e-6f))) {
        return false;
    }

    // Calculate distance
    float dist = -(dot(ray.pos, normal) + distance) / denom;

    // Reject if behind ray
    if (dist < 0.0f) {
        return false;
    }

    hit.distance = dist;
    hit.pos = add(ray.pos, mul(ray.dir, hit.distance));
    hit.normal = normal;
    hit.color = color;
    hit.roughness = roughness;
    return true;
}

bool intersect(const Ray& ray, const Scene& scene, Hit& hit) {
    Hit temp_hit;
    float min_distance = std::numeric_limits<float>::max();
    bool any_hit = false;

    // Process shapes in chunks of 4 for better SIMD utilization (optional)
    size_t i = 0;
    const size_t count = scene.shapes.size();

    for (; i < count; ++i) {
        if (scene.shapes[i]->intersect(ray, temp_hit)) {
            // Use SIMD comparison for distance check
            __m128 cmp = _mm_cmplt_ss(_mm_set_ss(temp_hit.distance), _mm_set_ss(min_distance));
                if (_mm_movemask_ps(cmp) & 1) {
                    hit = temp_hit;
                        min_distance = temp_hit.distance;
                        any_hit = true;
                }
        }
    }

    return any_hit;
}