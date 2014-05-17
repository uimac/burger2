/**
 * @file UMSubdivision.h
 * subdivision
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#pragma once

#include <memory>
#include "UMMacro.h"
#include "UMVector.h"
#include "UMMathTypes.h"

namespace umdraw
{
	class UMMesh;
	typedef std::shared_ptr<UMMesh> UMMeshPtr;
} // umdraw

namespace umrt
{
class UMSubdivision;
typedef std::shared_ptr<UMSubdivision> UMSubdivisionPtr;

/**
 * rays
 */
class UMSubdivision 
{
	DISALLOW_COPY_AND_ASSIGN(UMSubdivision);
public:
	UMSubdivision(umdraw::UMMeshPtr mesh);
	~UMSubdivision();

	/**
	 * get subdivided mesh
	 */
	umdraw::UMMeshPtr subdivided_mesh(unsigned int level);

private:
	class SudivImpl;
	typedef std::unique_ptr<SudivImpl> SubdivImplPtr;
	SubdivImplPtr impl_;
};

} // umrt
