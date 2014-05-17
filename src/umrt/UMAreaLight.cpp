/**
 * @file UMAreaLight.cpp
 * an area light
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#include "UMAreaLight.h"
#include "UMVector.h"
#include "UMRay.h"
#include "UMShaderParameter.h"
#include "UMLight.h"

#include <random>

namespace umrt
{

/**
 * get intensity from previous sample
 */
UMVec3d UMAreaLight::intensity(
	umdraw::UMLightPtr light,
	const UMVec3d& point)
{
	if (UMAreaLightPtr area_light = std::dynamic_pointer_cast<UMAreaLight>(light))
	{
		double r = 0;
		if (area_light->linear_fall_off_ != 0 || area_light->quadric_fall_off_ != 0)
		{
			r = (area_light->last_sample_point_ - point).length();
		}
		double constant = area_light->constant_fall_off_;
		double linear = area_light->linear_fall_off_ * r;
		double quadric = area_light->quadric_fall_off_ * r * r;
		return area_light->color() * area_light->area_ / (constant + linear + quadric);
	}
	UMVec3d zero(0);
	return zero;
}

/** 
 * sample a point
 */
bool UMAreaLight::sample(
	UMVec3d& intensity, 
	UMVec3d& point, 
	UMVec3d& direction, 
	umdraw::UMLightPtr light,
	const UMShaderParameter& parameter, 
	const UMVec2d& random_value)
{
	if (UMAreaLightPtr area_light = std::dynamic_pointer_cast<UMAreaLight>(light))
	{
		UMVec3d sample_point(
			area_light->edge1_ * random_value.x + 
			area_light->edge2_ * random_value.y + area_light->position());
		direction = sample_point - parameter.intersect_point;
		double direction_length_inv = 1.0 / direction.length();
		double cos_theta_in = std::max( parameter.normal.dot(direction) * direction_length_inv, 0.0 );
		double cos_theta_out = std::max( area_light->normal_.dot(-direction) * direction_length_inv, 0.0 );
		double factor = cos_theta_in * cos_theta_out * direction_length_inv * direction_length_inv;
		intensity = area_light->color() * factor * area_light->area_;
		point = sample_point;

		// save sample point
		area_light->last_sample_point_ = sample_point;
		return true;
	}
	return false;
}


}