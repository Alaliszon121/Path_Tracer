# Path Tracer
Simple path tracer made by me in C++ using http://nothings.org/stb/ to write out PNG image

<img src="https://github.com/user-attachments/assets/2bd89bf9-f5e5-4fe6-89c8-c0131cf714b5" width="200" height="200" />

### Roadmap:

<img src="https://github.com/user-attachments/assets/82ed804d-c49c-4e5e-b98c-8a6fcc32c2d7" width="600" height="700" />

 ## Spis Treści
1. [Wektory i Operacje na Wektorach](#wektory-i-operacje-na-wektorach)
2. [Intersekcja Promienia z Obiektem Sferycznym](#intersekcja-promienia-z-obiektem-sferycznym)
3. [Intersekcja Promienia z Płaszczyzną](#intersekcja-promienia-z-płaszczyzną)
4. [Refleksja Promienia na Powierzchni](#refleksja-promienia-na-powierzchni)
5. [Generowanie Liczb Losowych](#generowanie-liczb-losowych)
6. [Metoda Śledzenia Promieni](#metoda-śledzenia-promieni)
7. [Funkcja Renderująca Obraz](#funkcja-renderująca-obraz)
8. [Wielowątkowość](#wielowątkowość)
9. [Porównywanie Intersekcji i Ustawianie Danych Wynikowych](#porównywanie-intersekcji-i-ustawianie-danych-wynikowych)
10. [Saturacja](#saturacja)

## Wektory i Operacje na Wektorach

### Struktura `Vec3`
```cpp
struct Vec3 {
    float x, y, z;
};
```
Struktura reprezentująca wektor w trójwymiarowej przestrzeni.

### Operacje na Wektorach
- **Magnitude**: Oblicza długość wektora.
  ```cpp
  inline float magnitude(const Vec3& v) {
      return sqrt(magnitude2(v));
  }
  ```

- **Normalizacja**: Normalizuje wektor do jednostkowego.
  ```cpp
  inline Vec3 normalize(const Vec3& v) {
      const float length = magnitude(v);
      return v / length;
  }
  ```

- **Iloczyn Skalarny**: Oblicza iloczyn skalarny dwóch wektorów.
  ```cpp
  inline float dot(const Vec3& a, const Vec3& b) {
      return a.x * b.x + a.y * b.y + a.z * b.z;
  }
  ```

- **Iloczyn Wektorowy**: Oblicza iloczyn wektorowy dwóch wektorów.
  ```cpp
  inline Vec3 cross(const Vec3& a, const Vec3& b) {
      return { a.y * b.z - b.y * a.z, a.x * b.z - b.x * a.z, a.x * b.y - b.x * a.y };
  }
  ```

## Intersekcja Promienia z Obiektem Sferycznym

### Struktura `Sphere`
```cpp
struct Sphere {
    Vec3 position;
    float radius;
};
```
Struktura reprezentująca sferę w trójwymiarowej przestrzeni.

### Funkcja `intersect`
```cpp
bool intersect(const Ray& ray, const Sphere& sphere, Hit& hit);
```
Funkcja sprawdzająca, czy promień przecina sferę. Zwraca `true` jeśli tak, i ustawia dane wyniku w strukturze `Hit`.

## Intersekcja Promienia z Płaszczyzną

### Struktura `Plane`
```cpp
struct Plane {
    Vec3 normal;
    Vec3 position;
};
```
Struktura reprezentująca płaszczyznę w trójwymiarowej przestrzeni.

### Funkcja `intersectPlane`
```cpp
bool intersectPlane(const Ray& ray, const Plane& plane, Hit& hit);
```
Funkcja sprawdzająca, czy promień przecina płaszczyznę. Zwraca `true` jeśli tak, i ustawia dane wyniku w strukturze `Hit`.

## Refleksja Promienia na Powierzchni

### Funkcja `reflect`
```cpp
inline Vec3 reflect(const Vec3& dir, const Vec3& normal) {
    return dir - (2.0f * dot(dir, normal) * normal);
}
```
Funkcja obliczająca wektor odbicia promienia od powierzchni.

## Generowanie Liczb Losowych

### Funkcja `randFloat`
```cpp
float randFloat() {
    return ((rand() % 10000) / 10000.0f) * 2.0f - 1.0f;
}
```
Funkcja generująca losową liczbę zmiennoprzecinkową z zakresu [-1, 1].

### Funkcja `randVectorInSphere`
```cpp
Vec3 randVectorInSphere() {
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
```
Funkcja generująca losowy wektor wewnątrz jednostkowej sfery.

## Metoda Śledzenia Promieni

### Funkcja `pathTracing`
```cpp
Vec3 pathTracing(const Ray& ray, int bounces);
```
Rekurencyjna funkcja śledząca promienie w scenie, uwzględniając odbicia.

## Funkcja Renderująca Obraz

### Funkcja `render`
```cpp
Vec3 render(int x, int y, int width, int height);
```
Funkcja generująca kolor piksela na podstawie współrzędnych w obrazie.

## Wielowątkowość

### Użycie Wątków
```cpp
const int THREADCOUNT = 11;
int scope = height / THREADCOUNT; // Zakres dla każdego wątku

auto func = [=](int i) { 
    int x_start = scope * i;
    int x_end = scope * (i + 1);

    if(i == THREADCOUNT-1)
        x_end += (height % THREADCOUNT);

    for (int y = 0; y < height; y++) {
        for (int x = x_start; x < x_end; x++) {
            Vec3 color = render(x, y, width, height);

            unsigned char* pixel = imageData + channels * (x + width * y);

            pixel[0] = color.x * 255.0f;
            pixel[1] = color.y * 255.0f;
            pixel[2] = color.z * 255.0f;
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
```
Kod implementujący wielowątkowe renderowanie obrazu.

## Porównywanie Intersekcji i Ustawianie Danych Wynikowych

### Funkcja `intersectCompareAndSet`
```cpp
void intersectCompareAndSet(bool& isHit, Hit& hit, const Hit& tempHit, float& distance, Vec3 color);
```
Funkcja porównująca intersekcje i ustawiająca odpowiednie dane wynikowe, jeśli nowe trafienie jest bliższe.

## Saturacja

### Funkcja `saturate`
```cpp
float saturate(float x) {
    if (x < 0) {
        return 0;
    }

    if (x > 1) {
        return 1;
    }

    return x;
}
```
Funkcja ograniczająca wartość zmiennoprzecinkową do zakresu [0, 1].
