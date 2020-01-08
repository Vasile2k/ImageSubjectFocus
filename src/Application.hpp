#pragma once

#include "Image.hpp"

namespace isf {

class Application {

public:

	Application();

	int run();

protected:

	std::vector<Image> computeAllMaps(Image& input);

	Image computeLightnessDistanceMap(Image& input, Image* subjectMask = nullptr);
	Image computeColorDistanceMap(Image& input, Image* subjectMask = nullptr);
	Image computeContrastMap(Image& input);
	Image computeSharpnessMap(Image& input);
	Image computeEdgeStrengthMap(Image& input);

	void centerWeightMaps(std::vector<Image>& maps, size_t rc = -1, size_t cc = -1);
	void normalizeCenterWeightedMaps(std::vector<Image>& maps);

	std::vector<double> computeClusterDensities(std::vector<Image>& maps);

	Image computeBaselineMap(std::vector<Image>& maps, std::vector<double>& clusterDensities);
	Image thresholdBaselineMap(Image& baselineMap, double thresholdMultiplier = 1.5);

	std::vector<Image> computeAndRefineStage2Maps(Image& input, Image& firstGuessSubject);

	Image computeStage2Map(std::vector<Image>& maps, std::vector<double>& clusterDensities);

	void saveMapsToFile(std::vector<Image> maps, std::string name = "feature");

};

}
