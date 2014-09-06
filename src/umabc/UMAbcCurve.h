/**
 * @file UMAbcCurve.h
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
	
class UMAbcCurve;
typedef std::shared_ptr<UMAbcCurve> UMAbcCurvePtr;
typedef std::weak_ptr<UMAbcCurve> UMAbcCurveWeakPtr;

class UMAbcCurve : public UMAbcObject
{
	DISALLOW_COPY_AND_ASSIGN(UMAbcCurve);
public:

	/**
	 * crate instance
	 */
	static UMAbcCurvePtr create(Alembic::AbcGeom::ICurves curves);

	~UMAbcCurve();

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
	 * update curve all
	 */
	void update_curve_all();

protected:
	UMAbcCurve(Alembic::AbcGeom::ICurves curves)
		: UMAbcObject(curves)
		, curves_(curves)
	{}
	
	virtual UMAbcObjectPtr self_reference()
	{
		return self_reference_.lock();
	}

private:
	Alembic::AbcGeom::ICurves curves_;
	Alembic::AbcGeom::P3fArraySamplePtr positions_;
	Alembic::AbcGeom::Int32ArraySamplePtr vertex_count_;
	size_t curve_count_;

	Alembic::AbcGeom::ICurvesSchema::Sample initial_sample_;

	std::vector<const Imath::V3f* > points_;
	
	UMAbcCurveWeakPtr self_reference_;
};

}
