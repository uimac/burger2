/**
 * @file UMAbcNurbsPatch.cpp
 * any
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */

#include "UMAbcNurbsPatch.h"

#include "UMAbcConvert.h"

namespace umabc
{
	using namespace Alembic::Abc;
	using namespace Alembic::AbcGeom;
	
/**
 * create
 */
UMAbcNurbsPatchPtr UMAbcNurbsPatch::create(Alembic::AbcGeom::INuPatch patch)
{
	UMAbcNurbsPatchPtr instance = UMAbcNurbsPatchPtr(new UMAbcNurbsPatch(patch));
	instance->self_reference_ = instance;
	return instance;
}

UMAbcNurbsPatch::~UMAbcNurbsPatch()
{
}

/**
 * init
 */
bool UMAbcNurbsPatch::init(bool recursive)
{
	if (!is_valid()) return false;
	
	// // create our nurb renderer.
	// nurb = gluNewNurbsRenderer();

	// gluNurbsProperty(nurb, GLU_SAMPLING_TOLERANCE, 25.0);
	// gluNurbsProperty(nurb, GLU_DISPLAY_MODE, GLU_FILL);

	size_t num_samples = patch_.getSchema().getNumSamples();
	if (num_samples > 0)
	{
		// get constant sample
		patch_.getSchema().get(initial_sample_);
		
		// if not consistant, we get time
		if (!patch_.getSchema().isConstant())
		{
			TimeSamplingPtr time = patch_.getSchema().getTimeSampling();
			min_time_ = static_cast<unsigned long>(time->getSampleTime(0)*1000);
			max_time_ = static_cast<unsigned long>(time->getSampleTime(num_samples-1)*1000);
		}
	}

	return false;
}

/**
 * set_current_time
 */
void UMAbcNurbsPatch::set_current_time(unsigned long time, bool recursive)
{
	if (!is_valid()) return;
	
	UMAbcObject::set_current_time(time, recursive);

	update_patch_all();
}

/**
 * update patch all
 */
void UMAbcNurbsPatch::update_patch_all()
{
	if (!is_valid()) return;
	ISampleSelector selector(current_time(), ISampleSelector::kNearIndex);
	INuPatchSchema::Sample sample;
	patch_.getSchema().get(sample, selector);

	positions_ = sample.getPositions();
	u_knot_ = sample.getUKnot();
	v_knot_ = sample.getVKnot();
	u_size_ = sample.getNumU();
	v_size_ = sample.getNumV();
	u_order_ = sample.getUOrder();
	v_order_ = sample.getVOrder();
}

/**
 * update box
 */
void UMAbcNurbsPatch::update_box(bool recursive)
{
	if (!is_valid()) return;

	box_.init();
	if (positions_)
	{
		size_t position_count = positions_->size();
		for (size_t i = 0; i < position_count; ++i)
		{
			const Imath::V3f& p = (*positions_)[i];
			box_.extend(UMAbcConvert::imath_vec_to_um(p));
		}
	}
}

/**
 * refresh
 */
void UMAbcNurbsPatch::draw(bool recursive, UMAbc::DrawType type)
{
	if (!is_valid()) return;

	UMAbcObject::draw(recursive, type);
}


}

