#include "pch.h"
#include "ImageColor.hpp"

isf::ImageDoubleColor isf::ImageU8Color::toDoubleColor() const {
	return { (double)r/UINT8_MAX,
		(double)g/UINT8_MAX,
		(double)b/UINT8_MAX };
}

isf::ImageU8Color isf::ImageDoubleColor::toU8Color() const {
	return { (uint8_t)round(r*UINT8_MAX),
		(uint8_t)round(g*UINT8_MAX),
		(uint8_t)round(b*UINT8_MAX) };
}