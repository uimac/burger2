/**
 * @file UMPathTracer.cpp
 * a pathtracer
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#include "UMPathTracer.h"
#include "UMRenderParameter.h"
#include "UMShaderParameter.h"
#include "UMRay.h"
#include "UMVector.h"
#include "UMScene.h"
#include "UMSceneAccess.h"
#include "UMAreaLight.h"

#include <limits>
#include <algorithm>
#include <random>
#include <utility>
#include <functional>


namespace
{
	using namespace umrt;
	using namespace umdraw;

	const int minimum_path_depth = 2;
	
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

	UMVec4d map_one(UMVec4d src) {
		double max = std::max(src.x, std::max(src.y, src.z));
		if (max > 1.0) {
			return src / max;
		}
		src.w = 1.0;
		return src;
	}

	UMVec3d hemisphere(const UMVec3d& normal)
	{
		UMVec3d u, v, w;
		w = normal;
		if (fabs(w.x) > FLT_EPSILON) {
			u = UMVec3d(0, 1, 0).cross(w).normalized();
		} else {
			u = UMVec3d(1, 0, 0).cross(w).normalized();
		}
		v = w.cross(u);
		const double r1 = 2 * M_PI * xor128d();
		const double r2 = xor128d();
		const double r2s = sqrt(r2);
		UMVec3d dir = (u * cos(r1) * r2s + v * sin(r1) * r2s + w * sqrt(1.0 - r2)).normalized();
		return dir;
	}

}

namespace umrt
{

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

	static bool intersect(
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
		else
		{
			return false;
		}
	}

	static bool intersect(
		const UMRay& ray, 
		UMSceneAccessPtr scene_access)
	{
		UMPrimitiveList::const_iterator it = scene_access->render_primitive_list().begin();
		for (int i = 0; it != scene_access->render_primitive_list().end(); ++it, ++i)
		{
			UMPrimitivePtr primitive = *it;
			if (primitive->intersects(ray))
			{
				return true;
			}
		}
		return false;
	}

};

UMPathTracer::UMPathTracer() : 
	current_sample_count_(0),
	current_subpixel_x_(0),
	current_subpixel_y_(0),
	max_sample_count_(0)
	//sample_event_(std::make_shared<UMEvent>(eEventTypeRenderProgressSample))
{
	//mutable_event_list().push_back(sample_event_);
}

/**
 * trace and return color of the hit point
 */
UMVec3d UMPathTracer::trace(const UMRay& ray, UMSceneAccessPtr scene_access, UMShaderParameter& parameter)
{
	umdraw::UMScenePtr scene = scene_access->scene();
	UMIntersection intersection;
	if (!UMIntersection::intersect(ray, scene_access, parameter, intersection)){
		return scene->background_color();
	}

	UMVec3d normal = intersection.closest_parameter.normal;
	if (normal.dot(ray.direction()) >= 0.0)
	{
		normal = -normal;
	}
	
	UMVec3d point_color(intersection.closest_parameter.color);
	double russian_roulette_probability = std::max(point_color.x, std::max(point_color.y, point_color.z));
	
	if (parameter.depth < 16) {
		russian_roulette_probability *= pow(0.5, 16 - parameter.depth);
	}

	UMVec3d color = intersection.closest_parameter.emissive;

	if (parameter.depth < (parameter.max_depth - minimum_path_depth)) {
		if (xor128d() >= russian_roulette_probability)
		{
			return color;
		}
	} else {
		russian_roulette_probability = 1.0;
	}
	--parameter.depth;

	// diffuse direct
	color += illuminate_direct(ray, scene_access, intersection, parameter);
	// diffuse indirect
	color += illuminate_indirect(ray, scene_access, intersection, parameter) / russian_roulette_probability;

	return color;
}

/**
 * direct lighting
 */
UMVec3d UMPathTracer::illuminate_direct(
	const UMRay& ray, 
	UMSceneAccessPtr scene_access, 
	const UMIntersection& intersection,
	UMShaderParameter& parameter)
{
	umdraw::UMScenePtr scene = scene_access->scene();
	UMVec3d color(0);
	UMMaterialPtr mat = intersection.closest_parameter.material;

	UMLightList::const_iterator it = scene->light_list().begin();
	for (; it != scene->light_list().end(); ++it)
	{
		UMLightPtr light = *it;
		UMVec3d intensity;
		UMVec3d sample_point;
		UMVec3d direction;
		UMVec2d random_value(xor128d(), xor128d());
		if (UMAreaLight::sample(intensity, sample_point, direction, light, intersection.closest_parameter, random_value))
		{
			UMVec3d p(intersection.closest_parameter.intersect_point);
			UMRay shadow_ray(p, direction.normalized());
			shadow_ray.set_tmax( (sample_point - p).length() );
			if (!UMIntersection::intersect(shadow_ray, scene_access))
			{
				color += (intersection.closest_parameter.color * M_PI_INV).multiply(intensity);
			}
		}
	}
	return color;
}

/**
 * indirect lighting
 */
UMVec3d UMPathTracer::illuminate_indirect(
	const UMRay& ray, 
	UMSceneAccessPtr scene_access, 
	const UMIntersection& intersection,
	UMShaderParameter& parameter)
{
	UMVec3d color;
	UMMaterialPtr mat = intersection.closest_parameter.material;

	UMVec3d dir = hemisphere(parameter.normal);
	UMRay next_ray(intersection.closest_parameter.intersect_point, dir);
	UMVec3d traced_color = trace(next_ray, scene_access, parameter);
	// importance sampling
	color = traced_color.multiply(intersection.closest_parameter.color);
	return color;
}

/**
 * render
 */
bool UMPathTracer::render(UMSceneAccessPtr scene_access, UMRenderParameter& parameter)
{
	umdraw::UMScenePtr scene = scene_access->scene();
	if (!scene) return false;
	if (width_ == 0 || height_ == 0) return false;
	if (!scene->camera()) return false;

	const int sample_count = parameter.sample_count();
	//std::random_device random_device;
	//std::vector<unsigned int> seed(2 * height_);
	//std::generate(seed.begin(), seed.end(), std::ref(random_device));

	for (int y = 0; y < height_; ++y)
	{
		//std::mt19937 mt(std::seed_seq(seed.begin() + 2 * y, seed.begin() +  2 * (y + 1)));

		for (int x = 0; x < width_; ++x)
		{
			const int pos = width_ * y + x;
			for (int s = 0; s < sample_count; ++s)
			{
				UMVec2d sample_point(xor128d(),  xor128d());
				sample_point.x += x;
				sample_point.y += y;
				UMRay ray;
				scene_access->generate_ray(ray, sample_point);
				UMShaderParameter shader_parameter;
				UMVec3d color = trace(ray, scene_access, shader_parameter);
				parameter.output_image()->mutable_list()[pos] += UMVec4d(color, 1.0);
			}
		}
	}
	
	return true;
}

/**
 * progressive render
 */
bool UMPathTracer::progress_render(UMSceneAccessPtr scene_access, UMRenderParameter& parameter)
{
	umdraw::UMScenePtr scene = scene_access->scene();
	if (!scene) return false;
	if (width_ == 0 || height_ == 0) return false;
	if (!scene->camera()) return false;
	
	const UMVec2i super_sampling = parameter.super_sampling_count();
	
	// end
	if (current_sample_count_ == max_sample_count_ &&
		current_subpixel_x_ == (super_sampling.x-1) &&
		current_subpixel_y_ == (super_sampling.y-1)) 
	{
		current_subpixel_x_ = 0;
		current_subpixel_y_ = 0;
		return false;
	}
	// start
	if (current_sample_count_ == 0 &&
		current_subpixel_x_ == 0 &&
		current_subpixel_y_ == 0) {
		// init temporary image
		current_subpixel_x_ = 0;
		current_subpixel_y_ = 0;
		temporary_image_.init(width_, height_);
		max_sample_count_ = parameter.sample_count() / (super_sampling.x * super_sampling.y);
	}
	
	bool is_end_subpixel = 
		(current_subpixel_x_ == (super_sampling.x-1) &&
		 current_subpixel_y_ == (super_sampling.y-1));
	if (is_end_subpixel)
	{
		++current_sample_count_;
		current_subpixel_x_ = 0;
		current_subpixel_y_ = 0;
	}
	else
	{
		if (current_subpixel_x_ == (super_sampling.x-1))
		{
			++current_subpixel_y_;
			current_subpixel_x_ = 0;
		}
		else
		{
			++current_subpixel_x_;
		}
	}
	const double inv_current_sample_count = 1.0 / current_sample_count_;
	const double inv_current_subpixel_sample_count 
		= 1.0 / (current_subpixel_x_ + 1)
		* 1.0 / (current_subpixel_y_ + 1);
	const double inv_super_sampling_x = 1.0 / (double)super_sampling.x;
	const double inv_super_sampling_y = 1.0 / (double)super_sampling.y;
	
	//std::random_device random_device;
	//std::vector<unsigned int> seed(2 * height_);
	//std::generate(seed.begin(), seed.end(), std::ref(random_device));

//#pragma omp parallel for schedule(dynamic, 1) num_threads(8)
	for (int y = 0; y < height_; ++y)
	{
		//std::mt19937 mt(std::seed_seq(seed.begin() + 2 * y, seed.begin() +  2 * (y + 1)));
		for (int x = 0; x < width_; ++x)
		{
			// target pixel
			const int pos = width_ * y + x;
			UMVec4d& current_color = temporary_image_.mutable_list()[pos];
			UMVec4d& out_color = parameter.output_image()->mutable_list()[pos];
			
			// sample point
			UMVec2d sample_point(x, y);
			sample_point.x += current_subpixel_x_ * inv_super_sampling_x;
			sample_point.y += current_subpixel_y_ * inv_super_sampling_y;
			// generate camera ray
			UMRay ray;
			scene_access->generate_ray(ray, sample_point);
			// trace
			UMShaderParameter shader_param;
			UMVec3d color = trace(ray, scene_access, shader_param);
			// output
			current_color += UMVec4d(color, 1.0);

			if (is_end_subpixel)
			{
				out_color = map_one(current_color 
					* inv_current_sample_count
					* inv_super_sampling_x
					* inv_super_sampling_y);
			}
		}
	}

	//umbase::UMAny sample_count(current_sample_count_);
	//sample_event_->set_parameter(sample_count);
	//sample_event_->notify();

	return true;
}

} // umrt
