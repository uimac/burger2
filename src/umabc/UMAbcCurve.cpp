/**
 * @file UMAbcCurve.cpp
 * any
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */

#include "UMAbcCurve.h"
#include "UMAbcConvert.h"

namespace umabc
{
	using namespace Alembic::Abc;
	using namespace Alembic::AbcGeom;
	
/**
 * create
 */
UMAbcCurvePtr UMAbcCurve::create(Alembic::AbcGeom::ICurves curves)
{
	UMAbcCurvePtr instance = UMAbcCurvePtr(new UMAbcCurve(curves));
	instance->self_reference_ = instance;
	return instance;
}

UMAbcCurve::~UMAbcCurve()
{
}

/**
 * initialize
 */
bool UMAbcCurve::init(bool recursive)
{
	if (!is_valid()) return false;
	
	const size_t num_samples = curves_.getSchema().getNumSamples();
	if (num_samples > 0)
	{
		// get sample
		curves_.getSchema().get(initial_sample_);
		
		// if not consistant, we get time
		if (!curves_.getSchema().isConstant())
		{
			TimeSamplingPtr time = curves_.getSchema().getTimeSampling();
			min_time_ = static_cast<unsigned long>(time->getSampleTime(0)*1000);
			max_time_ = static_cast<unsigned long>(time->getSampleTime(num_samples-1)*1000);
		}
	}

	return UMAbcObject::init(recursive);
}

/**
 * set current time
 */
void UMAbcCurve::set_current_time(unsigned long time, bool recursive)
{
	if (!is_valid()) return;
	
	UMAbcObject::set_current_time(time, recursive);

	update_curve_all();
}

/**
 * update curve all
 */
void UMAbcCurve::update_curve_all()
{
	if (!is_valid()) return;
	if (curves_.getSchema().getNumSamples() <= 0) return;

	ISampleSelector selector(current_time(), ISampleSelector::kNearIndex);
	ICurvesSchema::Sample sample;
	curves_.getSchema().get(sample, selector);

	positions_ = sample.getPositions();
	curve_count_ = sample.getNumCurves();
	vertex_count_ = sample.getCurvesNumVertices();

}

/**
 * update box
 */
void UMAbcCurve::update_box(bool recursive)
{
	if (!is_valid()) return;
	box_.init();
	
	if (curves_.getSchema().getNumSamples() <= 0) return;
	ISampleSelector selector(current_time(), ISampleSelector::kNearIndex);
	ICurvesSchema::Sample sample;
	curves_.getSchema().get(sample, selector);

	box_.extend(UMAbcConvert::imath_box_to_um(sample.getSelfBounds()));
}

void UMAbcCurve::draw(bool recursive, UMAbc::DrawType type)
{
	if (!is_valid()) return;
	
	UMAbcObject::draw(recursive, type);
}


}

