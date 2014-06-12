/**
 * @file UMSoftwareIO.h
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#pragma once

#include "UMMacro.h"

#include "UMIO.h"
#include "UMScene.h"

namespace umdraw
{

/**
 * model in/out
 */
class UMSoftwareIO
{
	DISALLOW_COPY_AND_ASSIGN(UMSoftwareIO);

public:
	
	/** 
	 * import umdraw mesh list
	 * @param [out] dst distination mesh list
	 * @param [in] src source object
	 * @param [in] absolute_file_path file path
	 */
	static bool import_mesh_list(
		UMMeshList& dst, 
		const umio::UMObjectPtr src,
		const umstring& absolute_file_path);

};

} // umdraw
