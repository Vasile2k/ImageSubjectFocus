#include "pch.h"
#include <math.h>
#include <cstdint>

#define M1 w
#define M2 h

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

doubleColor rgbToCielab(color in);
color guessBackgroundColor(uint8_t* image, int w, int h);
color averageBlockColor(uint8_t* image, int w, int h, int xStart, int xStop, int yStart, int yStop);


/////******/////
double rgbToGrayScale(color in);
double EachBlockLuminanceShit(uint8_t *image, int w, int h, int xStart, int xStop, int yStart, int yStop);


//////-------//////
uint8_t * RobertsEdgeFilter(uint8_t *image, int w, int h);
uint8_t * RobertsEdgeFilters(uint8_t *image, int w, int h);
double BlockEdgeStrength(uint8_t *image, int w, int h, int xStart, int xStop, int yStart, int yStop);
doubleColor BlockEdgeStrengths(uint8_t *image, int w, int h, int xStart, int xStop, int yStart, int yStop);
uint8_t *GrayScaled(uint8_t *image, int w, int h);
////


double CenterWeightCalc(int w, int h, int xStart, int yStart);
std::vector<uint8_t *> MapsCenterWeight(int w, int h, std::vector<uint8_t*> maps, uint8_t *centerweightmap);
std::vector<uint8_t *> MapsCenterWeightNormalized(uint8_t *image, int w, int h, std::vector<uint8_t*> maps);


int main(){
	
	int x, y, channels;
	uint8_t* image = stbi_load("test/test03.jpg", &x, &y, &channels, 0);
	uint8_t * edged = RobertsEdgeFilter(image, x, y); /////// ------- /////// pe grayscale
	// ignore it : uint8_t * edged = RobertsEdgeFilters(image, x, y); /////// ------- /////// pe rgb

	color background = guessBackgroundColor(image, x, y);
	doubleColor backgroundCieColor = rgbToCielab(background);

	int mapWidth = x/4 - 1;
	int mapHeight = y/4 - 1;
	double* map1 = new double[mapWidth * mapHeight];
	double* map2 = new double[mapWidth * mapHeight];
	double* map3 = new double[mapWidth * mapHeight]; ////****////
	double* map4 = new double[mapWidth * mapHeight]; /////----/////
	// ignore it : double* map4 = new double[3*mapWidth * mapHeight]; ////-----//// pe rgb
	double* centerWeightMap = new double[mapWidth * mapHeight];
	
	for (int i = 0; i < x - 8; i += 4) {
		for (int j = 0; j < y - 8; j += 4) {
			color blockColor = averageBlockColor(image, x, y, i, i+8, j, j+8);
			doubleColor blockCieColor = rgbToCielab(blockColor);

			double f1 = abs(backgroundCieColor.ls - blockCieColor.ls);
			double deltaA = blockCieColor.as - backgroundCieColor.as;
			double deltaB = blockCieColor.bs - backgroundCieColor.bs;
			double f2 = sqrt((deltaA * deltaA) + (deltaB * deltaB));
			double f3 = EachBlockLuminanceShit(image, x, y, i, i + 8, j, j + 8); ////****/////
			

			map1[j/4 * mapWidth + i/4] = f1;
			map2[j/4 * mapWidth + i/4] = f2;
			map3[j/4 * mapWidth + i/4] = f3; ////*****/////
			map4[j/4 * mapWidth + i/4] = BlockEdgeStrength(edged, x, y, i, i + 8, j, j + 8); ////----////
			centerWeightMap[j/4 * mapWidth + i/4] = CenterWeightCalc(x,y,i, j);

			
			/*ignore it : 
			doubleColor edgestrength = BlockEdgeStrengths(edged, x, y, i, i + 8, j, j + 8); ////----////
			map4[3 * (j / 4 * mapWidth + i / 4)] = (uint8_t)edgestrength.r;
			map4[3 * (j / 4 * mapWidth + i / 4) + 1] = (uint8_t)edgestrength.g;
			map4[3 * (j / 4 * mapWidth + i / 4) + 2] = (uint8_t)edgestrength.b;*/
			
		}
	}

	double max1 = *std::max_element(map1, map1 + (mapWidth*mapHeight));
	double max2 = *std::max_element(map2, map2 + (mapWidth*mapHeight));
	double max3 = *std::max_element(map3, map3 + (mapWidth*mapHeight)); //////*****///////
	double max4 = *std::max_element(map4, map4 + (mapWidth*mapHeight)); /////------///////
	double centermax = *std::max_element(centerWeightMap, centerWeightMap + (mapWidth * mapHeight));

	uint8_t* charMap1 = new uint8_t[mapWidth * mapHeight];
	uint8_t* charMap2 = new uint8_t[mapWidth * mapHeight];
	uint8_t* charMap3 = new uint8_t[mapWidth * mapHeight]; ////******/////
	uint8_t* charMap4 = new uint8_t[mapWidth * mapHeight]; ////******/////
	uint8_t* centerweightmap = new uint8_t[mapWidth * mapHeight];

	for (int i = 0; i < mapWidth * mapHeight; ++i) {
		charMap1[i] = (uint8_t)round((map1[i] / max1) * UINT8_MAX);
		charMap2[i] = (uint8_t)round((map2[i] / max2) * UINT8_MAX);
		charMap3[i] = (uint8_t)round((map3[i] / max3) * UINT8_MAX); //////*******///////
		charMap4[i] = (uint8_t)round((map4[i] / max4) * UINT8_MAX); /////------/////
		centerweightmap[i] = (uint8_t)round((centerWeightMap[i] / centermax) * UINT8_MAX);
	}
	std::vector<uint8_t*> maps = {
		charMap1,
		charMap2,
		charMap3,
		charMap4
	};

	std::vector<uint8_t*> mapscw = MapsCenterWeight(mapWidth,mapHeight,maps,centerweightmap);
	std::vector<uint8_t*> mapscwn = MapsCenterWeightNormalized(image, mapWidth, mapHeight, maps);
	
	stbi_write_png("temp/map4.png", mapWidth, mapHeight, 1, charMap1, 0);
	stbi_write_png("temp/map5.png", mapWidth, mapHeight, 1, charMap2, 0);
	stbi_write_png("temp/map6.png", mapWidth, mapHeight, 1, charMap3, 0);//////*******////////
	stbi_write_png("temp/map7.png", mapWidth,mapHeight, 1, charMap4, 0);//////------///////
	//ignore it : stbi_write_png("temp/map10.png", x, y, 1, edged, 0);
	stbi_write_png("temp/centerwmap.png", mapWidth, mapHeight, 1, centerweightmap, 0);

	stbi_write_png("temp/centermapweight/initial/map1.png",mapWidth,mapHeight,1,mapscw[0],0);
	stbi_write_png("temp/centermapweight/initial/map2.png",mapWidth,mapHeight,1,mapscw[1],0);
	stbi_write_png("temp/centermapweight/initial/map3.png",mapWidth,mapHeight,1,mapscw[2],0);
	stbi_write_png("temp/centermapweight/initial/map4.png",mapWidth,mapHeight,1,mapscw[3],0);


	stbi_write_png("temp/centermapweight/normalized/map1.png", mapWidth,mapHeight, 1, mapscwn[0], 0);
	stbi_write_png("temp/centermapweight/normalized/map2.png", mapWidth,mapHeight, 1, mapscwn[1], 0);
	stbi_write_png("temp/centermapweight/normalized/map3.png", mapWidth,mapHeight, 1, mapscwn[2], 0);
	stbi_write_png("temp/centermapweight/normalized/map4.png", mapWidth,mapHeight, 1, mapscwn[3], 0);

	//std::vector<uint8_t*> maps = {
	//	charMap1,
	//	charMap2,
	//	charMap3,
	//	charMap4
	//};


	stbi_image_free(image);
	delete[] map1;
	delete[] charMap1;
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


//// ***** ////

double rgbToGrayScale(color in) {
	return 0.299 * in.r + 0.587 * in.g + 0.114 * in.b;
}

double EachBlockLuminanceShit(uint8_t *image, int w, int h, int xStart, int xStop, int yStart, int yStop) {
	double b = 0.7297;
	double k = 0.037644;
	double gamma = 2.2;
	double mean = 0;
	std::vector<double> grayscales;
	for (int x = xStart; x < xStop; ++x) {
		for (int y = yStart; y < yStop; ++y) {
			int i = w*y + x;
			double xg = rgbToGrayScale({ image[3 * i],image[3 * i + 1],image[3 * i + 2] });
			grayscales.push_back(xg);
			mean += std::pow((b + k * xg), gamma);
		}
	}
	mean /= ((yStop - yStart) * (xStop - xStart));
	double standarddev = 0;
	for (int i = 0; i < (yStop - yStart) * (xStop - xStart); i++) {
		double l = std::pow((b + k * grayscales.at(i)), gamma);
		standarddev += pow((l - mean), 2);
	}
	standarddev /= ((yStop - yStart) * (xStop - xStart) - 2);
	standarddev = sqrt(standarddev);
	

	auto f = [](double disp,double mean) {
		if (mean > 0) {
			return disp/mean;
		}
		else {
			return 0.0;
		}
	};

	return f(standarddev, mean);
}


//// ------- //////

uint8_t *GrayScaled(uint8_t *image, int w, int h) {
	uint8_t *result = new uint8_t[w*h];
	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			int i = y*w + x;
			result[y*w + x] = (uint8_t)rgbToGrayScale({ image[3 * i],image[3 * i + 1],image[3 * i + 2] });
		}
	}
	return result;
}

uint8_t * RobertsEdgeFilter(uint8_t *image, int w, int h) {
	uint8_t *result = new uint8_t[w * h];
	uint8_t *grayscaled = GrayScaled(image, w, h);
	int8_t gx;
	int8_t gy;
	uint8_t right;
	uint8_t bottom;
	uint8_t rightbottom;
	for (int x = 0; x < w; x++) {
		for (int y = 0; y < h; y++) {
			if (x == w - 1) {
				rightbottom = 0;
				right = 0;
			}
			if (y == h - 1) {
				rightbottom = 0;
				bottom = 0;
			}
			else {
				right = grayscaled[y*w + x + 1];
				bottom = grayscaled[(y + 1)*w + x];
				rightbottom = grayscaled[(y + 1)*w + x + 1];

			}
			gx = rightbottom - grayscaled[y*w + x];
			gy = bottom - right;

			result[y*w + x] = (uint8_t)sqrt(pow(gx, 2) + pow(gy, 2));
		}
	}

	return result;
}

double BlockEdgeStrength(uint8_t *image, int w, int h, int xStart, int xStop, int yStart, int yStop) {
	int  media = 0;
	for (int y = yStart; y < yStop; ++y) {
		for (int x = xStart; x < xStop; ++x) {
			media += image[y*w + x];
		}
	}
	
	auto f = [](int media) {
		return media / pow(8, 2);
	};

	return f(media);
}



//// ignore it!

doubleColor BlockEdgeStrengths(uint8_t *image, int w, int h, int xStart, int xStop, int yStart, int yStop) {
	int rr = 0, gg = 0, bb = 0;
	for (int y = yStart; y < yStop; ++y) {
		for (int x = xStart; x < xStop; ++x) {
			int i = w*y + x;
			rr += image[3 * i];
			gg += image[3 * i + 1];
			bb += image[3 * i + 2];
		}
	}


	auto f = [](int media) {
		return media / pow(8, 2);
	};

	return{ f(rr),f(gg),f(bb) };
}

uint8_t * RobertsEdgeFilters(uint8_t *image, int w, int h) {
	uint8_t *result = new uint8_t[3 * w * h];
	color gx;
	color gy;
	color right;
	color bottom;
	color rightbottom;
	for (int x = 0; x < w; x++) {
		for (int y = 0; y < h; y++) {
			if (x == w - 1) {
				rightbottom = color{ 0,0,0 };
				right = color{ 0,0,0 };
			}
			if (y == h - 1) {
				rightbottom = color{ 0,0,0 };
				bottom = color{ 0,0,0 };
			}
			else {
				right = color{ image[3 * (y*w + x + 1)] ,image[3 * (y*w + x + 1) + 1],image[3 * (y*w + x + 1) + 2] };
				bottom = color{ image[3 * ((y + 1)*w + x)] ,image[3 * ((y + 1)*w + x) + 1] ,image[3 * ((y + 1)*w + x) + 2] };
				rightbottom = color{ image[3 * ((y + 1)*w + x + 1)] ,image[3 * ((y + 1)*w + x + 1) + 1] ,image[3 * ((y + 1)*w + x + 1) + 2] };
			}

			int i = y*w + x;
			gx = color{ (uint8_t)abs(rightbottom.r - (int)image[3 * i]),(uint8_t)abs(rightbottom.g - (int)image[3 * i + 1]),(uint8_t)abs(rightbottom.b - (int)image[3 * i + 2]) };
			gy = color{ (uint8_t)abs(bottom.r - right.r),(uint8_t)abs(bottom.g - right.g),(uint8_t)abs(bottom.b - right.b) };
			result[3 * i] = (uint8_t)sqrt(pow(gx.r, 2) + pow(gy.r, 2));
			result[3 * i + 1] = (uint8_t)sqrt(pow(gx.g, 2) + pow(gy.g, 2));
			result[3 * i + 2] = (uint8_t)sqrt(pow(gx.b, 2) + pow(gy.b, 2));
		}
	}
	return result;
}






//2131313213//////////////

/*
uint8_t * Saturation(const uint8_t & image,int w,int h) {
	uint8_t *result = new uint8_t[w * h];
}*/

double CenterWeightCalc(int w,int h,int xStart, int yStart) {
	int rc = M2 / 2;
	int cc = M1 / 2;
	return 1 - sqrt(pow(yStart - rc,2) + pow(xStart - cc,2)) / sqrt(pow(M1/2,2) + pow(M2/2,2));
}


std::vector<uint8_t *> MapsCenterWeight(int w,int h,std::vector<uint8_t*> maps,uint8_t *centerweightmap){
	std::vector<uint8_t*> centered_maps;


	for (std::vector<uint8_t*>::iterator it = maps.begin(); it != maps.end(); it++) {
		uint8_t *aux = new uint8_t[w * h];
		uint8_t *currentmap = *it;
		for (int i = 0; i < w * h; i++) {
			aux[i] = centerweightmap[i] + currentmap[i];
		}
		centered_maps.push_back(aux);
	}
	return centered_maps;
}



std::vector<uint8_t *> MapsCenterWeightNormalized(uint8_t *image,int w, int h, std::vector<uint8_t*> maps) {
	std::vector<uint8_t*> centered_maps_normalized;
	uint8_t *aux = new uint8_t[w * h];
	for (int i = 0; i < w * h; i++) {
		aux[i] = 0;
		for (int j = 0; j < 4; j++) {
			aux[i] += maps[j][i];
		}
	}
	for (int i = 0; i < w * h; i++) {
		printf("%d\n", aux[i]);
		aux[i] = aux[i] / 4;
	}
	centered_maps_normalized.push_back(aux);
	
	centered_maps_normalized.push_back(aux);
	
	centered_maps_normalized.push_back(aux);
	
	centered_maps_normalized.push_back(aux);
	
	centered_maps_normalized.push_back(aux);
	/*uint8_t *grayscaled = GrayScaled(image, w, h);
	for (std::vector<uint8_t*>::iterator it = maps.begin(); it != maps.end(); it++) {
		uint8_t *aux = new uint8_t[w * h];
		uint8_t *currentmap = *it;
		int mapx = 0;
		int mapy = 0;
		for (int y = 0; y <= h-8; y+=8, mapy++) {
			for (int x = 0; x <= w-8; x+=8,mapx++) {
				for (int yy = y; yy <= y + 8; yy++) {
					for (int xx = x; xx <= x + 8; xx++) {
						aux[yy*w + xx] = currentmap[mapy * 8 + mapx];
					}
				}
			}
		}
		centered_maps_normalized.push_back(aux);
	}*/
	return centered_maps_normalized;
}

