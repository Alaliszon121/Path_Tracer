#include <iostream>
#include <string.h>
#include <conio.h> 

// dla wielowatkowosci
#include <thread>
#include <atomic>
#include <mutex>
std::mutex console_mutex;

// pliki z logika programu
// #include "customVector.h"
#include "objects.h"
#include "intersections.h"
#include "render.h"
#include "gaussian_filter.h"
#include "vec3_simd.h"

// definicje zapobiegajace ostrzezeniom z zewnetrznej biblioteki do zapisywania wyrenderowanego obrazu do pliku
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define _CRT_SECURE_NO_WARNINGS
#define __STDC_LIB_EXT1__
// zewnetrzna biblioteka do zapisywania wyrenderowanego obrazu do pliku
#include "png.h"

int main(int argc, const char* argv[])
{
	// menu
	printf("PATH TRACER\n");
	printf("Ten program generuje obraz w 3D za pomoca Path Tracingu\n");
	printf("\n\nOpcje:\n");
	printf("Renderowana jakosc obrazu (1- wysoka, 2 - niska): ");
	int quality = 1; std::cin >> quality;
	while (quality != 1 && quality != 2) {
		printf("Wpisana wartosc jest bledna:\n");
		printf("Renderowana jakosc obrazu (1- wysoka, 2 - niska): ");
		std::cin >> quality;
	}
	printf("\nUWAGA: Wyswietlenie postepu spowolni dzialanie algorytmu renderowania\n");
	printf("Czy wyswietlac postep path tracera (1- tak, 0- nie): ");
	bool progress = false; std::cin >> progress; 
	printf("\nUWAGA: Wybranie rozmycia obrazu wydluzy dzialanie programu\n");
	printf("Czy rozmyc obraz po wyrenderowaniu (1- tak, 0- nie): ");
	bool gaussian = false; std::cin >> gaussian;
	system("cls");

	srand((uint32_t)time(NULL));

	// ustawienia
	const uint32_t width = 1024;
	const uint32_t height = 768;
	const uint32_t bounces = 10 / quality;
	if (quality == 2) quality = 100;
	const uint32_t samples = 1000 / quality;
	const uint32_t tile_size = 64;
	const uint32_t batch_size = 1;

	// inne zmienne
	const uint32_t stride = 3; // glebia obrazu -> 3 dla RGB 
	const uint32_t image_size = width * height * stride; // wielkosc obrazu
	const uint32_t num_threads = std::thread::hardware_concurrency(); // ilosc watkow
	printf("Program rozpoczal dzialanie na %i watkach...\n", num_threads);

	void* image = malloc(image_size); // alokowanie bloku pamieci dla obrazu
	memset(image, 0, image_size); // wypelnia zaalokowana pamiêc (wielkosci image_size) bloku wskazywanego przez image na 0  

	// ustawienia sceny, dodanie obiektow na scene 3D
	Scene scene;

	// Add plane
	scene.add_plane(
		Vec3_simd(0.0f, 1.0f, 0.0f),    // normal (will be auto-normalized)
		-1.0f,                          // distance
		Vec3_simd(0.8f, 0.8f, 0.8f),    // color
		0.9f                            // roughness
	);

	// Add spheres (converting Vec3 literals to Vec3_simd)
	scene.add_sphere(
		Vec3_simd(-2.0f, 0.0f, 0.0f),   // position
		1.0f,                           // radius
		Vec3_simd(1.0f, 0.5f, 0.8f),    // color
		0.04f                           // roughness
	);

	scene.add_sphere(Vec3_simd(0.0f, 0.0f, 0.0f), 1.0f, Vec3_simd(0.6f, 0.9f, 0.6f), 0.3f);

	scene.add_sphere(Vec3_simd(2.0f, 0.0f, 0.0f), 1.0f, Vec3_simd(0.8f, 0.4f, 0.8f), 0.9f);

	// szerokosc i wysokosc fragmentow (kazdy watek dostaje pewna ilosc fragmentow obrazu do wyrenderowania)
	std::atomic<uint32_t> next_tile(0);
	const uint32_t num_tiles_x = (width + tile_size - 1) / tile_size;
	const uint32_t num_tiles_y = (height + tile_size - 1) / tile_size;
	const uint32_t total_tiles = num_tiles_x * num_tiles_y;

	std::vector<std::thread> jobs;
	std::atomic<uint32_t> tiles_done(0);

	// praca watkow
	for (uint32_t t = 0; t < num_threads; ++t)
	{
		jobs.emplace_back([&]() {
			while (true)
			{
				uint32_t start_tile_index = next_tile.fetch_add(batch_size); // ustawienie poczatkowego fragmentu zeby watek wiedzial jaki zakres fragmentow pobrac
				
				if (start_tile_index >= total_tiles)
					break;

				uint32_t end_tile_index = std::min(start_tile_index + batch_size, total_tiles); // wyznaczenie ostatniego pobranego fragmentu przez watek

				// renderowanie po kolei kazdego fragmentu
				for (uint32_t tile_index = start_tile_index; tile_index < end_tile_index; ++tile_index)
				{
					uint32_t tile_x = (tile_index % num_tiles_x) * tile_size;
					uint32_t tile_y = (tile_index / num_tiles_x) * tile_size;

					// renderowanie po kolei kazdego piksela z danego fragmentu
					for (uint32_t y = tile_y; y < tile_y + tile_size && y < height; ++y)
					{
						for (uint32_t x = tile_x; x < tile_x + tile_size && x < width; ++x)
						{
							Vec3_simd color = render(x, y, width, height, bounces, samples, scene); // wyliczenie kolorow RGB piksela
							uint8_t* pixel = (uint8_t*)image + stride * (x + y * width);

							// zapisanie do piksela kolorow RGB
							pixel[0] = static_cast<uint8_t>(saturate(color.x) * 255.0f);
							pixel[1] = static_cast<uint8_t>(saturate(color.y) * 255.0f);
							pixel[2] = static_cast<uint8_t>(saturate(color.z) * 255.0f);
						}
					}

					uint32_t done = tiles_done.fetch_add(1) + 1;

					float percent = (100.0f * done) / total_tiles;
					printf("\rProgress: %.2f%% (%u / %u tiles)", percent, done, total_tiles);
					fflush(stdout); // force flush for \r to work properly
					
				}
			}
		});
	}

	// dolaczanie watkow
	for (auto& job : jobs)
	{
		job.join();
	}

	printf("\nRenderowanie obrazu zakonczone.\n");

	if (gaussian) {
		printf("Aplikowanie filtru Gaussa...\n");
		apply_gaussian_filter((uint8_t*)image, width, height, stride, 3, 1.0f); // aplikowanie filtru gaussa na wyrenderowany obraz (radius = 3, sigma = 1.0)
		printf("Filtr zaaplikowany!\n");
	}

	// zapis wyrenderowanego obrazu do pliku
	int32_t res = stbi_write_png("render.png", width, height, 3, image, stride * width);

	if (res)
		printf("\nObraz zostal zapisany do pliku render.png\n");
	else
		printf("\nBlad zapisu obrazu do pliku render.png\n");

	free(image);
	return res;
}