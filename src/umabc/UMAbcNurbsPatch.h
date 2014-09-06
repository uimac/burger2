/**
 * @file UMAbcNurbsPatch.h
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
	
class UMAbcNurbsPatch;
typedef std::shared_ptr<UMAbcNurbsPatch> UMAbcNurbsPatchPtr;
typedef std::weak_ptr<UMAbcNurbsPatch> UMAbcNurbsPatchWeakPtr;

class UMAbcNurbsPatch : public UMAbcObject
{
	DISALLOW_COPY_AND_ASSIGN(UMAbcNurbsPatch);
public:

	/**
	 * crate instance
	 */
	static UMAbcNurbsPatchPtr create(Alembic::AbcGeom::INuPatch patch);

	~UMAbcNurbsPatch();

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
	
	/**
	 * update patch all
	 */
	void update_patch_all();

protected:
	UMAbcNurbsPatch(Alembic::AbcGeom::INuPatch patch)
		: UMAbcObject(patch)
		, patch_(patch)
		, u_size_(0)
		, v_size_(0)
		, u_order_(0)
		, v_order_(0)
	{}
	
	virtual UMAbcObjectPtr self_reference()
	{
		return self_reference_.lock();
	}

private:
	Alembic::AbcGeom::INuPatch patch_;
	Alembic::AbcGeom::INuPatchSchema::Sample initial_sample_;

	Alembic::AbcGeom::P3fArraySamplePtr positions_;
	Alembic::AbcGeom::FloatArraySamplePtr u_knot_;
	Alembic::AbcGeom::FloatArraySamplePtr v_knot_;
	size_t u_size_;
	size_t v_size_;
	int u_order_;
	int v_order_;
	
	std::vector<const Imath::V3f* > points_;
	
	UMAbcNurbsPatchWeakPtr self_reference_;
};

}
