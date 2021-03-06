#include "pch.h"
#include "Debugger.hpp"
#include "Application.hpp"
#include "ColorConverter.hpp"
#include <cmath>
#include <climits>
#include <iostream>

template<class T>
constexpr const T& clamp(const T& v, const T& lo, const T& hi) {
	ISF_ASSERT(!(hi < lo), "Invalid clamp call");
	return (v < lo) ? lo : (hi < v) ? hi : v;
}

isf::Application::Application() {

}

int isf::Application::run() {

	isf::Image test("test/test02.jpg");

	auto maps = computeAllMaps(test);

	saveMapsToFile(maps);

	centerWeightMaps(maps);
	normalizeCenterWeightedMaps(maps);

	auto clusterDensities = computeClusterDensities(maps);

	std::cout << "Cluster densities:" << std::endl;
	for (auto i: clusterDensities) {
		std::cout << "\t" << i << std::endl;
	}

	saveMapsToFile(maps, "featureNorm");

	auto baselineMap = computeBaselineMap(maps, clusterDensities);

	saveMapsToFile({ baselineMap }, "baselineMap");

	auto thresholdedBaselineMap = thresholdBaselineMap(baselineMap);

	saveMapsToFile({ thresholdedBaselineMap }, "thresholdedBaselineMap");

	auto stage2Maps = computeAndRefineStage2Maps(test, thresholdedBaselineMap);

	saveMapsToFile(stage2Maps, "stage2Maps");

	auto stage2Clusters = computeClusterDensities(stage2Maps);
	auto stage2Map = computeStage2Map(stage2Maps, stage2Clusters);

	auto subjectMap = thresholdBaselineMap(stage2Map, 2.0);

	saveMapsToFile({ subjectMap }, "finalSubjectMap");

	auto fx = addFxToSubject(test, subjectMap);

	saveMapsToFile({ fx }, "focused");

	return 0;
}

std::vector<isf::Image> isf::Application::computeAllMaps(Image& input) {
	std::vector<isf::Image> result;

	result.push_back(computeLightnessDistanceMap(input));
	result.push_back(computeColorDistanceMap(input));
	result.push_back(computeContrastMap(input));
	result.push_back(computeSharpnessMap(input));
	result.push_back(computeEdgeStrengthMap(input));

	return result;
}

isf::Image isf::Application::computeLightnessDistanceMap(Image& input, Image* subjectMask) {
	size_t mapWidth = input.getWidth()/4 - 1;
	size_t mapHeight = input.getHeight()/4 - 1;

	ImageU8Color background = ColorConverter::guessBackgroundColor(input, subjectMask);
	ImageDoubleColor backgroundCieColor = ColorConverter::rgbToCielab(background);

	Image map(ImageColorSpace::COLORSPACE_DOUBLE_UNNORMALIZED, mapWidth, mapHeight);

	for (int x = 0; x <= input.getWidth() - 8; x += 4) {
		for (int y = 0; y <= input.getHeight() - 8; y += 4) {
			ImageU8Color blockColor = ColorConverter::averageBlockColor(input, x, x+8, y, y+8);
			ImageDoubleColor blockCieColor = ColorConverter::rgbToCielab(blockColor);

			map.grayscaleAt<double>(x/4, y/4) = std::abs(backgroundCieColor.ls - blockCieColor.ls);
		}
	}
	return map;
}

isf::Image isf::Application::computeColorDistanceMap(Image& input, Image* subjectMask) {
	size_t mapWidth = input.getWidth()/4 - 1;
	size_t mapHeight = input.getHeight()/4 - 1;

	ImageU8Color background = ColorConverter::guessBackgroundColor(input, subjectMask);
	ImageDoubleColor backgroundCieColor = ColorConverter::rgbToCielab(background);

	Image map(ImageColorSpace::COLORSPACE_DOUBLE_UNNORMALIZED, mapWidth, mapHeight);

	for (int x = 0; x <= input.getWidth() - 8; x += 4) {
		for (int y = 0; y <= input.getHeight() - 8; y += 4) {
			ImageU8Color blockColor = ColorConverter::averageBlockColor(input, x, x+8, y, y+8);
			ImageDoubleColor blockCieColor = ColorConverter::rgbToCielab(blockColor);

			double deltaA = blockCieColor.as - backgroundCieColor.as;
			double deltaB = blockCieColor.bs - backgroundCieColor.bs;
			map.grayscaleAt<double>(x/4, y/4) = std::sqrt((deltaA * deltaA) + (deltaB * deltaB));
		}
	}
	return map;
}

isf::Image isf::Application::computeContrastMap(Image& input) {
	size_t mapWidth = input.getWidth()/4 - 1;
	size_t mapHeight = input.getHeight()/4 - 1;

	Image map(ImageColorSpace::COLORSPACE_DOUBLE_UNNORMALIZED, mapWidth, mapHeight);

	for (int x = 0; x <= input.getWidth() - 8; x += 4) {
		for (int y = 0; y <= input.getHeight() - 8; y += 4) {

			std::vector<double> l;
			for (int xx = 0; xx < 8; ++xx) {
				for (int yy = 0; yy < 8; ++yy) {

					auto color = input.rgbAt<ImageU8Color>((size_t)x + xx, (size_t)y + yy);

					double grayscaleRms = ColorConverter::colorToGrayscale(ColorConverter::colorToDoubleColor(color));

					const double _b = 0.7297;
					const double _k = 0.037644;
					const double _gamma = 2.2;

					double lum = std::pow(_b + _k * grayscaleRms, _gamma);

					l.push_back(lum);
				}
			}

			ISF_ASSERT(l.size() == 64, "Element count not as expected");

			double mean = std::accumulate(l.begin(), l.end(), 0.0)/64;
			double dispersion = 0.0;

			for (int i = 0; i < 64; ++i) {
				dispersion += (mean - l[i])*(mean - l[i]);
			}

			dispersion /= 64;
			// Here you may also need to sqrt the dispersion
			// Who the fuck knows what is standard deviation?
			dispersion = std::sqrt(dispersion);
			map.grayscaleAt<double>(x/4, y/4) = (mean > 0) ? dispersion/mean : 0;
		}
	}
	return map;
}

isf::Image isf::Application::computeSharpnessMap(Image& input) {
	size_t mapWidth = input.getWidth()/4 - 1;
	size_t mapHeight = input.getHeight()/4 - 1;

	Image map(ImageColorSpace::COLORSPACE_DOUBLE_UNNORMALIZED, mapWidth, mapHeight);

	Image localVariationMap(ImageColorSpace::COLORSPACE_DOUBLE_UNNORMALIZED, input.getWidth()/8, input.getHeight()/8);

	for (int x = 0; x < localVariationMap.getWidth(); ++x) {
		for (int y = 0; y < localVariationMap.getHeight(); ++y) {
			// Take a 2x2 block in this region, so 7x7 blocks in 8x8 pixels
			std::vector<double> vEpsilons;
			for (int blockX = 0; blockX < 7; ++blockX) {
				for (int blockY = 0; blockY < 7; ++blockY) {
					double vEpsilon = 0;

					auto color1 = input.rgbAt<ImageU8Color>((size_t)x*8 + blockX,     (size_t)y*8 + blockY);
					auto color2 = input.rgbAt<ImageU8Color>((size_t)x*8 + blockX + 1, (size_t)y*8 + blockY);
					auto color3 = input.rgbAt<ImageU8Color>((size_t)x*8 + blockX + 1, (size_t)y*8 + blockY + 1);
					auto color4 = input.rgbAt<ImageU8Color>((size_t)x*8 + blockX,     (size_t)y*8 + blockY + 1);
					double grayscaleRms1 = ColorConverter::colorToGrayscale(ColorConverter::colorToDoubleColor(color1));
					double grayscaleRms2 = ColorConverter::colorToGrayscale(ColorConverter::colorToDoubleColor(color2));
					double grayscaleRms3 = ColorConverter::colorToGrayscale(ColorConverter::colorToDoubleColor(color3));
					double grayscaleRms4 = ColorConverter::colorToGrayscale(ColorConverter::colorToDoubleColor(color4));

					double grays[] = { grayscaleRms1, grayscaleRms2, grayscaleRms3, grayscaleRms4 };

					for (int i = 0; i < sizeof(grays)/sizeof(grays[0]); ++i) {
						for (int j = 0; j < sizeof(grays)/sizeof(grays[0]); ++j) {
							vEpsilon += std::abs(grays[i] - grays[j]);
						}
					}

					vEpsilons.push_back(vEpsilon);
				}
			}
			localVariationMap.grayscaleAt<double>(x, y) = *std::max_element(vEpsilons.begin(), vEpsilons.end());
		}
	}

	for (int x = 0; x < mapWidth; ++x) {
		for (int y = 0; y < mapHeight; ++y){
			map.grayscaleAt<double>(x, y) = localVariationMap.grayscaleAt<double>(x/2, y/2);
		}
	}

	return map;
}

isf::Image isf::Application::computeEdgeStrengthMap(Image& input) {
	size_t mapWidth = input.getWidth()/4 - 1;
	size_t mapHeight = input.getHeight()/4 - 1;

	Image map(ImageColorSpace::COLORSPACE_DOUBLE_UNNORMALIZED, mapWidth, mapHeight);

	for (int x = 0; x <= input.getWidth() - 8; x += 4) {
		for (int y = 0; y <= input.getHeight() - 8; y += 4) {
			// Roberts edge detector
			// Since kernel is 2x2 we're gonna miss one pixel from one edge
			// Hence, the size of the resulting block is 7x7
			std::vector<double> robertsValues;
			int robertsValuesIndex = 0;

			for (int xx = 0; xx < 7; ++xx) {
				for (int yy = 0; yy < 7; ++yy) {
					auto pixel0 = input.rgbAt<ImageU8Color>((size_t)x + xx,     (size_t)y + yy);
					auto pixel1 = input.rgbAt<ImageU8Color>((size_t)x + xx + 1, (size_t)y + yy);
					auto pixel2 = input.rgbAt<ImageU8Color>((size_t)x + xx + 1, (size_t)y + yy + 1);
					auto pixel3 = input.rgbAt<ImageU8Color>((size_t)x + xx,     (size_t)y + yy + 1);

					// These need to be divided by 3
					// Will be done in final sum
					uint16_t roberts1 = (pixel0.r + pixel0.g + pixel0.b) - (pixel2.r + pixel2.g + pixel2.b);
					uint16_t roberts2 = (pixel1.r + pixel1.g + pixel1.b) - (pixel3.r + pixel3.g + pixel3.b);

					robertsValues.push_back(std::sqrt(double(((uint32_t)roberts1*roberts1) + ((uint32_t)roberts2*roberts2)))/3.0);
				}
			}

			ISF_ASSERT(robertsValues.size() == 7*7, "Invalid number of elements after Roberts edge")

			// Average of Roberts cross applied over this block
			double mean = std::accumulate(robertsValues.begin(), robertsValues.end(), 0.0)/robertsValues.size();
			map.grayscaleAt<double>(x/4, y/4) = mean;
		}
	}
	return map;
}

void isf::Application::centerWeightMaps(std::vector<Image>& maps, size_t rc, size_t cc) {
	// column center
	cc = cc == -1 ? maps[0].getWidth()/2 : cc;
	// row center
	rc = rc == -1 ? maps[0].getHeight()/2 : rc;

	for (int c = 0; c < maps[0].getWidth(); ++c) {
		for (int r = 0; r < maps[0].getHeight(); ++r) {
			double fc = 1 - std::sqrt((r - rc)*(r - rc) + (c - cc)*(c - cc))/std::sqrt(rc*rc + cc*cc);
			for (auto& map: maps) {
				map.grayscaleAt<double>(c, r) *= fc;
			}
		}
	}
}

void isf::Application::normalizeCenterWeightedMaps(std::vector<Image>& maps) {
	for (auto& map: maps) {

		double mapMin = DBL_MAX;
		double mapMax = DBL_MIN;

		for (int x = 0; x < map.getWidth(); ++x) {
			for (int y = 0; y < map.getHeight(); ++y) {
				if (map.grayscaleAt<double>(x, y) < mapMin) {
					mapMin = map.grayscaleAt<double>(x, y);
				}
				if (map.grayscaleAt<double>(x, y) > mapMax) {
					mapMax = map.grayscaleAt<double>(x, y);
				}
			}
		}

		for (int x = 0; x < map.getWidth(); ++x) {
			for (int y = 0; y < map.getHeight(); ++y) {
				map.grayscaleAt<double>(x, y) = (map.grayscaleAt<double>(x, y) - mapMin)/(mapMax - mapMin);
			}
		}
	}
}

std::vector<double> isf::Application::computeClusterDensities(std::vector<Image>& maps) {
	std::vector<double> results;

	for (auto& map: maps) {
		double alpha = 0;

		int card = 0;

		// centroid computation for points with value greater than 0.5
		size_t c0 = 0;
		size_t r0 = 0;

		for (int c = 0; c < map.getWidth(); ++c) {
			for (int r = 0; r < map.getHeight(); ++r) {
				if (map.grayscaleAt<double>(c, r) > 0.5) {
					c0 += c;
					r0 += r;
					++card;
				}
			}
		}

		c0 /= card;
		r0 /= card;

		int card2 = 0;

		for (int c = 0; c < map.getWidth(); ++c) {
			for (int r = 0; r < map.getHeight(); ++r) {
				if (map.grayscaleAt<double>(c, r) > 0.5) {
					alpha += std::sqrt((r0 - r)*(r0 - r) + (c0 - c)*(c0 - c));
					++card2;
				}
			}
		}

		ISF_ASSERT(card == card2, "Some kind of shit went terribly wrong here");

		double beta = 1.25;
		// card is the cardinal of the set of points with value greater than 0.5
		// just so you know, even if you don't care
		// why the fuck am I writing this?
		alpha /= std::pow(double(card), beta);

		results.push_back(alpha);
	}

	return results;
}

isf::Image isf::Application::computeBaselineMap(std::vector<Image>& maps, std::vector<double>& clusterDensities) {
	std::vector<double> sorted = clusterDensities;
	// Sort only first 3 values 'cause that's what I care about
	std::nth_element(sorted.begin(), sorted.begin(), sorted.end());
	std::nth_element(sorted.begin() + 1, sorted.begin() + 1, sorted.end());
	std::nth_element(sorted.begin() + 2, sorted.begin() + 2, sorted.end());

	double weights[] = { 1.0, 2.0/3, 1.0/3 };
	// no indexOf function so let's do it the dirty way
	int64_t indices[] = {
		std::distance(clusterDensities.begin(), std::find(clusterDensities.begin(), clusterDensities.end(), sorted[0])),
		std::distance(clusterDensities.begin(), std::find(clusterDensities.begin(), clusterDensities.end(), sorted[1])),
		std::distance(clusterDensities.begin(), std::find(clusterDensities.begin(), clusterDensities.end(), sorted[2])),
	};

	Image baseline(ImageColorSpace::COLORSPACE_DOUBLE_UNNORMALIZED, maps[0].getWidth(), maps[0].getHeight());

	for (int x = 0; x < baseline.getWidth(); x++) {
		for (int y = 0; y < baseline.getHeight(); y++) {
			baseline.grayscaleAt<double>(x, y) =
				(maps[indices[0]].grayscaleAt<double>(x, y) * weights[0] +
				maps[indices[1]].grayscaleAt<double>(x, y) * weights[1] +
				maps[indices[2]].grayscaleAt<double>(x, y) * weights[2])/
				 (weights[0] + weights[1] + weights[2]);
		}
	}

	std::vector<Image> baselineContainer = { baseline };
	normalizeCenterWeightedMaps(baselineContainer);

	return baselineContainer[0];
}

isf::Image isf::Application::thresholdBaselineMap(Image& baselineMap, double thresholdMultiplier) {
	double sum = 0;

	for (int x = 0; x < baselineMap.getWidth(); x++) {
		for (int y = 0; y < baselineMap.getHeight(); y++) {
			sum += baselineMap.grayscaleAt<double>(x, y);
		}
	}

	double mean = sum/(baselineMap.getWidth() * baselineMap.getHeight());
	double threshold = thresholdMultiplier * mean;

	Image thresholdedMap(ImageColorSpace::COLORSPACE_DOUBLE_GRAYSCALE, baselineMap.getWidth(), baselineMap.getHeight());

	for (int x = 0; x < baselineMap.getWidth(); x++) {
		for (int y = 0; y < baselineMap.getHeight(); y++) {
			thresholdedMap.grayscaleAt<double>(x, y) = baselineMap.grayscaleAt<double>(x, y) >= threshold ? 1.0 : 0.0;
		}
	}
	return thresholdedMap;
}

std::vector<isf::Image> isf::Application::computeAndRefineStage2Maps(Image& input, Image& firstGuessSubject) {
	std::vector<isf::Image> result;

	int card = 0;

	// centroid computation
	size_t c0 = 0;
	size_t r0 = 0;

	for (int c = 0; c < firstGuessSubject.getWidth(); ++c) {
		for (int r = 0; r < firstGuessSubject.getHeight(); ++r) {
			if (firstGuessSubject.grayscaleAt<double>(c, r)) {
				c0 += c;
				r0 += r;
				++card;
			}
		}
	}

	c0 /= card;
	r0 /= card;

	result.push_back(computeLightnessDistanceMap(input, &firstGuessSubject));
	result.push_back(computeColorDistanceMap(input, &firstGuessSubject));
	result.push_back(computeContrastMap(input));
	result.push_back(computeSharpnessMap(input));
	result.push_back(computeEdgeStrengthMap(input));

	centerWeightMaps(result, r0, c0);

	normalizeCenterWeightedMaps(result);

	return result;
}

isf::Image isf::Application::computeStage2Map(std::vector<Image>& maps, std::vector<double>& clusterDensities) {
	std::vector<double> sorted = clusterDensities;
	// Sort only first 2 values and the last one
	std::nth_element(sorted.begin(), sorted.begin(), sorted.end());
	std::nth_element(sorted.begin() + 1, sorted.begin() + 1, sorted.end());
	std::nth_element(sorted.begin() + 2, sorted.begin() + 4, sorted.end());

	int64_t indices[] = {
		std::distance(clusterDensities.begin(), std::find(clusterDensities.begin(), clusterDensities.end(), sorted[0])),
		std::distance(clusterDensities.begin(), std::find(clusterDensities.begin(), clusterDensities.end(), sorted[1])),
	};

	Image baseline(ImageColorSpace::COLORSPACE_DOUBLE_UNNORMALIZED, maps[0].getWidth(), maps[0].getHeight());

	for (int x = 0; x < baseline.getWidth(); x++) {
		for (int y = 0; y < baseline.getHeight(); y++) {

			double weights[] = { 1.0, (sorted[4] - clusterDensities[indices[1]])/(sorted[4] - sorted[0])};

			baseline.grayscaleAt<double>(x, y) =
				(maps[indices[0]].grayscaleAt<double>(x, y) * weights[0] +
				 maps[indices[1]].grayscaleAt<double>(x, y) * weights[1])/
				 (weights[0] + weights[1]);
		}
	}

	std::vector<Image> baselineContainer = { baseline };
	normalizeCenterWeightedMaps(baselineContainer);

	return baselineContainer[0];
}

isf::Image isf::Application::addFxToSubject(Image& input, Image& subjectMask) {
	Image bigMaskBlurred(ImageColorSpace::COLORSPACE_DOUBLE_GRAYSCALE, input.getWidth(), input.getHeight());

	// Make a blurry mask for smooth transition
	for (int x = 0; x < bigMaskBlurred.getWidth(); ++x) {
		for (int y = 0; y < bigMaskBlurred.getHeight(); ++y) {
			// kernel size of 5x5
			double sum = 0;
			for (int i = x - 2; i <= x + 2; ++i) {
				for (int j = y - 2; j <= y + 2; ++j) {
					int xx = clamp(i/4, 0, (int)subjectMask.getWidth() - 1);
					int yy = clamp(j/4, 0, (int)subjectMask.getHeight() - 1);
					sum += subjectMask.grayscaleAt<double>(xx, yy);
				}
			}
			bigMaskBlurred.grayscaleAt<double>(x, y) = sum/25.0;
		}
	}

	Image subjectBlurred(ImageColorSpace::COLORSPACE_U8_RGB, input.getWidth(), input.getHeight());
	// Make a blurry image for the background
	// Can be optimized to blur only background but who gives a shit?
	for (int x = 0; x < subjectBlurred.getWidth(); ++x) {
		for (int y = 0; y < subjectBlurred.getHeight(); ++y) {
			// kernel size of 7x7
			int ks = 7; // ks for kernelSize
			ImageDoubleColor col = { 0 };
			for (int i = x - ks/2; i <= x + ks/2; ++i) {
				for (int j = y - ks/2; j <= y + ks/2; ++j) {
					int xx = clamp(i, 0, (int)input.getWidth() - 1);
					int yy = clamp(j, 0, (int)input.getHeight() - 1);
					auto color = input.rgbAt<ImageU8Color>(xx, yy).toDoubleColor();
					// Remember when you were to lazy to overload operators?
					col.r += color.r;
					col.g += color.g;
					col.b += color.b;
				}
			}
			col.r /= ks*ks;
			col.g /= ks*ks;
			col.b /= ks*ks;

			subjectBlurred.rgbAt<ImageU8Color>(x, y) = col.toU8Color();
		}
	}

	// Now interpolate the original subject into the blurred picture
	for (int x = 0; x < subjectBlurred.getWidth(); ++x) {
		for (int y = 0; y < subjectBlurred.getHeight(); ++y) {

			double desaturationFactor = 0.4;

			if (bigMaskBlurred.grayscaleAt<double>(x, y) > 0) {
				auto t = bigMaskBlurred.grayscaleAt<double>(x, y);

				auto col = subjectBlurred.rgbAt<ImageU8Color>(x, y).toDoubleColor();
				auto scol = input.rgbAt<ImageU8Color>(x, y).toDoubleColor();
				
				// desaturate the color
				double average = (col.r + col.g + col.b)/3;
				col.r = average * desaturationFactor + col.r * (1 - desaturationFactor);
				col.g = average * desaturationFactor + col.g * (1 - desaturationFactor);
				col.b = average * desaturationFactor + col.b * (1 - desaturationFactor);

				ImageDoubleColor interpolatedColor = {
					t * scol.r + (1-t) * col.r,
					t * scol.g + (1-t) * col.g,
					t * scol.b + (1-t) * col.b,
				};

				subjectBlurred.rgbAt<ImageU8Color>(x, y) = interpolatedColor.toU8Color();
			} else {
				auto col = subjectBlurred.rgbAt<ImageU8Color>(x, y).toDoubleColor();
				// just desaturate the color
				double average = (col.r + col.g + col.b)/3;
				col.r = average * desaturationFactor + col.r * (1 - desaturationFactor);
				col.g = average * desaturationFactor + col.g * (1 - desaturationFactor);
				col.b = average * desaturationFactor + col.b * (1 - desaturationFactor);
				subjectBlurred.rgbAt<ImageU8Color>(x, y) = col.toU8Color();
			}
		}
	}

	return subjectBlurred;
}

void isf::Application::saveMapsToFile(std::vector<Image> maps, std::string name) {
	int mapIndex = 0;

	for (auto& map: maps) {

		auto printableMap = map.isWritableToFile() ? map : map.normalizeAndConvertToViewable();

		std::string mapPath = "temp/" + name;
		mapPath += std::to_string(mapIndex++);
		mapPath += std::string(".png");

		printableMap.saveToFile(mapPath);
	}
}
