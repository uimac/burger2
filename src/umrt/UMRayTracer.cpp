/**
 * @file UMRayTracer.cpp
 * a raytracer
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#include "UMRayTracer.h"
#include "UMRenderParameter.h"
#include "UMShaderParameter.h"
#include "UMRay.h"
#include "UMScene.h"
#include "UMVector.h"

#include <limits>
#include <algorithm>
#include <random>
#include <utility>

namespace
{
	using namespace umrt;
	using namespace umdraw;
	
	unsigned int xor128()
	{
		static unsigned int x = 123456789;
		static unsigned int y = 362436069;
		static unsigned int z = 521288629;
		static unsigned int w = 88675123;
		unsigned int t = x ^ (x << 11);
		x = y; y = z; z = w;
		return w = w ^ (w >> 19) ^ t ^ (t >> 8);
	}

	double xor128d()
	{
		return xor128() / 4294967296.0;
	}

	// definition
	UMVec3d trace(const UMRay& ray, UMSceneAccessPtr scene_access, UMShaderParameter& parameter);

	UMVec3d map_one(UMVec3d src) {
		double max = std::max(src.x, std::max(src.y, src.z));
		if (max > 1.0) {
			return src / max;
		}
		return src;
	}

	UMVec3d reflect(const UMRay& ray, const UMVec3d& normal)
	{
		UMVec3d origin(ray.origin());
		return normal * origin.dot(normal) * 2.0 - origin;
	}
	
	class UMIntersection
	{
		DISALLOW_COPY_AND_ASSIGN(UMIntersection);
		public:
			UMIntersection() : 
				closest_distance(std::numeric_limits<double>::max())
			{}

			double closest_distance;
			UMPrimitivePtr closest_primitive;
			UMShaderParameter closest_parameter;
	};

	bool intersect(
		const UMRay& ray, 
		UMSceneAccessPtr scene_access, 
		UMShaderParameter& parameter, 
		UMIntersection& intersection)
	{
		UMPrimitiveList::const_iterator it = scene_access->render_primitive_list().begin();
		for (int i = 0; it != scene_access->render_primitive_list().end(); ++it, ++i)
		{
			UMPrimitivePtr primitive = *it;
			if (primitive->intersects(ray, parameter))
			{
				if (parameter.distance < intersection.closest_distance) 
				{
					intersection.closest_distance = parameter.distance;
					intersection.closest_primitive = primitive;
					intersection.closest_parameter = parameter;
				}
			}
		}
		if (intersection.closest_primitive)
		{
			return true;
		}
		return false;
	}

	/**
	 * shading function
	 */
	UMVec3d shade(const UMPrimitivePtr current, const UMRay& ray, UMSceneAccessPtr scene_access, UMShaderParameter& parameter)
	{
		umdraw::UMScenePtr scene = scene_access->scene();
		UMVec3d normal(parameter.normal.normalized());
		UMVec3d radiance(0);
		UMLightList::const_iterator it = scene->light_list().begin();
		for (; it != scene->light_list().end(); ++it)
		{
			UMVec3d light_position = (*it)->position();
			UMVec3d L = (light_position - parameter.intersect_point).normalized();

			// shadow ray
			UMRay shadow_ray(parameter.intersect_point + parameter.normal * 0.00001, L);
			UMIntersection intersection;
			UMShaderParameter shadow_parameter;
			if (!intersect(shadow_ray, scene_access, shadow_parameter, intersection))
			{
				radiance += parameter.color * normal.dot(L);
			}
		}
		// reflection ray
		if (parameter.bounce > 0)
		{
			UMShaderParameter refrect_parameter;
			parameter.bounce--;
			refrect_parameter.bounce = parameter.bounce;
			UMVec3d refrection_dir = reflect(ray, normal).normalized();
			UMRay reflection_ray(parameter.intersect_point + normal * 0.00001, refrection_dir);
			UMVec3d color = trace(reflection_ray, scene_access, refrect_parameter);
			//UMVec3d nl = parameter.normal.dot(refrection_dir);
			radiance += color;
		}
		
		return map_one(radiance);
	}

	/**
	 * trace and return color of the hit point
	 */
	UMVec3d trace(const UMRay& ray, UMSceneAccessPtr scene_access, UMShaderParameter& parameter)
	{
		umdraw::UMScenePtr scene = scene_access->scene();
		UMIntersection intersection;
		if (parameter.bounce == 1)
		{
			if (!intersect(ray, scene_access, parameter, intersection)){
				return scene->background_color();
			}
		}

		if (intersection.closest_primitive)
		{
			return shade(intersection.closest_primitive, ray, scene_access, intersection.closest_parameter);
		}
		return scene->background_color();
	}
}

namespace umrt
{

/**
 * render
 */
bool UMRayTracer::render(UMSceneAccessPtr scene_access, UMRenderParameter& parameter)
{
	umdraw::UMScenePtr scene = scene_access->scene();
	if (!scene) return false;
	if (width_ == 0 || height_ == 0) return false;
	if (!scene->camera()) return false;
	
	const int sample_count = parameter.super_sampling_count().x * parameter.super_sampling_count().y;
	const double inv_sample_count = 1.0 / sample_count;

	UMImage::ImageBuffer& dst_buffer = parameter.output_image()->mutable_list();
	
	//std::random_device random_device;
	//std::vector<unsigned int> seed(2 * height_);
	//std::generate(seed.begin(), seed.end(), std::ref(random_device));
	
//#pragma omp parallel for schedule(dynamic, 1) num_threads(8)
	for (int y = 0; y < height_; ++y)
	{
		//std::mt19937 mt(std::seed_seq(seed.begin() + 2 * y, seed.begin() +  2 * (y + 1)));

		if (sample_count > 1)
		{
			for (int x = 0; x < width_; ++x)
			{
				const int pos = width_ * y + x;
				for (int s = 0; s < sample_count; ++s)
				{
					UMVec2d sample_point(xor128d(), xor128d());
					sample_point.x += x;
					sample_point.y += y;
					scene_access->generate_ray(ray_, sample_point);
					UMVec3d color = trace(ray_, scene_access, shader_param_);
					dst_buffer[pos] += UMVec4d(color, 1.0);
				}
				dst_buffer[pos] *= inv_sample_count;
			}
		}
		else
		{
			for (int x = 0; x < width_; ++x)
			{
				const int pos = width_ * y + x;
				scene_access->generate_ray(ray_, UMVec2d(x, y));
				UMVec3d color = trace(ray_, scene_access, shader_param_);
				dst_buffer[pos] = UMVec4d(color, 1.0);
			}
		}
	}
	return true;
}

/**
 * progressive render
 */
bool UMRayTracer::progress_render(UMSceneAccessPtr scene_access, UMRenderParameter& parameter)
{
	umdraw::UMScenePtr scene = scene_access->scene();
	if (!scene) return false;
	if (width_ == 0 || height_ == 0) return false;
	if (!scene->camera()) return false;
	
	const int ystep = 10;
	
	const int sample_count = parameter.super_sampling_count().x * parameter.super_sampling_count().y;
	const double inv_sample_count = 1.0 / sample_count;
	
	//std::random_device random_device;
	//std::vector<unsigned int> seed(2 * height_);
	//std::generate(seed.begin(), seed.end(), std::ref(random_device));
	
	for (int& y = current_y_, rows = (y + ystep); y < rows; ++y)
	{
		// end
		if (y == height_) { return false; }
		
		//std::mt19937 mt(std::seed_seq(seed.begin() + 2 * y, seed.begin() +  2 * (y + 1)));

		for (int x = 0; x < width_; ++x)
		{
			const int pos = width_ * y + x;
			for (int s = 0; s < sample_count; ++s)
			{
				UMVec2d sample_point(xor128d(), xor128d());
				sample_point.x += x;
				sample_point.y += y;
				scene_access->generate_ray(ray_, sample_point);
				UMVec3d color = trace(ray_, scene_access, shader_param_);
				parameter.output_image()->mutable_list()[pos] += UMVec4d(color, 1.0);
			}
			parameter.output_image()->mutable_list()[pos] *= inv_sample_count;
		}
	}
	
	return true;
}

} // umrt
