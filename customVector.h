#pragma once

// 3D vector (or color, or whatever has 3 floats)
struct Vec3
{
	float x, y, z;
};

Vec3 sub(Vec3 a, Vec3 b);
Vec3 add(Vec3 a, Vec3 b);

Vec3 mul(Vec3 a, Vec3 b);
Vec3 mul(Vec3 a, float s);

Vec3 cross(Vec3 a, Vec3 b);
float dot(Vec3 a, Vec3 b);
float mag(Vec3 a);

float saturate(float a);
Vec3 saturate(Vec3 a);

Vec3 norm(Vec3 a);
Vec3 reflect(Vec3 a, Vec3 n);

float randf();
Vec3 randf3();
Vec3 rand_in_sphere();