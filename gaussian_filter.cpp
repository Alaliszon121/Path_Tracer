#include "gaussian_filter.h"

// generowanie jadra filtra Gaussowskiego zeby ukryc szum
void generate_gaussian_kernel(std::vector<float>& kernel, int radius, float sigma)
{
	int size = 2 * radius + 1; // rozmiar jadra filtra (liczba próbek)
	kernel.resize(size);
	float sum = 0.0f;

	for (int i = -radius; i <= radius; ++i)
	{
		kernel[i + radius] = exp(-0.5f * (i * i) / (sigma * sigma)); // funkcja Gaussowska
		sum += kernel[i + radius];
	}

	// normalizowanie jadra
	// zapobiega zwiêkszeniu lub zmniejszeniu jasnoœci obrazu po zastosowaniu filtra
	for (auto& value : kernel)
	{
		value /= sum;
	}
}

// zastosowanie filtra Gaussowskiego do obrazu
void apply_gaussian_filter(uint8_t* image, int width, int height, int stride, int radius, float sigma)
{
	std::vector<float> kernel;
	generate_gaussian_kernel(kernel, radius, sigma);

	std::vector<uint8_t> temp_image(width * height * stride);

	// w poziomie dla ka¿dego wiersza
	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			float r = 0.0f, g = 0.0f, b = 0.0f, w = 0.0f;

			for (int k = -radius; k <= radius; ++k)
			{
				int nx = std::min(std::max(x + k, 0), width - 1); // zapewnia, ¿e wspó³rzêdne nie wychodz¹ poza obraz
				const uint8_t* pixel = &image[stride * (nx + y * width)];

				// na ka¿dym pikselu filtr jest stosowany w poziomie na podstawie s¹siadów w okreœlonym promieniu
				// oddzielnie dla ka¿dej sk³adowej koloru (R, G, B)
				r += kernel[k + radius] * pixel[0];
				g += kernel[k + radius] * pixel[1];
				b += kernel[k + radius] * pixel[2];
			}

			uint8_t* out_pixel = &temp_image[stride * (x + y * width)];
			out_pixel[0] = static_cast<uint8_t>(r);
			out_pixel[1] = static_cast<uint8_t>(g);
			out_pixel[2] = static_cast<uint8_t>(b);
		}
	}

	// w pionie dla ka¿dej kolumny - analogicznie jak dla wierszy
	for (int x = 0; x < width; ++x)
	{
		for (int y = 0; y < height; ++y)
		{
			float r = 0.0f, g = 0.0f, b = 0.0f;

			for (int k = -radius; k <= radius; ++k)
			{
				int ny = std::min(std::max(y + k, 0), height - 1);
				const uint8_t* pixel = &temp_image[stride * (x + ny * width)];

				r += kernel[k + radius] * pixel[0];
				g += kernel[k + radius] * pixel[1];
				b += kernel[k + radius] * pixel[2];
			}

			uint8_t* out_pixel = &image[stride * (x + y * width)];
			out_pixel[0] = static_cast<uint8_t>(r);
			out_pixel[1] = static_cast<uint8_t>(g);
			out_pixel[2] = static_cast<uint8_t>(b);
		}
	}
}
