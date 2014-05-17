/**
 * @file UMOpenGLAbcPoint.h
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

namespace umdraw
{
	class UMOpenGLPoint;
	typedef std::shared_ptr<UMOpenGLPoint> UMOpenGLPointPtr;
} // umdraw

namespace umabc
{
	
class UMAbcPoint;
typedef std::shared_ptr<UMAbcPoint> UMAbcPointPtr;
typedef std::weak_ptr<UMAbcPoint> UMAbcPointWeakPtr;

class UMOpenGLAbcPoint;
typedef std::shared_ptr<UMOpenGLAbcPoint> UMOpenGLAbcPointPtr;
typedef std::weak_ptr<UMOpenGLAbcPoint> UMOpenGLAbcPointWeakPtr;

class UMOpenGLAbcPoint
{
	DISALLOW_COPY_AND_ASSIGN(UMOpenGLAbcPoint);
public:
	UMOpenGLAbcPoint(UMAbcPointPtr abc_point);

	~UMOpenGLAbcPoint() {}
	
	/**
	 * draw
	 */
	void draw() const;
	
	/**
	 * update vertex
	 */
	void update_vertex(const Alembic::AbcGeom::P3fArraySamplePtr& vertex);

	/**
	 * get opengl point
	 */
	umdraw::UMOpenGLPointPtr opengl_point() { return gl_point_; }

private:
	UMAbcPointPtr abc_point() const { return abc_point_.lock(); }
	UMAbcPointWeakPtr abc_point_;

	umdraw::UMOpenGLPointPtr gl_point_;

	unsigned int vertex_size_;
};

}
