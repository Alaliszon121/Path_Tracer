#include "render.h"
#include <immintrin.h>

// Adjust ray origin to avoid self-intersection
inline void adjust(Ray& r) {
    const __m128 offset = _mm_set1_ps(0.0001f);
    r.pos.simd = _mm_add_ps(r.pos.simd, _mm_mul_ps(r.dir.simd, offset));
}

// Perturb ray direction with random sphere sampling
inline void perturb(Ray& r, float degree) {
    Vec3_simd v = mul(rand_in_sphere(), degree);
    r.dir = norm(add(r.dir, v));
}

// Recursive path tracing function with SIMD optimizations
Vec3_simd path_tracing(Ray ray, Scene& scene, uint32_t bounces) {
    Hit hit = {};
    if (bounces == 0 || !intersect(ray, scene, hit)) {
        // Background gradient using SIMD
        const __m128 white = _mm_set_ps(0.0f, 1.0f, 1.0f, 1.0f);
        const __m128 blue = _mm_set_ps(0.0f, 1.0f, 0.7f, 0.5f);

        // Calculate t factor
        __m128 dir_y = _mm_shuffle_ps(ray.dir.simd, ray.dir.simd, _MM_SHUFFLE(1, 1, 1, 1));
        __m128 t = _mm_mul_ps(_mm_add_ps(dir_y, _mm_set1_ps(1.0f)), _mm_set1_ps(0.5f));
        t = saturate__m128(t);

        // Lerp between white and blue
        __m128 one_minus_t = _mm_sub_ps(_mm_set1_ps(1.0f), t);
        __m128 result = _mm_add_ps(
            _mm_mul_ps(white, t),
            _mm_mul_ps(blue, one_minus_t)
        );
        return Vec3_simd(result);
    }

    bounces--;

    // Calculate reflection with SIMD
    Vec3_simd reflected = reflect(ray.dir, hit.normal);

    // Prepare bounced ray
    Ray ray_bounce;
    ray_bounce.pos = hit.pos;
    ray_bounce.dir = reflected;
    adjust(ray_bounce);
    perturb(ray_bounce, hit.roughness);

    // Recursive call with SIMD color multiplication
    Vec3_simd bounce_color = path_tracing(ray_bounce, scene, bounces);
    return mul(hit.color, bounce_color);
}

// Render function with SIMD optimizations
Vec3_simd render(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t bounces, uint32_t samples, Scene& scene) {
    // Camera setup with SIMD
    Vec3_simd camera_pos = { 0.0f, 0.0f, -3.0f };
    float camera_near = 0.5f;
    float aspect_ratio = width / (float)height;

    // Base pixel position calculation
    __m128 x_coord = _mm_sub_ps(
        _mm_mul_ps(_mm_set1_ps(aspect_ratio), _mm_div_ps(_mm_set1_ps((float)x), _mm_set1_ps((float)width))),
        _mm_mul_ps(_mm_set1_ps((aspect_ratio - 1.0f) * 0.5f + 0.5f), _mm_set1_ps(1.0f))
    );

    __m128 y_coord = _mm_sub_ps(
        _mm_div_ps(_mm_set1_ps((float)y), _mm_set1_ps((float)height)),
        _mm_set1_ps(0.5f)
    );

    __m128 z_coord = _mm_add_ps(camera_pos.simd, _mm_set1_ps(camera_near));

    Vec3_simd pixel_pos;
    pixel_pos.simd = _mm_blend_ps(
        _mm_blend_ps(x_coord, y_coord, 0b0010),
        z_coord,
        0b0100
    );

    // Sub-pixel calculations
    float sub_x = aspect_ratio / width;
    float sub_y = 1.0f / height;

    // Accumulate color with SIMD
    __m128 color_acc = _mm_setzero_ps();
    __m128 inv_samples = _mm_set1_ps(1.0f / samples);

    for (uint32_t i = 0; i < samples; ++i) {
        // Random offset within pixel
        Vec3_simd rand_pixel_pos = pixel_pos;
        float rand_x = randf() * sub_x - 0.5f * sub_x;
        float rand_y = randf() * sub_y - 0.5f * sub_y;

        rand_pixel_pos.x += rand_x;
        rand_pixel_pos.y += rand_y;

        // Create ray
        Ray ray;
        ray.pos = rand_pixel_pos;
        ray.dir = norm(sub(rand_pixel_pos, camera_pos));

        // Accumulate color
        color_acc = _mm_add_ps(color_acc, path_tracing(ray, scene, bounces).simd);
    }

    // Average samples
    Vec3_simd result;
    result.simd = _mm_mul_ps(color_acc, inv_samples);
    return result;
}