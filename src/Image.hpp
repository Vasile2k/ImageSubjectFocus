#pragma once
#include "ImageColorSpace.hpp"
#include "ImageColor.hpp"
#include <type_traits>

namespace isf{

class Image {

public:
	Image(const ImageColorSpace colorSpace, size_t width, size_t height);
	Image(const std::string& filename);
	Image(const Image& other);
	Image(Image&& other) noexcept; /* Compiler asks for noexcept */
	~Image();

	Image& operator=(const Image& other);
	Image& operator=(Image&& other) noexcept;

	ImageColorSpace getColorSpace() const;
	inline size_t getWidth() const;
	inline size_t getHeight() const;

	// PNG only
	bool saveToFile(const std::string& filename);

	template<typename T>
	inline T& rgbAt(size_t x, size_t y) const;

	template<typename T>
	inline T& grayscaleAt(size_t x, size_t y) const;

private:
	ImageColorSpace m_colorSpace;

	size_t m_width;
	size_t m_height;
	void* m_data;

	// Used internally after template typename checking
	template<typename T>
	inline T& _rgbAt(size_t x, size_t y) const;

	template<typename T>
	inline T& _grayscaleAt(size_t x, size_t y) const;

};

template<typename T>
T& isf::Image::rgbAt(size_t x, size_t y) const {
	static_assert((std::is_same<T, ImageU8Color>::value || std::is_same<T, ImageDoubleColor>::value), "Invalid type for retrieving RGB");
	return this->_rgbAt<T>(x, y);
}

template<typename T>
T& isf::Image::grayscaleAt(size_t x, size_t y) const {
	static_assert((std::is_same<T, uint8_t>::value || std::is_same<T, double>::value), "Invalid type for retrieving grayscale");
	return this->_grayscaleAt<T>(x, y);
}

}
