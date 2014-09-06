/**
 * @file UMAbcPoint.cpp
 * any
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */

#include "UMAbcPoint.h"

#include "UMAbcConvert.h"
#include "UMDirectX11.h"
#include "UMOpenGLAbcPoint.h"

namespace umabc
{
	using namespace Alembic::Abc;
	using namespace Alembic::AbcGeom;
	
/**
 * create
 */
UMAbcPointPtr UMAbcPoint::create(Alembic::AbcGeom::IPoints points)
{
	UMAbcPointPtr instance = UMAbcPointPtr(new UMAbcPoint(points));
	instance->self_reference_ = instance;
	return instance;
}

UMAbcPoint::~UMAbcPoint()
{
}

/**
 * initialize
 */
bool UMAbcPoint::init(bool recursive)
{
	if (!is_valid()) return false;
	
	// try to create colors.
	IPointsSchema &schema = points_.getSchema();
	if (const PropertyHeader *head = schema.getPropertyHeader("Cs"))
	{
		if (IC3fArrayProperty::matches(*head))
		{
			color_prop_ = IC3fArrayProperty(schema, "Cs");
		}
	}
	// try to create normals.
	if (const PropertyHeader *head = schema.getPropertyHeader("N"))
	{
		normal_prop_ = IN3fArrayProperty(schema, "N");
	}

	const size_t num_samples = schema.getNumSamples();
	if (num_samples > 0)
	{
		// get sample
		schema.get(sample_);
		
		// if not consistant, we get time
		if (!schema.isConstant())
		{
			TimeSamplingPtr time = schema.getTimeSampling();
			min_time_ = static_cast<unsigned long>(time->getSampleTime(0)*1000);
			max_time_ = static_cast<unsigned long>(time->getSampleTime(num_samples-1)*1000);
		}
	}
	
#ifdef WITH_OPENGL
	if (UMAbcPointPtr self = self_reference_.lock())
	{
		opengl_point_ = UMOpenGLAbcPointPtr(new UMOpenGLAbcPoint(self));
	}
#endif // WITH_OPENGL

	return UMAbcObject::init(recursive);
}

/**
 * set_current_time
 */
void UMAbcPoint::set_current_time(unsigned long time, bool recursive)
{
	if (!is_valid()) return;
	
	UMAbcObject::set_current_time(time, recursive);

	update_point_all();
}

/**
 * update box
 */
void UMAbcPoint::update_box(bool recursive)
{
	box_.init();

	if (initial_bounds_prop_ && initial_bounds_prop_.getNumSamples() > 0)
	{
		ISampleSelector selector(current_time(), ISampleSelector::kNearIndex);

		box_ = 
			UMAbcConvert::imath_box_to_um(
				initial_bounds_prop_.getValue(selector));
	}

	if (box_.is_empty() && positions_)
	{
		size_t point_count = positions_->size();
		for (size_t i = 0; i < point_count; ++i)
		{
			const Imath::V3f& p = (*positions_)[i];
			box_.extend(UMAbcConvert::imath_vec_to_um(p));
		}
	}
}

/** 
 * update color
 */
void UMAbcPoint::update_color()
{
	if (color_prop_)
	{
		ISampleSelector selector(current_time(), ISampleSelector::kNearIndex);
		colors_ = color_prop_.getValue(selector);
	}
}

/** 
 * update normal
 */
void UMAbcPoint::update_normal()
{
	if (normal_prop_)
	{
		ISampleSelector selector(current_time(), ISampleSelector::kNearIndex);
		normals_ = normal_prop_.getValue(selector);
	}
}

void UMAbcPoint::update_point()
{
	if (!is_valid()) return;
	ISampleSelector selector(current_time(), ISampleSelector::kNearIndex);
	IPointsSchema::Sample sample;
	points_.getSchema().get(sample, selector);
	positions_ = sample.getPositions();
	
#ifdef WITH_OPENGL
	if (opengl_point_ && !umdraw::UMDirectX11::current_device_pointer())
	{
		opengl_point_->update_vertex(positions_);
	}
#endif // WITH_OPENGL

}

/** 
 * update point all
 */
void UMAbcPoint::update_point_all()
{
	if (!is_valid()) return;
	update_point();
	update_color();
	update_normal();
}

void UMAbcPoint::draw(bool recursive, UMAbc::DrawType type)
{
	if (!is_valid()) return;
	if (!positions_) return;
	
#ifdef WITH_OPENGL
	if (type == UMAbc::eOpenGL && !umdraw::UMDirectX11::current_device_pointer())
	{
		if (opengl_point_)
		{
			opengl_point_->draw();
		}
	}
#endif // WITH_OPENGL
}


}

