#include "pch.h"
#include <iostream>
#include <cstdint>
#include <cassert>
#include "Image.hpp"
#include "ImageColor.hpp"
#include "Debugger.hpp"

#include "Application.hpp"
#include "Test.hpp"

std::vector<double*> maps;

int main(){
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
	return isf::Application().run();
}




