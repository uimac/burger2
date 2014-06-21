/**
 * @file sample3.cpp
 * @brief imathサンプル.
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

#include <Imath/ImathVec.h>
#include <Imath/ImathLine.h>
#include <Imath/ImathLineAlgo.h>

int main(int argc, char** argv)
{
	const int w = 256;
	const int h = 256;

	// 画像真ん中の後ろのほうをposとした適当なレイを作る.
	Imath::Line3d ray;
	ray.pos = Imath::V3d(w / 2.0, h / 2.0, 100);

	// 三角形.
	struct Triangle {
		Imath::V3d p0;
		Imath::V3d p1;
		Imath::V3d p2;
	} triangle;
	triangle.p0 = Imath::V3d(128, 50, -10);
	triangle.p1 = Imath::V3d(50, 200, -10);
	triangle.p2 = Imath::V3d(200, 200, -10);

	std::vector<unsigned char> image_buffer(w * h * 4);
	for (int y = 0; y < h; ++y)
	{
		for (int x = 0; x < w; ++x)
		{
			int pos = (y*w + x) * 4;
			image_buffer[pos + 0] = x;
			image_buffer[pos + 1] = y;
			image_buffer[pos + 2] = 0xFF;
			image_buffer[pos + 3] = 0xFF;

			// レイの方向.
			ray.dir = Imath::V3d(x, y, 0) - ray.pos;
			// 三角形とレイの交差判定.
			Imath::V3d hit_point;
			Imath::V3d barycentric;
			bool is_front = false;
			if (Imath::intersect(ray, triangle.p0, triangle.p1, triangle.p2, hit_point, barycentric, is_front))
			{
				if (is_front)
				{
					// レイが三角形前面に当たった.
					image_buffer[pos + 0] = static_cast<unsigned char>(barycentric.x * 0xFF);
					image_buffer[pos + 1] = static_cast<unsigned char>(barycentric.y * 0xFF);
					image_buffer[pos + 2] = static_cast<unsigned char>(barycentric.z * 0xFF);
					image_buffer[pos + 3] = 0xFF;
				}
			}
		}
	}
	stbi_write_png("out.png", w, h, STBI_rgb_alpha, &(*image_buffer.begin()), 0);
	return 0;
}
