/**
 * @file UMAbcPoint.h
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
	
class UMAbcPoint;
typedef std::shared_ptr<UMAbcPoint> UMAbcPointPtr;
typedef std::weak_ptr<UMAbcPoint> UMAbcPointWeakPtr;

class UMOpenGLAbcPoint;
typedef std::shared_ptr<UMOpenGLAbcPoint> UMOpenGLAbcPointPtr;

class UMAbcPoint : public UMAbcObject
{
	DISALLOW_COPY_AND_ASSIGN(UMAbcPoint);
public:

	/**
	 * crate instance
	 */
	static UMAbcPointPtr create(Alembic::AbcGeom::IPoints points);

	~UMAbcPoint();

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
	 * update point all
	 */
	void update_point_all();
	
	/**
	 * get opengl point
	 */
	UMOpenGLAbcPointPtr opengl_point() { return opengl_point_; }

	///**
	// * get directx point
	// */
	//UMDirectX11AbcPointPtr directx_point() { return directx_point_; }

protected:
	UMAbcPoint(Alembic::AbcGeom::IPoints points)
		: UMAbcObject(points)
		, points_(points)
	{}
	
	virtual UMAbcObjectPtr self_reference()
	{
		return self_reference_.lock();
	}

private:
	/**
	 * update points
	 */ 
	void update_point();

	/** 
	 * update normal
	 */
	void update_normal();
	
	/** 
	 * update color
	 */
	void update_color();

	Alembic::AbcGeom::IPoints points_;
	Alembic::AbcGeom::IPointsSchema::Sample sample_;

	Alembic::AbcGeom::IBox3dProperty initial_bounds_prop_;
	Alembic::AbcGeom::IC3fArrayProperty color_prop_;
	Alembic::AbcGeom::IN3fArrayProperty normal_prop_;

	Alembic::AbcGeom::P3fArraySamplePtr positions_;
	Alembic::AbcGeom::C3fArraySamplePtr colors_;
	Alembic::AbcGeom::N3fArraySamplePtr normals_;
	
	UMAbcPointWeakPtr self_reference_;

	UMOpenGLAbcPointPtr opengl_point_;
};

}
