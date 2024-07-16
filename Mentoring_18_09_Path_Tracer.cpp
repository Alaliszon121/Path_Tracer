#include <iostream>
#include <cmath>
#include <thread>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define _CRT_SECURE_NO_WARNINGS
#define __STDC_LIB_EXT1__

#include "png.h"

// intersekcje promienia i sfery (przeciêcie)

struct Vec3 {
    float x, y, z;
};

struct Sphere {
    Vec3 position;
    float radius;
};

struct Plane {
    Vec3 normal;
    Vec3 position;
};

struct Ray {
    Vec3 direction;
    Vec3 position;
};

struct Hit {
    Vec3 position;
    Vec3 normal;
    float distance; // from ray to hit
    Vec3 color = { 1.0f, 1.0f, 1.0f };
};

inline float magnitude2(const Vec3& v) {
    return v.x * v.x + v.y * v.y + v.z * v.z;
}

inline float magnitude(const Vec3& v) {
    return sqrt(magnitude2(v));
}

inline Vec3 operator/(const Vec3& v, float s) {
    return { v.x / s, v.y / s, v.z / s };
}

inline Vec3 operator-(const Vec3& a, const Vec3& b) {
    return { a.x - b.x, a.y - b.y, a.z - b.z };
}

inline Vec3 operator+(const Vec3& a, const Vec3& b) {
    return { a.x + b.x, a.y + b.y, a.z + b.z };
}

inline Vec3 operator*(const Vec3& a, const Vec3& b) {
    return { a.x * b.x, a.y * b.y, a.z * b.z };
}

inline Vec3 operator*(float s, const Vec3& a) {
    return { a.x * s, a.y * s, a.z * s };
}

inline Vec3 normalize(const Vec3& v) {
    const float length = magnitude(v);

    return v / length;
}

inline float dot(const Vec3& a, const Vec3& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline Vec3 cross(const Vec3& a, const Vec3& b) {
    return { a.y * b.z - b.y * a.z,
             a.x * b.z - b.x * a.z,
             a.x * b.y - b.x * a.y };
}

inline Vec3 normalToColor(const Vec3& n) {
    const static Vec3 ones = { 1.0f, 1.0f, 1.0f };
    return 0.5f * (n + ones);
}

inline Vec3 reflect(const Vec3& dir, const Vec3& normal) {
    return dir - (2.0f * dot(dir, normal) * normal);
}

float randFloat() // (rand / max_rand) * 2.0 - 1.0)
{
    return ((rand() % 10000) / 10000.0f) * 2.0f - 1.0f;
}

Vec3 randVectorInSphere()
{
    float x = randFloat();
    float y = randFloat();
    float z = randFloat();

    float mag = sqrt(x * x + y * y + z * z);
    x /= mag; 
    y /= mag; 
    z /= mag;

    float c = cbrt((rand() % 10000) / 10000.0f);

    return {x*c, y*c, z*c};
}

bool intersect(const Ray& ray, const Sphere& sphere, Hit& hit)
{
    const float direction = dot(ray.direction, sphere.position - ray.position);

    if (direction >= 0) {
        const Vec3 b = sphere.position - ray.position;
        const float h = magnitude(cross(ray.direction, b));

        if (h <= sphere.radius) {
            const float distSphereHalfToSurface = sqrt((sphere.radius * sphere.radius) - (h * h)); // distSphereHalfToSurface is an a in the drawing
            const float fLength = dot(sphere.position - ray.position, ray.direction); // ray = 1
            const float dist = fLength - distSphereHalfToSurface;
            const Vec3 q = ((dist)*ray.direction) + ray.position;

            hit.distance = dist;
            hit.normal = normalize(q - sphere.position);
            hit.position = q;

            return true;
        }
    }

    return false;
}

float saturate(float x) {
    if (x < 0) {
        return 0;
    }

    if (x > 1) {
        return 1;
    }

    return x;
}

bool intersectPlane(const Ray& ray, const Plane& plane, Hit& hit) {
    const float denominator = dot(plane.normal, ray.direction);

    // Check if the ray is parallel to the plane
    if (std::abs(denominator) > 1e-6) {
        const Vec3 P0_minus_P = plane.position - ray.position;
        const float t = dot(P0_minus_P, plane.normal) / denominator;

        if (t >= 0) {
            hit.distance = t;
            hit.position = ray.position + t * ray.direction;
            hit.normal = plane.normal;

            return true;
        }
    }

    return false;
}

void intersectCompareAndSet(bool& isHit, Hit& hit, const Hit& tempHit, float& distance, Vec3 color)
{
    if (tempHit.distance < distance)
    {
        isHit = true;
        distance = tempHit.distance;
        hit = tempHit;
        hit.color = color;
    }
}

bool intersectScene(const Ray& ray, Hit& hit)
{
    Hit tempHit{};
    bool isHit = false;
    float distance = 2137;

    static Sphere sphere1{ {1.0f, 0.0f, 0.0f}, 0.2f };
    static Sphere sphere2{ {1.1f, 0.45f, 0.0f}, 0.2f };

    static Plane plane{ { 0.0f, 0.0f, -1.0f }, { 0.f, 0.0f, 0.2f } };

    
    if (intersect(ray, sphere1, tempHit)) {
        intersectCompareAndSet(isHit, hit, tempHit, distance, { 0.7f, 1.0f, 0.7f });
    }
    if (intersect(ray, sphere2, tempHit)) {
        intersectCompareAndSet(isHit, hit, tempHit, distance, { 1.0f, 0.7f, 1.0f });
    }

    if (intersectPlane(ray, plane, tempHit)) {
        intersectCompareAndSet(isHit, hit, tempHit, distance, { 0.7f, 0.7f, 1.0f });
    }
    

    return isHit;
}

Vec3 pathTracing(const Ray& ray, int bounces)
{
    Hit hit{};
    const static Vec3 white = { 1.0f, 1.0f, 1.0f };
    const static Vec3 blue = { 0.5f, 0.7f, 1.0f };
    const static Vec3 dirToLight = normalize({ 0.0f, 0.0f, -1.0f });

    Ray rayBounce{};

    if (bounces == 0)
    {
        if (intersectScene(ray, hit))
        {
            rayBounce = { reflect(ray.direction, hit.normal), hit.position };

            //hit.color = saturate(0.1f + saturate(dot(hit.normal, dirToLight))) * hit.color;
        }
        else
        {
            const float t = saturate(0.5f * (ray.direction.z + 1.0f));
            hit.color = (t * white) + ((1.0f - t) * blue);
        }

        return hit.color;
    }

    bounces--;
    
    // calculate bounced ray (position of hit and direction reflected)
    if (intersectScene(ray, hit))
    {
        rayBounce = { reflect(ray.direction, hit.normal), hit.position };
        rayBounce.position = (0.0001 * rayBounce.direction) + rayBounce.position;
        //hit.color = saturate(0.1f + saturate(dot(hit.normal, dirToLight))) * hit.color;
    }
    else
    {
        const float t = saturate(0.5f * (ray.direction.z + 1.0f));
        hit.color = (t * white) + ((1.0f - t) * blue);
        return hit.color;
    }

    rayBounce.direction = normalize(rayBounce.direction + (0.2f * randVectorInSphere()));

    return hit.color * pathTracing(rayBounce, bounces);
}

Vec3 render(int x, int y, int width, int height) {
    const float horizontal = ((float)x / (float)width) * 2 - 1;
    const float vertical = ((float)y / (float)height) * 2 - 1;

    const static float screenDistance = 1.0f;

    const Ray ray = { normalize({screenDistance, horizontal, vertical}), 0.0f };

    const int bounces = 8;

    const int samples = 200;

    Vec3 results{ 0.0, 0.0, 0.0 };
    for (int i = 0; i < samples; i++) {
        results = pathTracing(ray, bounces) + results;
    }
    results = results / samples;
    return results;
}

int main() {
    srand(123456789);

    int width = 512;
    int height = 512;

    int channels = 3;

    int strideInBytes = width * channels;
    unsigned char* imageData = new unsigned char[width * height * channels];

    const int THREADCOUNT = 11;
    int scope = height / THREADCOUNT; // tribal knowledge: remainder goes to last thread

    auto func = [=](int i) { 
        int x_start = scope * i;
        int x_end = scope * (i + 1);

        if(i == THREADCOUNT-1)
            x_end += (height % THREADCOUNT);

        for (int y = 0; y < height; y++) {
            for (int x = x_start; x < x_end; x++) {
                Vec3 color = render(x, y, width, height);

                unsigned char* pixel = imageData + channels * (x + width * y);

                pixel[0] = color.x * 255.0f;   // <- color.x
                pixel[1] = color.y * 255.0f; // <- color.y
                pixel[2] = color.z * 255.0f;   // <- color.z
            }
        }
    };

    std::thread t[THREADCOUNT];
    for (int i = 0; i < THREADCOUNT; i++) {
        t[i] = std::thread(func, i);
    }
    for (int i = 0; i < THREADCOUNT; i++) {
        t[i].join();
    }

    /*
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            Vec3 color = render(x, y, width, height);

            pixel[0] = color.x * 255.0f;   // <- color.x
            pixel[1] = color.y * 255.0f; // <- color.y
            pixel[2] = color.z * 255.0f;   // <- color.z

            pixel += channels;
        }
    }
    */

    int result = stbi_write_png("jakis.png", width, height, channels, imageData, strideInBytes);

    delete[] imageData;
    return 0;
}