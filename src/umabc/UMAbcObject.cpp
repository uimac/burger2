/**
 * @file UMAbcObject.cpp
 * any
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#include "UMAbcObject.h"
#include "UMAbcMesh.h"
#include "UMAbcXform.h"
#include "UMAbcPoint.h"
#include "UMAbcCurve.h"
#include "UMAbcNurbsPatch.h"
#include "UMAbcCamera.h"
#include <memory>

namespace umabc
{

	using namespace Alembic::Abc;
	using namespace Alembic::AbcGeom;

/**
 * initialize
 */
bool UMAbcObject::init(bool recursive)
{
	if (!is_valid()) return false;
	if (!recursive) return true;

	name_ = object_.getName();

	if (!children_.empty())
	{
		children_.clear();
	}

	const size_t child_count = object_.getNumChildren();
	for (size_t i = 0; i < child_count; ++i)
	{
		const ObjectHeader &ohead = object_.getChildHeader(i);
		UMAbcObjectPtr child;
		if (IPolyMesh::matches(ohead))
		{
			IPolyMesh mesh(object_, ohead.getName());
			child = UMAbcMesh::create(mesh);
			child->parent_ = self_reference();
			children_.push_back(child);
		}
		else if (IPoints::matches(ohead))
		{
			IPoints points(object_, ohead.getName());
			child = UMAbcPoint::create(points);
			child->parent_ = self_reference();
			children_.push_back(child);
		}
		else if (ICurves::matches(ohead))
		{
			ICurves curves(object_, ohead.getName());
			child = UMAbcCurve::create(curves);
			child->parent_ = self_reference();
			children_.push_back(child);
		}
		else if (INuPatch::matches(ohead))
		{
			INuPatch patch(object_, ohead.getName());
			child = UMAbcNurbsPatch::create(patch);
			child->parent_ = self_reference();
			children_.push_back(child);
		}
		else if (IXform::matches(ohead))
		{
			IXform xform(object_, ohead.getName());
			child = UMAbcXform::create(xform);
			child->parent_ = self_reference();
			children_.push_back(child);
		}
		else if (ICamera::matches(ohead))
		{
			ICamera camera(object_, ohead.getName());
			child = UMAbcCamera::create(camera);
			child->parent_ = self_reference();
			children_.push_back(child);
		}
		// recursive
		if (child && recursive)
		{
			child->init(recursive);
		}

		// set time
		if (child && child->is_valid())
		{
			min_time_ = std::min( min_time_, child->min_time() );
			max_time_ = std::max( max_time_, child->max_time() );
		}
	}
	return true;
}

/**
 * set current time
 */
void UMAbcObject::set_current_time(unsigned long time, bool recursive)
{
	if (!object_) return;

	current_time_ = time;

	if (recursive)
	{
		UMAbcObjectList::iterator it = children_.begin();
		for (; it != children_.end(); ++it)
		{
			UMAbcObjectPtr child = *it;
			child->set_current_time(time, recursive);
		}
	}
}

/**
 * update box
 */
void UMAbcObject::update_box(bool recursive)
{
	box_.init();
	UMAbcObjectList::iterator it = children_.begin();
	for (; it != children_.end(); ++it)
	{
		UMAbcObjectPtr child = *it;
		if (recursive)
		{
			child->update_box(recursive);
		}
		box_.extend(child->box());
	}
}

/**
 * is visible
 */
bool UMAbcObject::is_visible() const
{
	Alembic::Abc::ICompoundProperty props = object_.getProperties();
	const Alembic::Abc::PropertyHeader* header = props.getPropertyHeader("visible");
	if (header)
	{
		Alembic::Abc::IScalarProperty visible(props, "visible");
		Alembic::Abc::ISampleSelector selector(current_time());
		Alembic::Abc::int8_t is_visible = 1;
		visible.get(reinterpret_cast<void*>(&is_visible), selector);
		return is_visible == 1;
	}
	return false;
}

/**
 * draw
 */
void UMAbcObject::draw(bool recursive, UMAbc::DrawType type)
{
	if (!is_valid()) return;
	//if (!is_visible()) return;
	
	UMAbcObjectList::const_iterator it = children_.begin();
	for (; it != children_.end(); ++it)
	{
		UMAbcObjectPtr child = *it;
		if (recursive)
		{
			child->draw(recursive, type);
		}
	}
}

} // umabc
