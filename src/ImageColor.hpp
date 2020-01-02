#pragma once
#include <cstdint>

namespace isf {

// Forward declaration
union ImageU8Color;
union ImageDoubleColor;

#pragma pack(push, 1)
union ImageU8Color {
	// For RGB colorspace
	struct {
		uint8_t r, g, b;
	};

	ImageDoubleColor toDoubleColor() const;
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

	ImageU8Color toU8Color() const;
};
#pragma pack(pop)

}
