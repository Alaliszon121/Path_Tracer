#include "intersections.h"
#include <math.h>

/* 
testowanie, czy promieñ przecina obiekty w scenie(sphere, plane), i dostarcza szczegó³owe informacje o punkcie przeciêcia
okreœlaj¹, w co promieñ trafia na swojej drodze - w skrocie; wykrywa obiekty

kluczowe funkcjonalnosci i ich zalety:
	- przetwarzanie wszystkich obiektów:
		* funkcja iteruje po wszystkich obiektach w scenie (s1, s2, s3 dla kul, p1 dla p³aszczyzny)
		* wywo³uje odpowiednie funkcje intersect dla ka¿dego z nich
	- znajdowanie najbli¿szego przeciêcia:
		* jeœli promieñ przecina obiekt (intersect zwraca true), sprawdzana jest odleg³oœæ do punktu przeciêcia (temp_hit.distance)
		* tylko najbli¿sze przeciêcie jest zapisywane w strukturze hit
	- rezultat:
		* funkcja zwraca true, jeœli promieñ trafi³ w cokolwiek w scenie, lub false, jeœli nie by³o trafienia
*/

bool intersect(Ray ray, Sphere sphere, Hit& hit) //przeciêcie promienia z kul¹
{
	Vec3_simd c = sub(sphere.pos, ray.pos); // wektor od pozycji promienia do œrodka kuli
	float d = mag(cross(ray.dir, c)); // odleg³oœæ pomiêdzy promieniem a œrodkiem kuli
	float t1 = dot(ray.dir, c); // odleg³oœæ wzd³u¿ promienia do najbli¿szego punktu kuli

	// sprawdzenie, czy promieñ przecina kulê
	// jeœli odleg³oœæ miêdzy œrodkiem kuli a promieniem jest mniejsza lub równa promieniowi, to mamy trafienie!
	if (t1 > 0.0f && d <= sphere.radius)
	{
		float t2 = sqrtf(sphere.radius * sphere.radius - d * d); // odleg³osc od punktu najblizszego do punktu przeciecia

		hit.distance = t1 - t2; // odleg³oœæ od promienia do punktu przeciêcia
		hit.pos = add(ray.pos, mul(ray.dir, hit.distance)); // pozycja punktu przeciêcia
		hit.normal = norm(sub(hit.pos, sphere.pos)); // normalna w punkcie przeciêcia
		hit.color = sphere.color; // kolor kuli
		hit.roughness = sphere.roughness; // szorstkoœæ kuli

		// odwracanie normalnej, jesli jest skierowana w tym samym kierunku co promien
		if (dot(ray.dir, hit.normal) > 0.0f)
		{
			hit.normal = mul(hit.normal, -1.0f);
		}

		return true; // promieñ przecina kule
	}

	return false; // promieñ nie przecina kuli
}

bool intersect(Ray ray, Plane plane, Hit& hit) //przeciêcie promienia z plaszczyzna
{
	float denom = dot(ray.dir, plane.normal); // iloczyn skalarny: sprawdza nachylenie promienia wzglêdem p³aszczyzny
	if (denom > 0.000001f) // jeœli denom jest bliskie zeru, oznacza to, ¿e promieñ jest równoleg³y do p³aszczyzny i nie ma przeciêcia
	{
		hit.distance = -(dot(ray.pos, plane.normal) + plane.distance) / denom; // odleg³oœæ do punktu przeciêcia
		hit.pos = add(ray.pos, mul(ray.dir, hit.distance)); // pozycja punktu przeciêcia
		hit.normal = plane.normal; // normalna p³aszczyzny
		hit.color = plane.color; // kolor
		hit.roughness = plane.roughness; // szorstkosc

		return true; // promieñ przecina plaszczyzne
	}

	return false; // promieñ nie przecina plaszczyzny
}

bool intersect(Ray ray, Scene& scene, Hit& hit) // przeciêcie promienia z ca³¹ scen¹.
{
	Hit temp_hit = {};
	float distance = 10000.0f; // pocz¹tkowo bardzo du¿a odleg³oœæ
	bool is_hit = 0.0f;

	// test przeciêcia promienia z ka¿d¹ kul¹ i p³aszczyzn¹ w scenie
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