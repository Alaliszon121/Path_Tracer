#include "intersections.h"
#include <math.h>

/* 
testowanie, czy promie� przecina obiekty w scenie(sphere, plane), i dostarcza szczeg�owe informacje o punkcie przeci�cia
okre�laj�, w co promie� trafia na swojej drodze - w skrocie; wykrywa obiekty

kluczowe funkcjonalnosci i ich zalety:
	- przetwarzanie wszystkich obiekt�w:
		* funkcja iteruje po wszystkich obiektach w scenie (s1, s2, s3 dla kul, p1 dla p�aszczyzny)
		* wywo�uje odpowiednie funkcje intersect dla ka�dego z nich
	- znajdowanie najbli�szego przeci�cia:
		* je�li promie� przecina obiekt (intersect zwraca true), sprawdzana jest odleg�o�� do punktu przeci�cia (temp_hit.distance)
		* tylko najbli�sze przeci�cie jest zapisywane w strukturze hit
	- rezultat:
		* funkcja zwraca true, je�li promie� trafi� w cokolwiek w scenie, lub false, je�li nie by�o trafienia
*/

bool intersect(Ray ray, Sphere sphere, Hit& hit) //przeci�cie promienia z kul�
{
	Vec3_simd c = sub(sphere.pos, ray.pos); // wektor od pozycji promienia do �rodka kuli
	float d = mag(cross(ray.dir, c)); // odleg�o�� pomi�dzy promieniem a �rodkiem kuli
	float t1 = dot(ray.dir, c); // odleg�o�� wzd�u� promienia do najbli�szego punktu kuli

	// sprawdzenie, czy promie� przecina kul�
	// je�li odleg�o�� mi�dzy �rodkiem kuli a promieniem jest mniejsza lub r�wna promieniowi, to mamy trafienie!
	if (t1 > 0.0f && d <= sphere.radius)
	{
		float t2 = sqrtf(sphere.radius * sphere.radius - d * d); // odleg�osc od punktu najblizszego do punktu przeciecia

		hit.distance = t1 - t2; // odleg�o�� od promienia do punktu przeci�cia
		hit.pos = add(ray.pos, mul(ray.dir, hit.distance)); // pozycja punktu przeci�cia
		hit.normal = norm(sub(hit.pos, sphere.pos)); // normalna w punkcie przeci�cia
		hit.color = sphere.color; // kolor kuli
		hit.roughness = sphere.roughness; // szorstko�� kuli

		// odwracanie normalnej, jesli jest skierowana w tym samym kierunku co promien
		if (dot(ray.dir, hit.normal) > 0.0f)
		{
			hit.normal = mul(hit.normal, -1.0f);
		}

		return true; // promie� przecina kule
	}

	return false; // promie� nie przecina kuli
}

bool intersect(Ray ray, Plane plane, Hit& hit) //przeci�cie promienia z plaszczyzna
{
	float denom = dot(ray.dir, plane.normal); // iloczyn skalarny: sprawdza nachylenie promienia wzgl�dem p�aszczyzny
	if (denom > 0.000001f) // je�li denom jest bliskie zeru, oznacza to, �e promie� jest r�wnoleg�y do p�aszczyzny i nie ma przeci�cia
	{
		hit.distance = -(dot(ray.pos, plane.normal) + plane.distance) / denom; // odleg�o�� do punktu przeci�cia
		hit.pos = add(ray.pos, mul(ray.dir, hit.distance)); // pozycja punktu przeci�cia
		hit.normal = plane.normal; // normalna p�aszczyzny
		hit.color = plane.color; // kolor
		hit.roughness = plane.roughness; // szorstkosc

		return true; // promie� przecina plaszczyzne
	}

	return false; // promie� nie przecina plaszczyzny
}

bool intersect(Ray ray, Scene& scene, Hit& hit) // przeci�cie promienia z ca�� scen�.
{
	Hit temp_hit = {};
	float distance = 10000.0f; // pocz�tkowo bardzo du�a odleg�o��
	bool is_hit = 0.0f;

	// test przeci�cia promienia z ka�d� kul� i p�aszczyzn� w scenie
	if (intersect(ray, scene.s1, temp_hit))
	{
		if (temp_hit.distance < distance)
		{
			hit = temp_hit;
			distance = temp_hit.distance;
			is_hit = true;
		}
	}

	if (intersect(ray, scene.s2, temp_hit))
	{
		if (temp_hit.distance < distance)
		{
			hit = temp_hit;
			distance = temp_hit.distance;
			is_hit = true;
		}
	}

	if (intersect(ray, scene.s3, temp_hit))
	{
		if (temp_hit.distance < distance)
		{
			hit = temp_hit;
			distance = temp_hit.distance;
			is_hit = true;
		}
	}

	if (intersect(ray, scene.p1, temp_hit))
	{
		if (temp_hit.distance < distance)
		{
			hit = temp_hit;
			distance = temp_hit.distance;
			is_hit = true;
		}
	}

	return is_hit;
}