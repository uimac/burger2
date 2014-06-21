/**
 * @file sample3.cpp
 * @brief tinyobjloaderサンプル.
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

#include <tiny_obj_loader.h>

int main(int argc, char** argv)
{
	if (argc <= 1) { return 0; }

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
		Imath::V3d n0;
		Imath::V3d n1;
		Imath::V3d n2;
	};

	// OBJファイルの読み込み
	std::vector<tinyobj::shape_t> shapes;
	std::string err = tinyobj::LoadObj(shapes, argv[1]);
	if (!err.empty()) {
		std::cerr << err << std::endl;
		exit(1);
	}

	// OBJファイルから三角形リストを読み込む.
	// カメラが適当なので、適当にスケールかけます.
	// 画像はy座標が下向きなので、モデルのy座標を反転します.
	const Imath::V3f scale(100.0, -100.0, 100.0);
	// 三角形リスト.
	std::vector<Triangle> triangles;
	for (size_t i = 0; i < shapes.size(); i++) 
	{
		const size_t face_count = shapes[i].mesh.indices.size() / 3;
		for (size_t f = 0; f < face_count; f++)
		{
			const int vi0 = shapes[i].mesh.indices[f*3+0];
			const int vi1 = shapes[i].mesh.indices[f*3+1];
			const int vi2 = shapes[i].mesh.indices[f*3+2];
			Triangle tri;
			// 頂点.
			{
				std::vector<float>& mp = shapes[i].mesh.positions;
				tri.p0 = Imath::V3d(mp[vi0 * 3 + 0], mp[vi0 * 3 + 1], mp[vi0 * 3 + 2]) * scale;
				tri.p1 = Imath::V3d(mp[vi1 * 3 + 0], mp[vi1 * 3 + 1], mp[vi1 * 3 + 2]) * scale;
				tri.p2 = Imath::V3d(mp[vi2 * 3 + 0], mp[vi2 * 3 + 1], mp[vi2 * 3 + 2]) * scale;
			}
			// 法線.
			if (!shapes[i].mesh.normals.empty())
			{
				std::vector<float>& mn = shapes[i].mesh.normals;
				tri.n0 = Imath::V3d(mn[vi0 * 3 + 0], mn[vi0 * 3 + 1], mn[vi0 * 3 + 2]);
				tri.n1 = Imath::V3d(mn[vi1 * 3 + 0], mn[vi1 * 3 + 1], mn[vi1 * 3 + 2]);
				tri.n2 = Imath::V3d(mn[vi2 * 3 + 0], mn[vi2 * 3 + 1], mn[vi2 * 3 + 2]);
			}
			triangles.push_back(tri);
		}
	}

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
			// レイが当たったかどうか.
			bool is_hit = false;
			// 一番近くの当たった点までの距離.
			double closest_distance = std::numeric_limits<double>::max();
			// 一番近くの当たった点の法線.
			Imath::V3d closest_normal;

			for (size_t t = 0; t < triangles.size(); ++t)
			{
				Triangle& triangle = triangles[t];

				// 三角形とレイの交差判定.
				Imath::V3d hit_point;
				Imath::V3d uvw;
				bool is_front = false;
				if (Imath::intersect(ray, triangle.p0, triangle.p1, triangle.p2, hit_point, uvw, is_front))
				{
					if (is_front)
					{
						is_hit = true;
						// 前当たったとこより近いかどうか記憶しておく.
						const double distance = (hit_point - ray.pos).length();
						if (closest_distance > distance)
						{
							closest_distance = distance;
							closest_normal = (triangle.n0*uvw.x + triangle.n1*uvw.y + triangle.n2*uvw.z).normalized();
						}
					}
				}
			}

			if (is_hit)
			{
				// レイが三角形前面に当たった.
				Imath::V3d light(1, 1, 1);
				double nl = std::max(0.0, closest_normal.dot(light.normalized()));
				image_buffer[pos + 0] = static_cast<unsigned char>(nl * 0xFF);
				image_buffer[pos + 1] = static_cast<unsigned char>(nl * 0xFF);
				image_buffer[pos + 2] = static_cast<unsigned char>(nl * 0xFF);
				image_buffer[pos + 3] = 0xFF;
			}
		}
	}
	stbi_write_png("out.png", w, h, STBI_rgb_alpha, &(*image_buffer.begin()), 0);
	return 0;
}
