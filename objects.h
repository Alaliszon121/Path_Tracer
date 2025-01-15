#pragma once
#include "customVector.h"

struct Sphere // kula
{
	Vec3 pos;		// pozycja, srodek kuli
	float radius;	// promien kuli

	Vec3 color;
	float roughness;	// szorstkosc (zakres od 0.0 do 0.9): 0.0 - g³adka (metaliczna), 0.9 - szorstka (rozmyta) -> NIE U¯YWAÆ 1.0
};

struct Plane // plaszczyzna
{
	Vec3 normal;	// normalna, prostopad³a do powierzchni
	float distance;	// odleg³oœæ od punktu (0,0,0) do p³aszczyzny wzd³u¿ normalnej

	Vec3 color;
	float roughness;
};

struct Ray // promien, polprosta
{
	Vec3 pos;	// pozycja, skad promien startuje
	Vec3 dir;	// k9ierunek- gdzie leci, na co "patrzy"
};

struct Hit // obiekt trafiony promieniem
{
	Vec3 pos;		// pozycja, miejsce w ktore promien trafil obiekt
	float distance;	// odleg³oœæ wzd³u¿ promienia do pozycji trafienia, u¿ywana do porównywania dwóch intersekcji(przeciec), bo musimy wiedzieæ, które z nich jest bli¿ej
	Vec3 normal;	// normalna trafionej powierzchni

	Vec3 color;
	float roughness;
};

struct Scene // scena i obiekty znajdujace sie na niej
{
	Sphere s1;
	Sphere s2;
	Sphere s3;
	Plane p1;
};
