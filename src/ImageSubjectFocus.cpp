#include <iostream>
#include "pch.h"
#include "stb_image.h"

int main(){
	stbi_set_flip_vertically_on_load(1); // Just to test if it builds
	std::cout << "The horse is in the house" << log10(100) << std::endl;
	return 0;
}