#include "pch.h"
#include <iostream>
#include <cstdint>
#include <cassert>
#include "Image.hpp"
#include "ImageColor.hpp"
#include "Debugger.hpp"
#include "Test.hpp"

struct color {
	uint8_t r, g, b;
};

union doubleColor {
	struct {
		double r, g, b;
	};
	struct {
		double x, y, z;
	};
	struct {
		double ls, as, bs;
	};
};

color averageBlockColor(uint8_t* image, int w, int h, int xStart, int xStop, int yStart, int yStop);
color guessBackgroundColor(uint8_t* image, int w, int h);
doubleColor colorToDoubleColor(color in);
color doubleColorToColor(doubleColor in);
double linearizeColorComponent(double color);
doubleColor linearizeRgb(doubleColor in);
doubleColor rgbToCiexyz(doubleColor in);
doubleColor ciexyzToCielab(doubleColor in);
doubleColor rgbToCielab(color in);
double colorToGrayscale(doubleColor in);

std::vector<double*> maps;

int main(){

	isf::Image test1("test/test01.jpg");

	isf::Image test2 = test1;

	test2.rgbAt<isf::ImageU8Color>(15, 25) = {255, 0, 0};

	isf::Image test3(isf::ImageColorSpace::COLORSPACE_DOUBLE_RGB, 12, 300);

	test2.saveToFile("temp/temptest02.png");

	/**int x, y, channels;
	uint8_t* image = stbi_load("test/test02.jpg", &x, &y, &channels, 0);

	color background = guessBackgroundColor(image, x, y);
	doubleColor backgroundCieColor = rgbToCielab(background);

	int mapWidth = x/4 - 1;
	int mapHeight = y/4 - 1;
	double* map;

	/////////////////////////////////
	// MAP 1 - lightness distance
	/////////////////////////////////

	map = new double[mapWidth * mapHeight];
	for (int i = 0; i <= x - 8; i += 4) {
		for (int j = 0; j <= y - 8; j += 4) {
			color blockColor = averageBlockColor(image, x, y, i, i+8, j, j+8);
			doubleColor blockCieColor = rgbToCielab(blockColor);

			double f1 = abs(backgroundCieColor.ls - blockCieColor.ls);
			map[j/4 * mapWidth + i/4] = f1;
		}
	}
	maps.push_back(map);

	/////////////////////////////////
	// MAP 2 - color distance
	/////////////////////////////////

	map = new double[mapWidth * mapHeight];

	for (int i = 0; i <= x - 8; i += 4) {
		for (int j = 0; j <= y - 8; j += 4) {
			color blockColor = averageBlockColor(image, x, y, i, i+8, j, j+8);
			doubleColor blockCieColor = rgbToCielab(blockColor);

			double deltaA = blockCieColor.as - backgroundCieColor.as;
			double deltaB = blockCieColor.bs - backgroundCieColor.bs;
			double f2 = sqrt((deltaA * deltaA) + (deltaB * deltaB));
			map[j/4 * mapWidth + i/4] = f2;
		}
	}
	maps.push_back(map);

	/////////////////////////////////
	// MAP 3 - contrast
	/////////////////////////////////

	map = new double[mapWidth * mapHeight];

	for (int i = 0; i <= x - 8; i += 4) {
		for (int j = 0; j <= y - 8; j += 4) {

			double l[64];
			int lIndex = 0;
			for (int xx = 0; xx < 8; ++xx) {
				for (int yy = 0; yy < 8; ++yy) {

					int pixelIndex = (j+yy)*x + (i+xx);
					uint8_t rr = image[3*pixelIndex];
					uint8_t gg = image[3*pixelIndex + 1];
					uint8_t bb = image[3*pixelIndex + 2];

					double grayscaleRms = colorToGrayscale(colorToDoubleColor({rr, gg, bb}));

					const double _b = 0.7297;
					const double _k = 0.037644;
					const double _gamma = 2.2;

					double lum = std::pow(_b + _k * grayscaleRms, _gamma);

					l[lIndex++] = lum;
				}
			}

			assert(lIndex == 64);

			double mean = std::accumulate(l, l + 64, 0.0)/64;
			double dispersion = 0.0;

			for (int i = 0; i < 64; ++i) {
				dispersion += (mean - l[i])*(mean - l[i]);
			}

			dispersion /= 64;
			// TODO: here you may also need to sqrt the dispersion
			// Who the fuck knows what is standard deviation

			map[j/4 * mapWidth + i/4] = (mean > 0) ? dispersion/mean : 0;
		}
	}
	maps.push_back(map);

	/////////////////////////////////
	// MAP 4 - sharpness
	/////////////////////////////////

	map = new double[mapWidth * mapHeight];

	for (int i = 0; i <= x - 32; i += 8) {
		for (int j = 0; j <= y - 32; j += 8) {

		}
	}

	maps.push_back(map);

	/////////////////////////////////
	// MAP 5 - edge strength
	/////////////////////////////////

	map = new double[mapWidth * mapHeight];

	for (int i = 0; i <= x - 8; i += 4) {
		for (int j = 0; j <= y - 8; j += 4) {
			// Roberts edge detector
			// Since kernel is 2x2 we're gonna miss one pixel from one edge
			// Hence, the size of the resulting block is 7x7
			double robertsValues[7*7];
			int robertsValuesIndex = 0;

			for (int xx = 0; xx < 7; ++xx) {
				for (int yy = 0; yy < 7; ++yy) {

					int pixelIndex0 = (j+yy)*x + (i+xx);
					int pixelIndex1 = (j+yy)*x + (i+xx+1);
					int pixelIndex2 = (j+yy+1)*x + (i+xx+1);
					int pixelIndex3 = (j+yy+1)*x + (i+xx);
					
					uint8_t rr0 = image[3*pixelIndex0];
					uint8_t gg0 = image[3*pixelIndex0 + 1];
					uint8_t bb0 = image[3*pixelIndex0 + 2];
					uint8_t rr1 = image[3*pixelIndex1];
					uint8_t gg1 = image[3*pixelIndex1 + 1];
					uint8_t bb1 = image[3*pixelIndex1 + 2];
					uint8_t rr2 = image[3*pixelIndex2];
					uint8_t gg2 = image[3*pixelIndex2 + 1];
					uint8_t bb2 = image[3*pixelIndex2 + 2];
					uint8_t rr3 = image[3*pixelIndex3];
					uint8_t gg3 = image[3*pixelIndex3 + 1];
					uint8_t bb3 = image[3*pixelIndex3 + 2];
					// These need to be divided by 3
					// Will be done in final sum
					uint16_t roberts1 = rr0 + gg0 + bb0 - rr2 + gg2 + bb2;
					uint16_t roberts2 = rr1 + gg1 + bb1 - rr3 + gg3 + bb3;
					
					robertsValues[robertsValuesIndex++] = std::sqrt(double(roberts1*roberts1 + roberts2*roberts2))/3.0;
				}
			}

			// Average of Roberts cross applied over this block
			double mean = std::accumulate(robertsValues, robertsValues + (7*7), 0.0)/(7*7);
			map[j/4 * mapWidth + i/4] = mean;
		}
	}

	maps.push_back(map);

	////////////////////////////////////////
	// Normalize, write and deallocate maps
	////////////////////////////////////////

	int mapIndex = 0;

	for (auto& map: maps) {

		double max = *std::max_element(map, map + (mapWidth*mapHeight));

		uint8_t* charMap = new uint8_t[mapWidth * mapHeight];

		for (int i = 0; i < mapWidth * mapHeight; ++i) {
			charMap[i] = (uint8_t)round((map[i] / max) * UINT8_MAX);
		}

		std::string mapPath = "temp/map";
		mapPath += std::to_string(mapIndex++);
		mapPath += std::string(".png");

		stbi_write_png(mapPath.c_str(), mapWidth, mapHeight, 1, charMap, 0);

		delete[] map;
		delete[] charMap;
	}
	
	stbi_image_free(image);*/
	std::cout << "The horse is in the house!" << std::endl;
	return 0;
}

color averageBlockColor(uint8_t* image, int w, int h, int xStart, int xStop, int yStart, int yStop) {

	uint64_t rr = 0;
	uint64_t gg = 0;
	uint64_t bb = 0;

	for (int x = xStart; x < xStop; ++x) {
		for (int y = yStart; y < yStop; ++y) {
			int i = y*w + x;
			rr += image[3*i];
			gg += image[3*i + 1];
			bb += image[3*i + 2];
		}
	}

	int count = (xStop - xStart)*(yStop - yStart);

	return { (uint8_t)(rr/(count)), (uint8_t)(gg/(count)), (uint8_t)(bb/(count)) };
}

color guessBackgroundColor(uint8_t* image, int w, int h) {

	uint64_t rr = 0;
	uint64_t gg = 0;
	uint64_t bb = 0;

	for (int i = 0; i < w*h; ++i) {
		rr += image[3*i];
		gg += image[3*i + 1];
		bb += image[3*i + 2];
	}

	return { (uint8_t)(rr/(w*h)), (uint8_t)(gg/(w*h)), (uint8_t)(bb/(w*h)) };
}

doubleColor colorToDoubleColor(color in) {
	return { (double)in.r/UINT8_MAX, (double)in.g/UINT8_MAX, (double)in.b/UINT8_MAX };
}

color doubleColorToColor(doubleColor in) {
	return { (uint8_t)round(in.r*UINT8_MAX), (uint8_t)round(in.g*UINT8_MAX), (uint8_t)round(in.b*UINT8_MAX) };
}

double linearizeColorComponent(double color) {
	double col = color;

	if (col <= 0.04045) {
		col /= 12.92;
	} else {
		col = pow((col + 0.055)/1.055, 2.4);
	}

	return col;
}

doubleColor linearizeRgb(doubleColor in) {
	return { linearizeColorComponent(in.r), linearizeColorComponent(in.g), linearizeColorComponent(in.b) };
}

doubleColor rgbToCiexyz(doubleColor in) {
	return { 0.412453*in.r + 0.357580*in.g + 0.180423*in.b,
		0.212671*in.r + 0.715160*in.g + 0.072169*in.b,
		0.019334*in.r + 0.119193*in.g + 0.950227*in.b };
}

doubleColor ciexyzToCielab(doubleColor in) {
	double xr = 0.950456;
	double yr = 1;
	double zr = 1.088754;

	auto g = [](double t) {
		if (t > 0.008856) {
			return pow(t, 1.0/3);
		} else {
			return 7.787*t + 16.0/116;
		}
	};

	double l = 116.0 * g(in.y/yr) - 16.0;
	double a = 500.0 * (g(in.x/xr) - g(in.y/yr));
	double b = 200.0 * (g(in.y/yr) - g(in.z/zr));

	return { l, a, b };
}

doubleColor rgbToCielab(color in) {
	return ciexyzToCielab(rgbToCiexyz(linearizeRgb(colorToDoubleColor(in))));
}

double colorToGrayscale(doubleColor in) {
	return in.r * 0.299 + in.g * 0.587 + in.b * 0.114;
}
