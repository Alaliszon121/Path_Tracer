#include "customVector.h"
#include <math.h>
#include <time.h>
#include <stdlib.h>

// odejmowanie wektorow
Vec3 sub(Vec3 a, Vec3 b)
{
	return { a.x - b.x, a.y - b.y, a.z - b.z };
}

// dodawanie wektorow
Vec3 add(Vec3 a, Vec3 b)
{
	return { a.x + b.x, a.y + b.y, a.z + b.z };
}

// mnozenie wektorow
Vec3 mul(Vec3 a, Vec3 b)
{
	return { a.x * b.x, a.y * b.y, a.z * b.z };
}

// mnozenie wektora przez stala
Vec3 mul(Vec3 a, float s)
{
	return { a.x * s, a.y * s, a.z * s };
}

// cross product dwoch wektorow 3D
Vec3 cross(Vec3 a, Vec3 b)
{
	return { a.y * b.z - a.z * b.y, a.x * b.z - a.z * b.x, a.x * b.y - a.y * b.x };
}


// dot product dwoch wektorow 3D
float dot(Vec3 a, Vec3 b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

// magnitude - dlugosc wektora
float mag(Vec3 a)
{
	return sqrt(dot(a, a));	// bo dot(a,a) = a.x^2 + a.y^2 + a.z^2 , a tego potrzebujemy (reuse kodu)
}

float saturate(float a) // saturacja -> zachowanie oryginalnego koloru
{
	if (a < 0.0f)
		return 0.0f;
	if (a > 1.0f)
		return 1.0f;
	return a;
}
Vec3 saturate(Vec3 a)
{
	return { saturate(a.x), saturate(a.x), saturate(a.x) };
}

// normalizacja wektora (przeskalowanie go do dlugosci rownej 1)
Vec3 norm(Vec3 a)
{
	return mul(a, 1.0f / mag(a));
}

// obliczenia wektora odbitego wzglêdem normalnej n
Vec3 reflect(Vec3 a, Vec3 n)
{
	return sub(a, mul(n, 2.0f * dot(a, n)));
}

float randf()
{
	return rand() / (RAND_MAX + 1.0f);
}

Vec3 randf3()
{
	return { 2.0f * randf() - 1.0f, 2.0f * randf() - 1.0f, 2.0f * randf() - 1.0f };
}

Vec3 rand_in_sphere()
{
	Vec3 value = randf3();

	while (mag(value) > 1.0f)
	{
		value = randf3();
	}

	return value;
}


