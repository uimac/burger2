/**
 * @file UMToonRender.cpp
 * a raytracer
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#include "UMToonRender.h"
#include "UMRenderParameter.h"
#include "UMShaderParameter.h"
#include "UMRay.h"
#include "UMScene.h"
#include "UMVector.h"
#include "UMStringUtil.h"

#include "UMPathTracer.h"

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
		UMVec3d normal = parameter.normal;
		UMVec3d radiance(0);
		UMLightList::const_iterator it = scene->light_list().begin();
		for (; it != scene->light_list().end(); ++it)
		{
			UMVec3d light_position = (*it)->position();
			UMVec3d L = light_position.normalized();
			UMVec3d light_dir_from_point = (light_position - parameter.intersect_point).normalized();

			// shadow ray
			UMRay shadow_ray(parameter.intersect_point + normal * 0.00001, light_dir_from_point);
			UMIntersection intersection;
			UMShaderParameter shadow_parameter;
			//if (!intersect(shadow_ray, scene_access, shadow_parameter, intersection))
			{
				radiance += parameter.color * std::max(0.0, normal.dot(L));
			}
		}
		//// reflection ray
		//if (parameter.bounce > 0)
		//{
		//	UMShaderParameter refrect_parameter;
		//	parameter.bounce--;
		//	refrect_parameter.bounce = parameter.bounce;
		//	UMVec3d refrection_dir = reflect(ray, normal).normalized();
		//	UMRay reflection_ray(parameter.intersect_point + normal * 0.00001, refrection_dir);
		//	UMVec3d color = trace(reflection_ray, scene_access, refrect_parameter);
		//	//UMVec3d nl = parameter.normal.dot(refrection_dir);
		//	radiance += color*0.01;
		//}
		
		return map_one(radiance);
	}

	/**
	 * trace and return color of the hit point
	 */
	UMVec3d trace(const UMRay& ray, UMSceneAccessPtr scene_access, UMShaderParameter& parameter)
	{
		umdraw::UMScenePtr scene = scene_access->scene();
		UMIntersection intersection;
		//if (parameter.bounce == 1)
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

	/**
	 * trace 24 rays around ray
	 */
	double trace_cone(
		const UMVec2d& pixel, 
		const UMRay& ray, 
		UMSceneAccessPtr scene_access, 
		UMShaderParameter& parameter)
	{
		double foreign_geometry_area = 0.0;
		double half_size = parameter.outline_size * 0.5;
		
		const int number_of_stencil_ray = 24;
		std::vector<UMRay> rays;
		rays.resize(number_of_stencil_ray);
		{
			double theta_adder = M_PI / 4.0;
			for (int i = 0; i < 8; ++i)
			{
				double theta = theta_adder * i;
				UMVec2d point(
					pixel.x + half_size * cos(theta),
					pixel.y + half_size * sin(theta));
				scene_access->generate_ray(rays.at(i), point);
			}
		}
		{
			double theta_adder = M_PI / 8.0;
			for (int i = 0; i < 16; ++i)
			{
				double theta = theta_adder * i;
				UMVec2d point(
					pixel.x + parameter.outline_size * cos(theta),
					pixel.y + parameter.outline_size * sin(theta));
				scene_access->generate_ray(rays.at(8 + i), point);
			}
		}

		int sample_material = -1;
		if (parameter.material)
		{
			sample_material = parameter.material->id();
		}

		int hit_other_material = 0;
		int far_from_sample_rays = 0;
		UMVec3d gradient_normals[number_of_stencil_ray];
		const double distance_threshold = 3.0;
		for (int i = 0; i < number_of_stencil_ray; ++i)
		{
			UMRay& ray = rays.at(i);
			UMIntersection intersection;
			UMShaderParameter shader_parameter;
			if (intersect(ray, scene_access, shader_parameter, intersection))
			{
				int material_id = intersection.closest_parameter.material->id();
				if (sample_material != material_id)
				{
					++hit_other_material;
				}
				else
				{
					//if (i == 8 || i == 12 || i == 16 || i == 20)
					{
						gradient_normals[i] = intersection.closest_parameter.face_normal;
					}
					if ( fabs(intersection.closest_distance - parameter.distance) > distance_threshold)
					{
						++far_from_sample_rays;
					}
				}
			}
			else
			{
				if (sample_material != -1)
				{
					++hit_other_material;
				}
				else
				{
					++far_from_sample_rays;
				}
			}
		}

		if (hit_other_material == 0)
		{
			// All the stencil rays strike the same material.
			// crease edge or self-occluding silhouettes.

			// (1) crease edge
			if (parameter.material)
			{
				const double threshold = umbase::um_to_radian(85.0);
				int targets[] = { 8 , 12, 16, 20 };
				for (int i = 0; i < 4; ++i)
				{
					double theta = ::acos(parameter.face_normal.dot(gradient_normals[targets[i]]));
					if (theta > threshold)
					{
						int over_count = 0;
						for (int k = 0; k < number_of_stencil_ray; ++k)
						{
							double theta2 = ::acos(parameter.face_normal.dot(gradient_normals[k]));
							if (theta2 > threshold)
							{
								++over_count;
							}
						}
						return over_count / 12.0;
					}
				}
			}

			// (2) self-occluding silhouettes.
			if (parameter.material && far_from_sample_rays > 0)
			{
				foreign_geometry_area = far_from_sample_rays / 12.0;
				return foreign_geometry_area;
			}
		}
		else
		{
			// The stencil straddles different materials.
			// a silhouette edge or an intersection line.
			foreign_geometry_area = hit_other_material / 12.0;
			return foreign_geometry_area;
		}
		return 0.0;
	}
}

namespace umrt
{

/**
 * UMToonRender implementation class
 */
class UMToonRender::Impl
{
public:
	Impl(int width, int height) 
		:  current_x_(0)
		, current_y_(0)
		, width_(width)
		, height_(height)
	{}

	virtual ~Impl()
	{
	}

	bool render(UMSceneAccessPtr scene_access, UMRenderParameter& parameter);

	bool progress_render(UMSceneAccessPtr scene_access, UMRenderParameter& parameter);
	
	/** 
	 * set client width
	 */
	void set_width(int width)
	{
		width_ = width;
	}

	/** 
	 * set client height
	 */
	void set_height(int height)
	{
		height_ = height;
	}

	bool init() 
	{
		current_x_ = 0;
		current_y_ = 0;
		return true;
	}

	OSL::RendererServices* render_service()
	{
		return NULL;
	}

private:
	// for progress render
	int current_x_;
	int current_y_;
	int width_;
	int height_;
};

bool UMToonRender::Impl::render(UMSceneAccessPtr scene_access, UMRenderParameter& parameter)
{
	umdraw::UMScenePtr scene = scene_access->scene();
	if (!scene) return false;
	if (width_ == 0 || height_ == 0) return false;
	if (!scene->camera()) return false;
	
	const int sample_count = parameter.super_sampling_count().x * parameter.super_sampling_count().y;
	const double inv_sample_count = 1.0 / sample_count;

	UMImage::ImageBuffer& dst_buffer = parameter.output_image()->mutable_list();
	
	UMPathTracer path_tracer;
	path_tracer.set_width(width_);
	path_tracer.set_height(height_);
	for (int i = 0; i < 20; ++i)
	{
		path_tracer.progress_render(scene_access, parameter);
	}

	for (int y = 0; y < height_; ++y)
	{
		//if (sample_count > 1)
		//{
		//	for (int x = 0; x < width_; ++x)
		//	{
		//		const int pos = width_ * y + x;
		//		for (int s = 0; s < sample_count; ++s)
		//		{
		//			UMVec2d sample_point(xor128d(), xor128d());
		//			sample_point.x += x;
		//			sample_point.y += y;
		//			UMRay ray;
		//			scene_access->generate_ray(ray, sample_point);
		//			UMShaderParameter shader_parameter;
		//			UMVec3d color = trace(ray, scene_access, shader_parameter);
		//			dst_buffer[pos] += UMVec4d(color, 1.0);
		//		}
		//		dst_buffer[pos] *= inv_sample_count;
		//	}
		//}
		//else
		{
			for (int x = 0; x < width_; ++x)
			{
				const int pos = width_ * y + x;
				UMVec2d pixel(x, y);
				UMRay ray;
				scene_access->generate_ray(ray, pixel);
				UMShaderParameter shader_parameter;
				UMVec3d color = trace(ray, scene_access, shader_parameter);
				//bool is_hit = shader_parameter.intersect_point != UMVec3d(0,0,0);
				//if (is_hit)
				//{
				//	// hit camera ray.
				//	dst_buffer[pos] = UMVec4d(color, 0.2);
				//}
				
				double area = trace_cone(pixel, ray, scene_access, shader_parameter);
				if (area > 0)
				{
					//if (is_hit)
					{
						dst_buffer[pos] = dst_buffer[pos].multiply(UMVec4d(UMVec3d(umbase::um_clip(1.0 - area)) , 1.0));
					}
					////else
					//{
					//	dst_buffer[pos] = UMVec4d(UMVec3d(umbase::um_clip(1.0 - area)) , 1.0);
					//}
				}
			}
		}
	}
	return true;
}

bool UMToonRender::Impl::progress_render(UMSceneAccessPtr scene_access, UMRenderParameter& parameter)
{
	umdraw::UMScenePtr scene = scene_access->scene();
	if (!scene) return false;
	if (width_ == 0 || height_ == 0) return false;
	if (!scene->camera()) return false;
	
	const int ystep = 10;
	
	const int sample_count = parameter.super_sampling_count().x * parameter.super_sampling_count().y;
	const double inv_sample_count = 1.0 / sample_count;
	
	for (int& y = current_y_, rows = (y + ystep); y < rows; ++y)
	{
		// end
		if (y == height_) { return false; }
		
		for (int x = 0; x < width_; ++x)
		{
			const int pos = width_ * y + x;
			for (int s = 0; s < sample_count; ++s)
			{
				UMVec2d sample_point(xor128d(), xor128d());
				sample_point.x += x;
				sample_point.y += y;
				UMRay ray;
				scene_access->generate_ray(ray, sample_point);
				UMShaderParameter shader_parameter;
				UMVec3d color = trace(ray, scene_access, shader_parameter);
				parameter.output_image()->mutable_list()[pos] += UMVec4d(color, 1.0);
			}
			parameter.output_image()->mutable_list()[pos] *= inv_sample_count;
		}
	}
	
	return true;
}

/**
 * constructor
 */
UMToonRender::UMToonRender()
	: impl_(new UMToonRender::Impl(width(), height()))
{}

/**
 * destructor
 */
UMToonRender::~UMToonRender()
{}

/**
 * initialize
 * @note needs a context
 */
bool UMToonRender::init() 
{
	return impl_->init();
}

/**
 * render
 */
bool UMToonRender::render(UMSceneAccessPtr scene_access, UMRenderParameter& parameter)
{
	return impl_->render(scene_access, parameter);
}

/**
 * progressive render
 */
bool UMToonRender::progress_render(UMSceneAccessPtr scene_access, UMRenderParameter& parameter)
{
	return impl_->progress_render(scene_access, parameter);
}

/** 
 * set client width
 */
void UMToonRender::set_width(int width)
{
	impl_->set_width(width);
	UMRenderer::set_width(width);
}

/** 
 * set client height
 */
void UMToonRender::set_height(int height)
{
	impl_->set_height(height);
	UMRenderer::set_height(height);
}

OSL::RendererServices* UMToonRender::render_service()
{
	return impl_->render_service();
}

} // umrt
