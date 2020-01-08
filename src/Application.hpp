#pragma once

#include "Image.hpp"

namespace isf {

class Application {

public:

	Application();

	int run();

protected:

	std::vector<Image> computeAllMaps(Image& input);

	Image computeLightnessDistanceMap(Image& input);
	Image computeColorDistanceMap(Image& input);
	Image computeContrastMap(Image& input);
	Image computeSharpnessMap(Image& input);
	Image computeEdgeStrengthMap(Image& input);

	void centerWeightMaps(std::vector<Image>& maps);
	void normalizeCenterWeightedMaps(std::vector<Image>& maps);

	std::vector<double> computeClusterDensities(std::vector<Image>& maps);

	void saveMapsToFile(std::vector<Image> maps, std::string name = "feature");

};

}
