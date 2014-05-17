/**
 * @file UMAbcXform.h
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
#include <Alembic/AbcCoreFactory/All.h>

#include "UMMacro.h"
#include "UMAbcObject.h"

namespace umabc
{

class UMAbcXform;
typedef std::shared_ptr<UMAbcXform> UMAbcXformPtr;
typedef std::weak_ptr<UMAbcXform> UMAbcXformWeakPtr;

class UMAbcXform : public UMAbcObject
{
	DISALLOW_COPY_AND_ASSIGN(UMAbcXform);
public:
	static UMAbcXformPtr create(Alembic::AbcGeom::IXform xform);

	~UMAbcXform() {}

	/**
	 * initialize
	 * @param [in] recursive do children recursively
	 * @retval succsess or fail
	 */
	virtual bool init(bool recursive);
	
	/**
	 * set current time
	 * @param [in] time time
	 * @param [in] recursive do children recursively
	 */
	virtual void set_current_time(unsigned long time, bool recursive);
	
	/**
	 * update box
	 * @param [in] recursive do children recursively
	 */
	virtual void update_box(bool recursive);

	/**
	 * draw
	 * @param [in] recursive do children recursively
	 */
	virtual void draw(bool recursive, UMAbc::DrawType type);

protected:
	UMAbcXform(Alembic::AbcGeom::IXform xform)
		: UMAbcObject(xform)
		, xform_(xform)
	{}
	
	virtual UMAbcObjectPtr self_reference()
	{
		return self_reference_.lock();
	}

private:
	Alembic::AbcGeom::IXform xform_;
	UMAbcXformWeakPtr self_reference_;

	UMMat44d static_matrix_;
	bool is_inherit_;
};

} // umabc
