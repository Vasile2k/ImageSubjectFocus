#include "pch.h"
#include "Image.hpp"
#include "Allocator.hpp"
#include "Debugger.hpp"

isf::Image::Image(const ImageColorSpace colorSpace, size_t width, size_t height) : m_colorSpace(colorSpace), m_width(width), m_height(height) {
	m_data = ISF_MALLOC(m_width * m_height * m_colorSpace.getSize());
}

isf::Image::Image(const std::string& filename) : m_colorSpace(ImageColorSpace::COLORSPACE_U8_RGB) {
	int x;
	int y;
	int channels;
	m_data = stbi_load(filename.c_str(), &x, &y, &channels, 3);
	m_width = x;
	m_height = y;
	ISF_ASSERT(channels == 3, "Wrong number of channels read");
}

isf::Image::Image(const Image& other) : Image(other.m_colorSpace, other.m_width, other.m_height) {
	size_t size = m_width * m_height * m_colorSpace.getSize();
	m_data = ISF_MALLOC(size);
	ISF_ASSERT(m_data, "Allocation failed");
	if (m_data) {
		memcpy(m_data, other.m_data, size);
	}
}

isf::Image::Image(Image&& other) noexcept : Image(other.m_colorSpace, other.m_width, other.m_height) {
	m_data = other.m_data;
	other.m_data = nullptr;
}

isf::Image::~Image() {
	if (m_data) {
		ISF_FREE(m_data);
	}
}

isf::Image& isf::Image::operator=(const Image& other) {
	if (this != &other) {
		if (m_data) {
			ISF_FREE(m_data);
		}
		m_colorSpace = other.m_colorSpace;
		m_width = other.m_width;
		m_height = other.m_height;
		size_t size = m_width * m_height * m_colorSpace.getSize();
		m_data = ISF_MALLOC(size);
		ISF_ASSERT(m_data, "Allocation failed");
		if (m_data) {
			memcpy(m_data, other.m_data, size);
		}
	}
	return *this;
}

isf::Image& isf::Image::operator=(Image&& other) noexcept {
	if (this != &other) {
		if (m_data) {
			ISF_FREE(m_data);
		}
		m_colorSpace = other.m_colorSpace;
		m_width = other.m_width;
		m_height = other.m_height;
		m_data = other.m_data;
		other.m_data = nullptr;
	}
	return *this;
}

isf::ImageColorSpace isf::Image::getColorSpace() const {
	return m_colorSpace;
}

size_t isf::Image::getWidth() const {
	return m_width;
}

size_t isf::Image::getHeight() const {
	return m_height;
}

bool isf::Image::saveToFile(const std::string& filename) {
	ISF_ASSERT(m_data != nullptr, "No data to be written");
	ISF_ASSERT(m_colorSpace.isWritable(), "Can't write double color space to image");
	return stbi_write_png(filename.c_str(), (int)m_width, (int)m_height, m_colorSpace.getChannels(), m_data, 0);
}

template<>
isf::ImageU8Color& isf::Image::_rgbAt(size_t x, size_t y) const {
	ISF_ASSERT(!m_colorSpace.isDouble(), "Data type is not U8");
	ISF_ASSERT(m_colorSpace.getChannels() == 3, "Image is not RGB");
	return *(static_cast<ImageU8Color*>(m_data) + y * m_width + x);
}

template<>
isf::ImageDoubleColor& isf::Image::_rgbAt(size_t x, size_t y) const {
	ISF_ASSERT(m_colorSpace.isDouble(), "Data type is not double");
	ISF_ASSERT(m_colorSpace.getChannels() == 3, "Image is not RGB");
	return *(static_cast<ImageDoubleColor*>(m_data) + y * m_width + x);
}

template<>
uint8_t& isf::Image::_grayscaleAt(size_t x, size_t y) const {
	ISF_ASSERT(!m_colorSpace.isDouble(), "Data type is not U8");
	ISF_ASSERT(m_colorSpace.getChannels() == 1, "Image is not grayscale");
	return *(static_cast<uint8_t*>(m_data) + y * m_width + x);
}

template<>
double& isf::Image::_grayscaleAt(size_t x, size_t y) const {
	ISF_ASSERT(m_colorSpace.isDouble(), "Data type is not double");
	ISF_ASSERT(m_colorSpace.getChannels() == 1, "Image is not grayscale");
	return *(static_cast<double*>(m_data) + y * m_width + x);
}
