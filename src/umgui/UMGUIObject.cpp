/**
 * @file UMGUIObject.cpp
 * any
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#include "UMGUIObject.h"
#include "UMGUIBoard.h"
#include "UMEvent.h"
#include "UMGUIEventType.h"

#include <memory>

namespace umgui
{

UMGUIObject::UMGUIObject()
	: is_visible_(true)
	, is_root_(false)
	, is_node_(false)
	, update_event_(std::make_shared<umbase::UMEvent>(eGUIEventObjectUpdated))
{
}

UMGUIObject::~UMGUIObject()
{
}

/**
 * initialize
 */
bool UMGUIObject::init(bool recursive)
{
	if (!is_valid()) return false;
	if (!recursive) return true;

	if (!children_.empty())
	{
		children_.clear();
	}
	
	return true;
}

/**
 * update box
 */
void UMGUIObject::update_box(bool recursive)
{
	umbase::UMMat44d local = mutable_local_transform();
	local.m[3][1] *= -1;
	box_.set_minimum(local * initial_box_.minimum());
	box_.set_maximum(local * initial_box_.maximum());
	UMGUIObjectList::iterator it = children_.begin();
	if (recursive)
	{
		for (; it != children_.end(); ++it)
		{
			UMGUIObjectPtr child = *it;
			child->update_box(recursive);
	//	box_.extend(child->box());
		}
	}
}

/**
 * is visible
 */
bool UMGUIObject::is_visible() const
{
	return is_visible_;
}

/**
 * draw
 */
void UMGUIObject::draw(bool recursive, UMGUI::DrawType type)
{
	if (!is_valid()) return;
	//if (!is_visible()) return;
	
	if (recursive)
	{
		UMGUIObjectList::const_iterator it = children_.begin();
		for (; it != children_.end(); ++it)
		{
			UMGUIObjectPtr child = *it;
			child->draw(recursive, type);
		}
	}
}

/**
 * update
 */
bool UMGUIObject::update(bool recursive)
{
	if (!is_valid()) return true;
	
	UMGUIObjectList::const_iterator it = children_.begin();
	for (; it != children_.end(); ++it)
	{
		UMGUIObjectPtr child = *it;
		if (recursive)
		{
			child->update(recursive);
		}
	}
	update_event_->notify();
	return true;
}

/**
 * intersect
 */
void UMGUIObject::intersect(UMGUIObjectPtr object, UMGUIObjectList& intersect_list, double x, double y)
{
	if (!object->is_valid()) return;
	
	if (object->intersect(x, y))
	{
		intersect_list.push_back(object);
	}

	UMGUIObjectList::const_iterator it = object->children().begin();
	for (; it != object->children().end(); ++it)
	{
		UMGUIObjectPtr child = *it;
		UMGUIObject::intersect(child, intersect_list, x, y);
	}
}

/**
 * intersect
 */
bool UMGUIObject::intersect(double x, double y) const
{
	if (!is_valid()) return false;
	
	if (x >= box_.minimum().x 
		&& x < box_.maximum().x
		&& y >= box_.minimum().y
		&& y < box_.maximum().y)
	{
		return true;
	}
	return false;
}

} // umgui
