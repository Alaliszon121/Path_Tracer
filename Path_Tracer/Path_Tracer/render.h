#pragma once

#include "objects.h"
#include "intersections.h"
#include <stdint.h>

void adjust(Ray& r);
void perturb(Ray& r, float degree);
Vec3_simd path_tracing(Ray ray, Scene& scene, uint32_t bounces);
Vec3_simd render(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t bounces, uint32_t samples, Scene& scene);