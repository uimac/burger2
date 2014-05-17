/**
 * @file UMGUIObject.h
 * any
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#pragma once

#include <memory>
#include <string>

#include "UMMacro.h"
#include "UMBox.h"
#include "UMMathTypes.h"
#include "UMNode.h"
#include "UMGUI.h"

namespace umgui
{

class UMGUIObject;
typedef std::shared_ptr<UMGUIObject> UMGUIObjectPtr;
typedef std::weak_ptr<UMGUIObject> UMGUIObjectWeakPtr;
typedef std::vector<UMGUIObjectPtr> UMGUIObjectList;

class UMGUIObject : public umdraw::UMNode
{
	DISALLOW_COPY_AND_ASSIGN(UMGUIObject);
public:
	UMGUIObject() {}

	~UMGUIObject() {}
	
	/**
	 * initialize
	 * @param [in] recursive do children recursively
	 */
	virtual bool init(bool recursive);
	
	/**
	 * is valid or not
	 */
	virtual bool is_valid() const { return true; }

	/**
	 * is visible or not
	 */
	virtual bool is_visible() const;

	/**
	 * get bounding box
	 */
	virtual const UMBox& box() const { return box_; }
	
	/**
	 * get no inherit bounding box
	 */
	virtual const UMBox& no_inherit_box() const { return no_inherit_box_; }

	/**
	 * update box
	 * @param [in] recursive do children recursively
	 */
	virtual void update_box(bool recursive);

	/**
	 * draw
	 */
	virtual void draw(bool recursive, UMGUI::DrawType type);
	
	/**
	 * update
	 */
	virtual bool update(bool recursive);

	/**
	 * get children
	 */
	const UMGUIObjectList& children() const { return children_; }
	
	/**
	 * get children
	 */
	UMGUIObjectList& mutable_children() { return children_; }

	/**
	 * get name
	 */
	const std::string& name() const { return name_; }

protected:
	std::string name_;
	UMBox box_;
	UMBox no_inherit_box_;
	UMGUIObjectList children_;

};

} // umgui
