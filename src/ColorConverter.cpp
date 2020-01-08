#include "pch.h"
#include "ColorConverter.hpp"
#include <cmath>

isf::ImageU8Color isf::ColorConverter::averageBlockColor(Image& image, int xStart, int xStop, int yStart, int yStop) {
	uint64_t rr = 0;
	uint64_t gg = 0;
	uint64_t bb = 0;

	for (int x = xStart; x < xStop; ++x) {
		for (int y = yStart; y < yStop; ++y) {
			auto color = image.rgbAt<ImageU8Color>(x, y);
			rr += color.r;
			gg += color.g;
			bb += color.b;
		}
	}

	int count = (xStop - xStart)*(yStop - yStart);

	return { (uint8_t)(rr/(count)), (uint8_t)(gg/(count)), (uint8_t)(bb/(count)) };
}

isf::ImageU8Color isf::ColorConverter::guessBackgroundColor(Image& image) {
	uint64_t rr = 0;
	uint64_t gg = 0;
	uint64_t bb = 0;

	// May not necessary be width * height, if computed only for region
	uint64_t wh = 0;

	for (int x = 0; x < image.getWidth(); ++x) {
		for (int y = 0; y < image.getHeight(); ++y) {
			auto color = image.rgbAt<ImageU8Color>(x, y);
			rr += color.r;
			gg += color.g;
			bb += color.b;
			++wh;
		}
	}

	return { (uint8_t)(rr/wh), (uint8_t)(gg/wh), (uint8_t)(bb/wh) };
}

isf::ImageDoubleColor isf::ColorConverter::colorToDoubleColor(ImageU8Color in) {
	return { (double)in.r/UINT8_MAX, (double)in.g/UINT8_MAX, (double)in.b/UINT8_MAX };
}

isf::ImageU8Color isf::ColorConverter::doubleColorToColor(ImageDoubleColor in) {
	return { (uint8_t)round(in.r*UINT8_MAX), (uint8_t)round(in.g*UINT8_MAX), (uint8_t)round(in.b*UINT8_MAX) };
}

double isf::ColorConverter::linearizeColorComponent(double color) {
	double col = color;

	if (col <= 0.04045) {
		col /= 12.92;
	} else {
		col = pow((col + 0.055)/1.055, 2.4);
	}

	return col;
}

isf::ImageDoubleColor isf::ColorConverter::linearizeRgb(ImageDoubleColor in) {
	return { linearizeColorComponent(in.r), linearizeColorComponent(in.g), linearizeColorComponent(in.b) };
}

isf::ImageDoubleColor isf::ColorConverter::rgbToCiexyz(ImageDoubleColor in) {
	return { 0.412453*in.r + 0.357580*in.g + 0.180423*in.b,
	0.212671*in.r + 0.715160*in.g + 0.072169*in.b,
	0.019334*in.r + 0.119193*in.g + 0.950227*in.b };
}

isf::ImageDoubleColor isf::ColorConverter::ciexyzToCielab(ImageDoubleColor in) {
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

isf::ImageDoubleColor isf::ColorConverter::rgbToCielab(ImageU8Color in) {
	return ciexyzToCielab(rgbToCiexyz(linearizeRgb(colorToDoubleColor(in))));
}

double isf::ColorConverter::colorToGrayscale(ImageDoubleColor in) {
	return in.r * 0.299 + in.g * 0.587 + in.b * 0.114;
}
