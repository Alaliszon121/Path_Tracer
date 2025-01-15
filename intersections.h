#pragma once
#include "objects.h"

bool intersect(Ray ray, Sphere sphere, Hit& hit);
bool intersect(Ray ray, Plane plane, Hit& hit);
bool intersect(Ray ray, Scene& scene, Hit& hit);