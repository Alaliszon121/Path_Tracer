#include "render.h"

/*
Ca�y proces path tracingu
Dla ka�dego piksela:
	- generowane s� promienie dla r�nych pr�bek w obr�bie piksela (losowe pod-piksele)
	- promienie s� �ledzone w scenie za pomoc� funkcji path_tracing:
		* odbicia s� rekurencyjnie obliczane a� do osi�gni�cia maksymalnej liczby odbi� (bounces) lub braku trafienia w obiekt
		* kolory obiekt�w s� propagowane wzd�u� promieni odbitych 
		  (informacje o kolorze obiektu, na kt�ry promie� pada, s� uwzgl�dniane i przekazywane dalej przez kolejne odbicia promienia)
	- po�redni wynik jest kolorowany gradientem t�a, je�li promie� niczego nie trafia
	- finalny kolor piksela jest �redni� z pr�bek

Funkcjonalnosci i ich zalety:
	- fizycznie poprawne �wiat�o: odbicia, rozpraszanie (szorstko��), gradient t�a
	- Monte Carlo: u�ycie losowych pr�bek do symulacji antyaliasingu i rozpraszania �wiat�a
	- rekurencyjny algorytm: funkcja path_tracing symuluje rozchodzenie si� �wiat�a przez odbicia, a� do osi�gni�cia t�a lub limitu odbi�
*/

// przesuwa pocz�tek promienia r minimalnie w jego kierunku, aby unikn�� problem�w z samonak�adaniem si� geometrii
// tzw.offset ray origin, stosowany w renderowaniu w celu unikni�cia problem�w numerycznych
void adjust(Ray& r)
{
	r.pos = add(r.pos, mul(r.dir, 0.0001f));
}

// dodaje do kierunku promienia pewn� losow� perturbacj�
// symuluje rozpraszanie �wiat�a w zaleznosci od szorstkosci powierzchni
void perturb(Ray& r, float degree)
{
	Vec3 v = mul(rand_in_sphere(), degree);
	r.dir = norm(add(r.dir, v));
}

// KLUCZOWA FUNKCJA PROGRAMU - sledzi odbijanie promienia (f. rekurencyjna)
/*
Gdy promie� trafia w obiekt:
	1) obliczenie kierunku odbicia promienia
	2) stworzenie promienia odbitego 
		punkt pocz�tkowy: punkt trafienia, 
		kierunek: reflected, 
		offset pocz�tkowy, 
		szum: losowe zaburzenie odbicia, symuluj�c szorstko�� powierzchni
	3) mno�y kolor obiektu przez wynik rekurencyjnego wywo�ania path_tracing dla promienia odbitego
*/
Vec3 path_tracing(Ray ray, Scene& scene, uint32_t bounces)
{
	// gdy promie� niczego nie trafia funkcja zwraca gradient jako kolor t�a, 
	// przechodz�cy od bieli do b��kitu w zale�no�ci od wysoko�ci
	Hit hit = {};
	if (bounces == 0 || !intersect(ray, scene, hit))
	{
		Vec3 white = { 1.0f, 1.0f, 1.0f };
		Vec3 blue = { 0.5f, 0.7f, 1.0f };
		float t = saturate(0.5f * (ray.dir.y + 1.0f));
		return add(mul(white, t), mul(blue, 1.0f - t));
	}

	bounces--;

	Vec3 reflected = reflect(ray.dir, hit.normal); // obliczenie odbicia

	Ray ray_bounce;
	ray_bounce.pos = hit.pos; // punkt poczatkowy = punkt trafienia
	ray_bounce.dir = reflected; // kierunek odbicia
	adjust(ray_bounce); // przesuniecie poczatku promienia
	perturb(ray_bounce, hit.roughness); // losowe zaburzenie odbicia

	Vec3 color = hit.color;

	return mul(color, path_tracing(ray_bounce, scene, bounces)); // mno�y kolor obiektu (hit.color) przez wynik rekurencyjnego wywo�ania path_tracing dla promienia odbitego
}

// Renderuje kolor piksela
Vec3 render(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t bounces, uint32_t samples, Scene& scene)
{
	// Kamera: 
	Vec3 camera_pos = { 0.0f, 0.0f, -3.0f };
	float camera_near = 0.5f;	// odleg�o�� od pozycji kamery do bliskiej p�aszczyzny kamery

	// pozycja piksela znajduje si� na bliskiej p�aszczy�nie kamery
	float aspect_ratio = width / (float)height;
	Vec3 pixel_pos = {
		aspect_ratio * (float)x / (float)width - (aspect_ratio - 1.0f) * 0.5f - 0.5f,
		(float)y / (float)height - 0.5f,
		camera_pos.z + camera_near
	};

	float sub_x = aspect_ratio / width;
	float sub_y = 1.0f / height;

	Vec3 color = {};

	// Podzia� piksela na pr�bki- antyaliasing: generuje wiele promieni w losowych punktach w obr�bie piksela
	for (uint32_t i = 0; i < samples; ++i)
	{
		Vec3 rand_pixel_pos = pixel_pos;
		rand_pixel_pos.x += randf() * sub_x - 0.5f * sub_x;
		rand_pixel_pos.y += randf() * sub_y - 0.5f * sub_y;

		// ray starting at pixel position
		Ray ray;
		ray.pos = rand_pixel_pos;
		ray.dir = norm(sub(rand_pixel_pos, camera_pos));

		color = add(color, path_tracing(ray, scene, bounces));
	}

	// wynik dzielony przez liczb� pr�bek (samples) w celu uzyskania �redniego koloru
	return mul(color, 1.0f / (float)samples); 
}