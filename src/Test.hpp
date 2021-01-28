#pragma once

#include "ImageColor.hpp"

namespace isf {

/**
 * In order to perform the tests include this file somewhere in the project
 * This never gets executed but is evaluated at compile time
 * If everything is good it will do nothing
 * Else it will throw a compile-time error
 */
class Test {
private:
	Test() = delete;
	bool testPacking() {
		static_assert(sizeof(isf::ImageU8Color) == 3, "Packing does not work correctly");
		static_assert(sizeof(isf::ImageDoubleColor) == 3 * sizeof(double), "Packing does not work correctly");
	}
};

}
