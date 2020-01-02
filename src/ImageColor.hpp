#pragma once
#include <cstdint>

namespace isf {

union ImageU8Color {
	// For RGB colorspace
	struct {
		uint8_t r, g, b;
	};
};

union ImageDoubleColor {
	// For RGB colorspace
	struct {
		double r, g, b;
	};
	// For CIE-XYZ colorspace
	struct {
		double x, y, z;
	};
	// FOR CIE-LAB colorspace
	struct {
		double ls, as, bs;
	};
	// For unnormalized/unknown colorspace
	struct {
		double val0, val1, val2;
	};
};

}
