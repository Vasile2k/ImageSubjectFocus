#pragma once
#include "ImageColorSpace.hpp"

namespace isf{

class Image {

public:
	Image(const ImageColorSpace colorSpace, size_t width, size_t height);
	Image(const std::string& filename);
	Image(const Image& other);
	Image(Image&& other);
	~Image();

	Image& operator=(const Image& other);
	Image& operator=(Image&& other);

	ImageColorSpace getColorSpace() const;
	inline size_t getWidth() const;
	inline size_t getHeight() const;

private:
	ImageColorSpace m_colorSpace;

	size_t m_width;
	size_t m_height;
	void* m_data;

};

}
