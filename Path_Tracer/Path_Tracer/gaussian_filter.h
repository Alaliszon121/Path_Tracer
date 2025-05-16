#pragma once
#include <vector>

void generate_gaussian_kernel(std::vector<float>& kernel, int radius, float sigma);
void apply_gaussian_filter(uint8_t* image, int width, int height, int stride, int radius, float sigma);