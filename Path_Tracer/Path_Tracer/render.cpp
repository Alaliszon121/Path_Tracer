#include "render.h"

/*
Ca³y proces path tracingu
Dla ka¿dego piksela:
	- generowane s¹ promienie dla ró¿nych próbek w obrêbie piksela (losowe pod-piksele)
	- promienie s¹ œledzone w scenie za pomoc¹ funkcji path_tracing:
		* odbicia s¹ rekurencyjnie obliczane a¿ do osi¹gniêcia maksymalnej liczby odbiæ (bounces) lub braku trafienia w obiekt
		* kolory obiektów s¹ propagowane wzd³u¿ promieni odbitych 
		  (informacje o kolorze obiektu, na który promieñ pada, s¹ uwzglêdniane i przekazywane dalej przez kolejne odbicia promienia)
	- poœredni wynik jest kolorowany gradientem t³a, jeœli promieñ niczego nie trafia
	- finalny kolor piksela jest œredni¹ z próbek

Funkcjonalnosci i ich zalety:
	- fizycznie poprawne œwiat³o: odbicia, rozpraszanie (szorstkoœæ), gradient t³a
	- Monte Carlo: u¿ycie losowych próbek do symulacji antyaliasingu i rozpraszania œwiat³a
	- rekurencyjny algorytm: funkcja path_tracing symuluje rozchodzenie siê œwiat³a przez odbicia, a¿ do osi¹gniêcia t³a lub limitu odbiæ
*/

// przesuwa pocz¹tek promienia r minimalnie w jego kierunku, aby unikn¹æ problemów z samonak³adaniem siê geometrii
// tzw.offset ray origin, stosowany w renderowaniu w celu unikniêcia problemów numerycznych
void adjust(Ray& r)
{
	r.pos = add(r.pos, mul(r.dir, 0.0001f));
}

// dodaje do kierunku promienia pewn¹ losow¹ perturbacjê
// symuluje rozpraszanie œwiat³a w zaleznosci od szorstkosci powierzchni
void perturb(Ray& r, float degree)
{
	Vec3 v = mul(rand_in_sphere(), degree);
	r.dir = norm(add(r.dir, v));
}

// KLUCZOWA FUNKCJA PROGRAMU - sledzi odbijanie promienia (f. rekurencyjna)
/*
Gdy promieñ trafia w obiekt:
	1) obliczenie kierunku odbicia promienia
	2) stworzenie promienia odbitego 
		punkt pocz¹tkowy: punkt trafienia, 
		kierunek: reflected, 
		offset pocz¹tkowy, 
		szum: losowe zaburzenie odbicia, symuluj¹c szorstkoœæ powierzchni
	3) mno¿y kolor obiektu przez wynik rekurencyjnego wywo³ania path_tracing dla promienia odbitego
*/
Vec3 path_tracing(Ray ray, Scene& scene, uint32_t bounces)
{
	// gdy promieñ niczego nie trafia funkcja zwraca gradient jako kolor t³a, 
	// przechodz¹cy od bieli do b³êkitu w zale¿noœci od wysokoœci
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

	return mul(color, path_tracing(ray_bounce, scene, bounces)); // mno¿y kolor obiektu (hit.color) przez wynik rekurencyjnego wywo³ania path_tracing dla promienia odbitego
}

// Renderuje kolor piksela
Vec3 render(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t bounces, uint32_t samples, Scene& scene)
{
	// Kamera: 
	Vec3 camera_pos = { 0.0f, 0.0f, -3.0f };
	float camera_near = 0.5f;	// odleg³oœæ od pozycji kamery do bliskiej p³aszczyzny kamery

	// pozycja piksela znajduje siê na bliskiej p³aszczyŸnie kamery
	float aspect_ratio = width / (float)height;
	Vec3 pixel_pos = {
		aspect_ratio * (float)x / (float)width - (aspect_ratio - 1.0f) * 0.5f - 0.5f,
		(float)y / (float)height - 0.5f,
		camera_pos.z + camera_near
	};

	float sub_x = aspect_ratio / width;
	float sub_y = 1.0f / height;

	Vec3 color = {};

	// Podzia³ piksela na próbki- antyaliasing: generuje wiele promieni w losowych punktach w obrêbie piksela
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

	// wynik dzielony przez liczbê próbek (samples) w celu uzyskania œredniego koloru
	return mul(color, 1.0f / (float)samples); 
}