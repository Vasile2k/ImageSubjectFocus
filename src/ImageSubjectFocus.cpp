#include <iostream>
#include "pch.h"
#include <cstdint>

int main(){
	
	int x, y, channels;
	uint8_t* image = stbi_load("test/test01.jpg", &x, &y, &channels, 0);

	stbi_write_png("temp/show1.png", x, y, channels, image, 0);

	stbi_image_free(image);
	
	std::cout << "The horse is in the house" << log10(100) << std::endl;
	return 0;
}

