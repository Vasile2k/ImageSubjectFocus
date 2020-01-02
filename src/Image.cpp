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
	memcpy(m_data, other.m_data, size);
}

isf::Image::Image(Image&& other) : Image(other.m_colorSpace, other.m_width, other.m_height) {
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
		memcpy(m_data, other.m_data, size);
	}
}

isf::Image& isf::Image::operator=(Image&& other) {
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
