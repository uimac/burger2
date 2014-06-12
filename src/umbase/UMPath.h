/**
 * @file UMPath.h
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#pragma once

#include <string>
#include "UMMacro.h"

namespace umbase
{

/**
 * path utility
 */
class UMPath
{
	DISALLOW_COPY_AND_ASSIGN(UMPath);

public:

	static bool exists(const umstring& absolute_path);

	static umstring module_absolute_path();

	static umstring resource_absolute_path(const umstring& file_name);

	static umstring get_file_name(const umstring& file_name);
	
	static umstring get_absolute_path(const umstring& base_path, umstring& file_name);

	static umstring get_env(const umstring& env);

};

} // umbase
