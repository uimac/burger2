/**
 * @file UMTga.cpp
 * TGA IO
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#include "UMTga.h"
#include "UMImage.h"
#include "UMVector.h"

#include <iostream>
#include <fstream>

namespace umimage
{

/**
 * save tga
 */
bool UMTga::save(const std::string& path, const UMImage& image) const
{
	if (image.list().empty()) return false;

	try
	{
		std::ofstream ofs(path.c_str(), std::ios::out|std::ios::binary|std::ios::trunc);
		if (!ofs) return false;
		
		// header
		unsigned char header[18] = { 
			0, // id field
			0, // colormap
			2, // format
			0, 0, // colormap entry
			0, // colormap entry size
			0, 0, // x
			0, 0, // y
			0, 0, // w
			0, 0, // h
			0, // bits
			0, // param
		};
		// w
		header[12] = static_cast<unsigned char>((image.width() & 0x00FF));
		header[13] = static_cast<unsigned char>((image.width() & 0xFF00) >> 8);
		// h
		header[14] = static_cast<unsigned char>((image.height() & 0x00FF));
		header[15] = static_cast<unsigned char>((image.height() & 0xFF00) >> 8);
		// bits
		header[16] = 24;
		ofs.write(reinterpret_cast<const char*>(header), 18);

		// floating image to 8bit rgb
		UMImage::B8G8R8Buffer img;
		UMImagePtr flipped = image.create_flip_image(0, true);
		flipped->create_b8g8r8_buffer(img);

		ofs.write(reinterpret_cast<const char*>(&(*img.begin())), img.size());

		// footer
		unsigned char footer[26] = { 
			"\0\0\0\0\0\0\0\0"
			"TRUEVISION-TARGA"
			"\0" };
		ofs.write(reinterpret_cast<const char*>(footer), 26);
	}
	catch(...)
	{
		return false;
	}

	return true;
}

} // umimage
