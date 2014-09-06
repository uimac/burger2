/**
 * @file UMDirectX11AbcPoint.h
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

class UMDirectX11AbcPoint;
typedef std::shared_ptr<UMDirectX11AbcPoint> UMDirectX11AbcPointPtr;
typedef std::weak_ptr<UMDirectX11AbcPoint> UMDirectX11AbcPointWeakPtr;

class UMDirectX11AbcPoint
{
	DISALLOW_COPY_AND_ASSIGN(UMDirectX11AbcPoint);
public:
	
	UMDirectX11AbcPoint(UMAbcPointPtr abc_point) :
		abc_point_(abc_point)
	{}

	~UMDirectX11AbcPoint();

	/**
	 * draw
	 */
	void draw() const;

private:
	UMAbcPointPtr abc_point() const { return abc_point_.lock(); }
	UMAbcPointWeakPtr abc_point_;

};

}
