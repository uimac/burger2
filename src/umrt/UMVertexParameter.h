/**
 * @file UMVertexParameter.h
 * vertex parameters
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#pragma once

#include "UMMacro.h"
#include "UMVector.h"
#include "UMMesh.h"
#include "UMMathTypes.h"
#include "UMBox.h"

namespace umabc
{
	class UMAbcMesh;
	typedef std::shared_ptr<UMAbcMesh> UMAbcMeshPtr;
	typedef std::weak_ptr<UMAbcMesh> UMAbcMeshWeakPtr;
} // umabc

namespace umrt
{

class UMVertexParameter;
typedef std::shared_ptr<UMVertexParameter> UMVertexParameterPtr;
typedef std::vector<UMVertexParameterPtr> UMVertexParameterList;

/**
 * vertex parameters
 */
class UMVertexParameter
{
	DISALLOW_COPY_AND_ASSIGN(UMVertexParameter);
public:
	typedef std::vector<int> IndexList;
	
	UMVertexParameter() 
		: triangle_index_list_(0)
	{}

	UMVertexParameter(umdraw::UMMeshPtr mesh) 
		: mesh_(mesh),
		triangle_index_list_(0)
	{}
	
	UMVertexParameter(umabc::UMAbcMeshPtr mesh) 
		: abc_mesh_(mesh),
		triangle_index_list_(0)
	{}

	~UMVertexParameter() {}

	/**
	 * get triangle index list
	 */
	const IndexList& triangle_index_list() const { return triangle_index_list_; }
	
	/**
	 * get triangle index list
	 */
	IndexList& mutable_triangle_index_list() { return triangle_index_list_; }

private:
	umdraw::UMMeshPtr mesh() { return mesh_.lock(); }
	umdraw::UMMeshPtr mesh() const { return mesh_.lock(); }
	umdraw::UMMeshWeakPtr mesh_;
	
	umabc::UMAbcMeshPtr abc_mesh() { return abc_mesh_.lock(); }
	umabc::UMAbcMeshPtr abc_mesh() const { return abc_mesh_.lock(); }
	umabc::UMAbcMeshWeakPtr abc_mesh_;

	IndexList triangle_index_list_;
};

} // umrt
