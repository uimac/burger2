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
#include "UMStringUtil.h"

#include <limits>
#include <algorithm>
#include <random>
#include <utility>

#ifdef WITH_OSL
	#include <OSL/oslexec.h>
#endif

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
				radiance += parameter.color * std::max(0.0, normal.dot(L));
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

}

namespace umrt
{
	
#ifdef WITH_OSL
	using namespace OSL;

/**
 * OSLRenderService implementation class
 */
class UMOSLRenderService : public OSL::RendererServices
{
public:
	UMOSLRenderService() {}
	virtual ~UMOSLRenderService() {}
	
	/// Get the 4x4 matrix that transforms by the specified
	/// transformation at the given time.  Return true if ok, false
	/// on error.
	virtual bool get_matrix (Matrix44 &result, TransformationPtr xform, float time)
	{
		return false;
	}
	
	/// Get the 4x4 matrix that transforms by the specified
	/// transformation at the given time.  Return true if ok, false on
	/// error.  The default implementation is to use get_matrix and
	/// invert it, but a particular renderer may have a better technique
	/// and overload the implementation.
	virtual bool get_inverse_matrix (Matrix44 &result, TransformationPtr xform, float time)
	{
		return false;
	}

	/// Get the 4x4 matrix that transforms by the specified
	/// transformation.  Return true if ok, false on error.  Since no
	/// time value is given, also return false if the transformation may
	/// be time-varying.
	virtual bool get_matrix (Matrix44 &result, TransformationPtr xform) 
	{
		return false;
	}
	
	/// Get the 4x4 matrix that transforms by the specified
	/// transformation.  Return true if ok, false on error.  Since no
	/// time value is given, also return false if the transformation may
	/// be time-varying.  The default implementation is to use
	/// get_matrix and invert it, but a particular renderer may have a
	/// better technique and overload the implementation.
	virtual bool get_inverse_matrix (Matrix44 &result, TransformationPtr xform)
	{
		return false;
	}

	/// Get the 4x4 matrix that transforms points from the named
	/// 'from' coordinate system to "common" space at the given time.
	/// Returns true if ok, false if the named matrix is not known.
	virtual bool get_matrix (Matrix44 &result, ustring from, float time)
	{
		return false;
	}
	
	/// Get the 4x4 matrix that transforms 'from' to "common" space.
	/// Since there is no time value passed, return false if the
	/// transformation may be time-varying (as well as if it's not found
	/// at all).
	virtual bool get_matrix (Matrix44 &result, ustring from)
	{
		return false;
	}

	/// Get the named attribute from the renderer and if found then
	/// write it into 'val'.  Otherwise, return false.  If no object is
	/// specified (object == ustring()), then the renderer should search *first*
	/// for the attribute on the currently shaded object, and next, if
	/// unsuccessful, on the currently shaded "scene".
	///
	/// Note to renderers: if renderstate is NULL, that means
	/// get_attribute is being called speculatively by the runtime
	/// optimizer, and it doesn't know which object the shader will be
	/// run on. Be robust to this situation, return 'true' (retrieve the
	/// attribute) if you can (known object and attribute name), but
	/// otherwise just fail by returning 'false'.
	virtual bool get_attribute (
		void *renderstate, 
		bool derivatives,
		ustring object, 
		TypeDesc type, 
		ustring name,
		void *val )
	{
		return false;
	}

	/// Similar to get_attribute();  this method will return the 'index'
	/// element of an attribute array.
	virtual bool get_array_attribute (
		void *renderstate, 
		bool derivatives,
		ustring object, 
		TypeDesc type,
		ustring name, 
		int index, 
		void *val )
	{
		return false;
	}

	/// Get the named user-data from the current object and write it into
	/// 'val'. If derivatives is true, the derivatives should be written into val
	/// as well. Return false if no user-data with the given name and type was
	/// found.
	virtual bool get_userdata (
		bool derivatives, 
		ustring name, 
		TypeDesc type,
		void *renderstate, 
		void *val)
	{
		return false;
	}

	/// Does the current object have the named user-data associated with it?
	virtual bool has_userdata (ustring name, TypeDesc type, void *renderstate)
	{
		return false;
	}
};
#endif

/**
 * UMRayTracer implementation class
 */
class UMRayTracer::Impl
{
public:
	Impl(int width, int height) 
		:  current_x_(0)
		, current_y_(0)
		, width_(width)
		, height_(height)
#ifdef WITH_OSL
		, render_service_(new UMOSLRenderService())
#else
		, render_service_(NULL)
#endif
	{}

	virtual ~Impl()
	{
#ifdef WITH_OSL
		delete render_service_;
#endif
		render_service_ = NULL;
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
		return render_service_;
	}

private:
	OSL::RendererServices* render_service_;
	// for progress render
	int current_x_;
	int current_y_;
	int width_;
	int height_;
};

#ifdef WITH_OSL
static void setup_shader_globals(
	UMRenderParameter& parameter,
	OSL::ShaderGlobals& sg,
	OSL::ShadingSystem* shading_system,
	int x,
	int y)
{
	const int w = parameter.output_image()->width();
	const int h = parameter.output_image()->height();

	sg.u = static_cast<float>(x + 0.5) / w;
	sg.v = static_cast<float>(y + 0.5) / h;
	sg.dudx = 1.0f / w;
	sg.dvdy = 1.0f / h;

	sg.P = Imath::V3f(sg.u, sg.v, 1.0f);

	sg.dPdx = Imath::V3f(sg.dudx, sg.dudy, 0.0f);
	sg.dPdy = Imath::V3f(sg.dudx, sg.dudy, 0.0f);
	sg.dPdz = Imath::V3f(0.0f);

	sg.dPdu = Imath::V3f(1.0f, 0.0f, 0.0f);
	sg.dPdu = Imath::V3f(0.0f, 1.0f, 0.0f);

	sg.N = Imath::V3f(0, 0, 1);
	sg.Ng = Imath::V3f(0, 0, 1);

	sg.surfacearea = 1;
	sg.Ci = NULL;
}
#endif 

bool UMRayTracer::Impl::render(UMSceneAccessPtr scene_access, UMRenderParameter& parameter)
{
	umdraw::UMScenePtr scene = scene_access->scene();
	if (!scene) return false;
	if (width_ == 0 || height_ == 0) return false;
	if (!scene->camera()) return false;
	
	//OSL::ErrorHandler error_handler;
	//OSL::TextureSystem* texture_system = render_service()->texturesys();
	//OSL::ShadingSystem* shading_system = OSL::ShadingSystem::create(render_service(), texture_system, &error_handler);
	//shading_system->attribute("hoge", "moga");

	//shading_system->ShaderGroupBegin();
	//umstring oslpath = parameter.osl_filepath();
	//shading_system->Shader("surface", umbase::UMStringUtil::utf16_to_utf8(oslpath).c_str(), NULL);
	//shading_system->ShaderGroupEnd();

	//OSL::ShadingAttribStateRef shading_state = shading_system->state();
	//// create shading context
	//OSL::PerThreadInfo *thread_info = shading_system->create_thread_info();
	//OSL::ShadingContext *shading_context = shading_system->get_context(thread_info);

	//// setup global input variables to shader
	//OSL::ShaderGlobals shader_globals;
	//setup_shader_globals(parameter, shader_globals, shading_system, 0, 0);

	//if (shading_system->execute(*shading_context, *shading_state, shader_globals))
	//{
	//	std::cout << "shader executed success!!" << std::endl;
	//}
	//else
	//{
	//	std::cout << "shader filed to execute" << std::endl;
	//}

	//shading_state = OSL::ShadingAttribStateRef();
	//OSL::TextureSystem::destroy(texture_system, true);
	//shading_system->release_context(shading_context);
	//shading_system->destroy_thread_info(thread_info);
	//thread_info = NULL;
	//OSL::ShadingSystem::destroy(shading_system);
	//shading_system = NULL;

	const int sample_count = parameter.super_sampling_count().x * parameter.super_sampling_count().y;
	const double inv_sample_count = 1.0 / sample_count;

	UMImage::ImageBuffer& dst_buffer = parameter.output_image()->mutable_list();
	
//#pragma omp parallel for schedule(dynamic, 1) num_threads(4)
	for (int y = 0; y < height_; ++y)
	{
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
					UMRay ray;
					scene_access->generate_ray(ray, sample_point);
					UMShaderParameter shader_parameter;
					UMVec3d color = trace(ray, scene_access, shader_parameter);
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
				UMRay ray;
				scene_access->generate_ray(ray, UMVec2d(x, y));
				UMShaderParameter shader_parameter;
				UMVec3d color = trace(ray, scene_access, shader_parameter);
				dst_buffer[pos] = UMVec4d(color, 1.0);
			}
		}
	}
	return true;
}

bool UMRayTracer::Impl::progress_render(UMSceneAccessPtr scene_access, UMRenderParameter& parameter)
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
UMRayTracer::UMRayTracer()
	: impl_(new UMRayTracer::Impl(width(), height()))
{}

/**
 * destructor
 */
UMRayTracer::~UMRayTracer()
{}

/**
 * initialize
 * @note needs a context
 */
bool UMRayTracer::init() 
{
	return impl_->init();
}

/**
 * render
 */
bool UMRayTracer::render(UMSceneAccessPtr scene_access, UMRenderParameter& parameter)
{
	return impl_->render(scene_access, parameter);
}

/**
 * progressive render
 */
bool UMRayTracer::progress_render(UMSceneAccessPtr scene_access, UMRenderParameter& parameter)
{
	return impl_->progress_render(scene_access, parameter);
}

/** 
 * set client width
 */
void UMRayTracer::set_width(int width)
{
	impl_->set_width(width);
	UMRenderer::set_width(width);
}

/** 
 * set client height
 */
void UMRayTracer::set_height(int height)
{
	impl_->set_height(height);
	UMRenderer::set_height(height);
}

OSL::RendererServices* UMRayTracer::render_service()
{
	return impl_->render_service();
}

} // umrt
