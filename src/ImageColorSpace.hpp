#pragma once

namespace isf {

enum class ImageColorSpace {
	COLORSPACE_U8_RGB,
	COLORSPACE_U8_GRAYSCALE,
	COLORSPACE_DOUBLE_RGB,
	COLORSPACE_DOUBLE_GRAYSCALE,
	COLORSPACE_DOUBLE_GRAYSCALE_RMS,
	COLORSPACE_DOUBLE_RGB_LINIARIZED,
	COLORSPACE_DOUBLE_CIE_XYZ,
	COLORSPACE_DOUBLE_CIE_LAB,
	COLORSPACE_DOUBLE_UNNORMALIZED,
};

}
