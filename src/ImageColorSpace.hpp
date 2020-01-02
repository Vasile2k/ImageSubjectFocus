#pragma once

namespace isf {

class ImageColorSpace {

public:
	enum ImageColorSpaceType : uint8_t {
		COLORSPACE_U8_RGB,
		COLORSPACE_U8_GRAYSCALE,
		COLORSPACE_DOUBLE_RGB,
		COLORSPACE_DOUBLE_CIE_XYZ,
		COLORSPACE_DOUBLE_CIE_LAB,
		COLORSPACE_DOUBLE_RGB_LINIARIZED,
		COLORSPACE_DOUBLE_GRAYSCALE,
		COLORSPACE_DOUBLE_GRAYSCALE_RMS,
		COLORSPACE_DOUBLE_UNNORMALIZED,
	};

	constexpr ImageColorSpace(ImageColorSpaceType type) : m_type(type) {}
	const inline size_t getSize() const {
		switch (this->m_type) {
			case COLORSPACE_U8_RGB:
				return 3*sizeof(uint8_t);
			case COLORSPACE_U8_GRAYSCALE:
				return sizeof(uint8_t);
			case COLORSPACE_DOUBLE_RGB:
			case COLORSPACE_DOUBLE_CIE_XYZ:
			case COLORSPACE_DOUBLE_CIE_LAB:
			case COLORSPACE_DOUBLE_RGB_LINIARIZED:
				return 3*sizeof(double);
			case COLORSPACE_DOUBLE_GRAYSCALE:
			case COLORSPACE_DOUBLE_GRAYSCALE_RMS:
			case COLORSPACE_DOUBLE_UNNORMALIZED:
				return sizeof(double);
		}
	}

private:
	ImageColorSpaceType m_type;
};

}
