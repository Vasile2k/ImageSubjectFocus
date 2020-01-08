#pragma once
#include "ImageColor.hpp"
#include "Image.hpp"

namespace isf {

class ColorConverter final {

public:
	static ImageU8Color averageBlockColor(Image& image, int xStart, int xStop, int yStart, int yStop);
	static ImageU8Color guessBackgroundColor(Image& image, Image* subjectMask = nullptr);
	static ImageDoubleColor colorToDoubleColor(ImageU8Color in);
	static ImageU8Color doubleColorToColor(ImageDoubleColor in);
	static double linearizeColorComponent(double color);
	static ImageDoubleColor linearizeRgb(ImageDoubleColor in);
	static ImageDoubleColor rgbToCiexyz(ImageDoubleColor in);
	static ImageDoubleColor ciexyzToCielab(ImageDoubleColor in);
	static ImageDoubleColor rgbToCielab(ImageU8Color in);
	static double colorToGrayscale(ImageDoubleColor in);

private:
	ColorConverter() = delete;

};

}
