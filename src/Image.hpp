#pragma once
#include "ImageColorSpace.hpp"

namespace isf{

class Image {

public:
	Image();
	Image(const std::string& filename);
	Image(const Image& other);
	Image(Image&& other);
	~Image();

	Image& operator=(const Image& other);
	Image& operator=(Image&& other);

	ImageColorSpace GetColorSpace();

private:
	ImageColorSpace m_colorSpace;

	void* data;

};

}
