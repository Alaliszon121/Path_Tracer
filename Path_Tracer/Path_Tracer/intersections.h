#pragma once
#include "objects.h"

// Use const references to avoid copying aligned objects
bool intersect(const Ray& ray, const Scene& scene, Hit& hit);