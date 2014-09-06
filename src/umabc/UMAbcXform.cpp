/**
 * @file UMAbcXform.cpp
 * any
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#include "UMAbcXform.h"
#include "UMAbcConvert.h"

namespace umabc
{
	
	using namespace Alembic::Abc;
	using namespace Alembic::AbcGeom;

/**
 * create
 */
UMAbcXformPtr UMAbcXform::create(Alembic::AbcGeom::IXform xform)
{
	UMAbcXformPtr instance = UMAbcXformPtr(new UMAbcXform(xform));
	instance->self_reference_ = instance;
	return instance;
}

/**
 * initialize
 */
bool UMAbcXform::init(bool recursive)
{
	if (!is_valid()) return false;
	
	mutable_local_transform().identity();
	static_matrix_.identity();

	if (xform_.getSchema().isConstant())
	{
		static_matrix_ = 
			UMAbcConvert::imath_mat_to_um(
				xform_.getSchema().getValue().getMatrix());
	}
	else
	{
		// not consistant.
		// we get time
		size_t num_samples = xform_.getSchema().getNumSamples();
		if (num_samples > 0)
		{
			TimeSamplingPtr time = xform_.getSchema().getTimeSampling();
			min_time_ = static_cast<unsigned long>(time->getSampleTime(0)*1000);
			max_time_ = static_cast<unsigned long>(time->getSampleTime(num_samples-1)*1000);
		}
	}

	return UMAbcObject::init(recursive);
}

/**
 * set current time
 */
void UMAbcXform::set_current_time(unsigned long time, bool recursive)
{
	if (!is_valid()) {
		mutable_local_transform().identity();
		return;
	}
	
	if (min_time() < time && time < max_time())
	{
		ISampleSelector selector(time / 1000.0, ISampleSelector::kNearIndex);
		is_inherit_ = xform_.getSchema().getInheritsXforms(selector);

		if (xform_.getSchema().isConstant())
		{
			mutable_local_transform() = static_matrix_;
		}
		else
		{
			mutable_local_transform() = 
				UMAbcConvert::imath_mat_to_um(
					xform_.getSchema().getValue(selector).getMatrix());
		}
	}

	UMAbcObject::set_current_time(time, recursive);
}

/**
 * update box
 */
void UMAbcXform::update_box(bool recursive)
{
	UMAbcObject::update_box(recursive);

	box_.init();
	no_inherit_box_.init();

	UMAbcObjectList::iterator it = children_.begin();
	for (; it != children_.end(); ++it)
	{
		UMAbcObjectPtr child = *it;
		if (!child->box().is_empty())
		{
			const UMBox& child_box = child->box();
			child_box.transformed(local_transform());
			mutable_global_transform() = mutable_global_transform() * local_transform();

			if (!child_box.is_empty())
			{
				if (is_inherit_)
				{
					box_.extend(child_box);
				}
				else
				{
					no_inherit_box_.extend(child_box);
				}
			}

			const UMBox& child_no_inherit_box = child->no_inherit_box();
			if (!child_no_inherit_box.is_empty())
			{
				no_inherit_box_.extend(child_no_inherit_box);
			}
		}
	}
}

/**
 * refresh
 */
void UMAbcXform::draw(bool recursive, UMAbc::DrawType type)
{
	UMAbcObject::draw(recursive, type);
}

} // umabc
