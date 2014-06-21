/**
 * @file sample2.cpp
 * @brief stb_image 画像書き出しサンプル.
 * @author tori31001 at gmail.com
 * Licensed  under Public Domain.
 *
 */
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <stb_image.h>
#include <stb_image_write.h>

int main(int argc, char** argv)
{
	/// 画像の書き出しサンプル.
	const int w = 256;
	const int h = 256;
	std::vector<unsigned char> image_buffer(w * h * 4);
	for (int y = 0; y < h; ++y)
	{
		for (int x = 0; x < w; ++x)
		{
			int pos = (y*w+x) * 4;
			image_buffer[pos + 0] = x;
			image_buffer[pos + 1] = y;
			image_buffer[pos + 2] = 0xFF;
			image_buffer[pos + 3] = 0xFF;
		}
	}
	stbi_write_png("out.png", w, h, STBI_rgb_alpha, &(*image_buffer.begin()), 0);
	return 0;
}
