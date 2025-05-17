#include "intersections.h"
#include <math.h>
#include <limits>

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

bool Sphere::intersect(const Ray& ray, Hit& hit) const {
    Vec3_simd c = sub(pos, ray.pos);
    float t1 = dot(ray.dir, c);
    float c_sq = dot(c, c);
    float d_sq = c_sq - t1 * t1;

    if (t1 < 0.0f || d_sq > radius * radius) {
        return false;
    }

    float t2 = sqrt(radius * radius - d_sq);
    hit.distance = t1 - t2;
    hit.pos = add(ray.pos, mul(ray.dir, hit.distance));
    hit.normal = norm(sub(hit.pos, pos));

    if (dot(ray.dir, hit.normal) > 0.0f) {
        hit.normal = mul(hit.normal, -1.0f);
    }

    hit.color = color;
    hit.roughness = roughness;
    return true;
}

bool Plane::intersect(const Ray& ray, Hit& hit) const {
    float denom = dot(normal, ray.dir);
    if (fabs(denom) <= 1e-6f) return false;

    float dist = -(dot(ray.pos, normal) + distance) / denom;
    if (dist < 0.0f) return false;

    hit.distance = dist;
    hit.pos = add(ray.pos, mul(ray.dir, hit.distance));
    hit.normal = normal;
    hit.color = color;
    hit.roughness = roughness;
    return true;
}

bool intersect(const Ray& ray, const Scene& scene, Hit& hit) {
    Hit temp_hit;
    float min_distance = std::numeric_limits<float>::max();
    bool any_hit = false;

    for (const auto& shape : scene.shapes) {
        if (shape->intersect(ray, temp_hit) && temp_hit.distance < min_distance) {
            hit = temp_hit;
            min_distance = temp_hit.distance;
            any_hit = true;
        }
    }

    return any_hit;
}