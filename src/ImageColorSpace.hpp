#pragma once

#include "Debugger.hpp"

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
		ISF_ASSERT(false, "Unkown data type");
		return 0;
	}

	const inline int getChannels() const {
		switch (this->m_type) {
			case COLORSPACE_U8_RGB:
			case COLORSPACE_DOUBLE_RGB:
			case COLORSPACE_DOUBLE_CIE_XYZ:
			case COLORSPACE_DOUBLE_CIE_LAB:
			case COLORSPACE_DOUBLE_RGB_LINIARIZED:
				return 3;
			case COLORSPACE_U8_GRAYSCALE:
			case COLORSPACE_DOUBLE_GRAYSCALE:
			case COLORSPACE_DOUBLE_GRAYSCALE_RMS:
			case COLORSPACE_DOUBLE_UNNORMALIZED:
				return 1;
		}
		ISF_ASSERT(false, "Unkown data type");
		return 0;
	}

	// Can be written to file?
	const inline bool isWritable() const {
		switch (this->m_type) {
			case COLORSPACE_U8_RGB:
			case COLORSPACE_U8_GRAYSCALE:
				return true;
			default:
				return false;
		}
	}

	// Is this data type double or U8?
	const inline bool isDouble() const {
		return this->getSize() == sizeof(double) * this->getChannels();
	}

private:
	ImageColorSpaceType m_type;
};

}
