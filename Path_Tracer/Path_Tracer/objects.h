#pragma once
#include "vec3_simd.h"
#include <memory>
#include <vector>

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

// Base class for all shapes
class alignas(16) Shape {
public:
    Vec3_simd color;    // Color (16-byte aligned)
    float roughness;    // 0.0 (smooth) to 0.9 (rough)
    virtual ~Shape() = default;
    virtual bool intersect(const Ray& ray, Hit& hit) const = 0;
};

// Sphere with SIMD-aligned members
class alignas(16) Sphere : public Shape {
public:
    Vec3_simd pos;      // Center (16-byte aligned)
    float radius;       // Sphere radius
    bool intersect(const Ray& ray, Hit& hit) const override;
};

// Plane with SIMD-aligned members
class alignas(16) Plane : public Shape {
public:
    Vec3_simd normal;   // Surface normal (16-byte aligned)
    float distance;     // Distance from origin
    bool intersect(const Ray& ray, Hit& hit) const override;
};

class Scene {
public:
    std::vector<std::unique_ptr<Shape>> shapes;

    // Helper functions to add shapes
    void add_sphere(Vec3_simd pos, float radius, Vec3_simd color, float roughness) {
        auto sphere = std::make_unique<Sphere>();
        sphere->pos = pos;
        sphere->radius = radius;
        sphere->color = color;
        sphere->roughness = roughness;
        shapes.push_back(std::move(sphere));
    }

    void add_plane(Vec3_simd normal, float distance, Vec3_simd color, float roughness) {
        auto plane = std::make_unique<Plane>();
        plane->normal = normal;
        plane->distance = distance;
        plane->color = color;
        plane->roughness = roughness;
        shapes.push_back(std::move(plane));
    }
};
