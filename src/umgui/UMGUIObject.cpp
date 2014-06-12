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

#include <memory>

namespace umgui
{

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
	box_.init();
	UMGUIObjectList::iterator it = children_.begin();
	for (; it != children_.end(); ++it)
	{
		UMGUIObjectPtr child = *it;
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
bool UMGUIObject::is_visible() const
{
	return true;
}

/**
 * draw
 */
void UMGUIObject::draw(bool recursive, UMGUI::DrawType type)
{
	if (!is_valid()) return;
	//if (!is_visible()) return;
	
	UMGUIObjectList::const_iterator it = children_.begin();
	for (; it != children_.end(); ++it)
	{
		UMGUIObjectPtr child = *it;
		if (recursive)
		{
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
	return true;
}


} // umgui
