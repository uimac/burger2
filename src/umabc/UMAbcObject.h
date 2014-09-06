/**
 * @file UMAbcObject.h
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
#include <Alembic/Abc/All.h>
#include <Alembic/AbcGeom/All.h>
#include <Alembic/AbcMaterial/All.h>
#include <Alembic/AbcCoreFactory/All.h>
#include <string>

#include "UMMacro.h"
#include "UMBox.h"
#include "UMMathTypes.h"
#include "UMAbcCallback.h"
#include "UMAbc.h"
#include "UMNode.h"

namespace umabc
{

class UMAbcObject;
typedef std::shared_ptr<UMAbcObject> UMAbcObjectPtr;
typedef std::weak_ptr<UMAbcObject> UMAbcObjectWeakPtr;
typedef std::vector<UMAbcObjectPtr> UMAbcObjectList;

class UMAbcObject : public umdraw::UMNode
{
	DISALLOW_COPY_AND_ASSIGN(UMAbcObject);
public:
	UMAbcObject(Alembic::Abc::IObject object) 
		: object_(object)
		, min_time_(0)
		, max_time_(0)
		, current_time_(0)
	{}

	~UMAbcObject() {}
	
	/**
	 * initialize
	 * @param [in] recursive do children recursively
	 */
	virtual bool init(bool recursive);

	/**
	 * get minumum time
	 */
	virtual unsigned long min_time() const { return min_time_; }
	
	/**
	 * get maximum time
	 */
	virtual unsigned long max_time() const { return max_time_; }

	/**
	 * set current time
	 */
	virtual void set_current_time(unsigned long time, bool recursive);

	/**
	 * get current time
	 */
	double current_time() const { return static_cast<double>(current_time_ / 1000.0); }
	/**
	 * get current time
	 */
	unsigned int current_time_ms() const { return current_time_ ; }

	/**
	 * is valid or not
	 */
	virtual bool is_valid() const { return object_ ? object_.valid() : false; }

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
	virtual void draw(bool recursive, UMAbc::DrawType type);

	/**
	 * get children
	 */
	const UMAbcObjectList& children() const { return children_; }
	
	/**
	 * get children
	 */
	UMAbcObjectList& mutable_children() { return children_; }

	/**
	 * get parent
	 */
	UMAbcObjectPtr parent() { return parent_.lock(); }

	/**
	 * get name
	 */
	const std::string& name() const { return name_; }

protected:
	virtual UMAbcObjectPtr self_reference() { return UMAbcObjectPtr(); }

	Alembic::Abc::IObject object_;

	std::string name_;

	unsigned long min_time_;
	unsigned long max_time_;
	unsigned long current_time_;

	UMBox box_;
	UMBox no_inherit_box_;
	UMAbcObjectList children_;
	UMAbcObjectWeakPtr parent_;

};

} // umabc
