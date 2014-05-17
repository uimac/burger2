/**
 * @file UMRay.h
 * a ray
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#pragma once

#include "UMMacro.h"
#include "UMMathTypes.h"
#include "UMVector.h"

/// Uimac raytracing library
namespace umrt
{

/**
 * a ray
 */
class UMRay
{
	DISALLOW_COPY_AND_ASSIGN(UMRay);

public:
	UMRay() :
		origin_(0),
		direction_(0),
		tmin_(FLT_EPSILON),
		tmax_(FLT_MAX)
	{}
	
	/**
	 * @param [in] origin origin
	 * @param [in] direction direction
	 */
	UMRay(const UMVec3d& origin, const UMVec3d& direction) :
		origin_(origin),
		direction_(direction),
		tmin_(FLT_EPSILON),
		tmax_(FLT_MAX) {}

	~UMRay() {}

	/**
	 * get origin
	 */
	const UMVec3d& origin() const { return origin_; }
	
	/**
	 * set origin
	 * @param [in] origin source origin
	 */
	void set_origin(const UMVec3d& origin) { origin_ = origin; }

	/**
	 * get direction
	 */
	const UMVec3d& direction() const { return direction_; }
	
	/**
	 * set direction
	 * @param [in] direction source direction
	 */
	void set_direction(const UMVec3d& direction) { direction_ = direction; }
	
	/**
	 * get tmin
	 */
	double tmin() const { return tmin_; }

	/**
	 * get tmin
	 */
	void set_tmin(double tmin) { tmin_ = tmin; }
	
	/**
	 * get tmax
	 */
	double tmax() const { return tmax_; }
	
	/**
	 * get tmax
	 */
	void set_tmax(double tmax) { tmax_ = tmax; }

private:
	UMVec3d origin_;
	UMVec3d direction_;
	double tmin_;
	double tmax_;
};

} // umrt
