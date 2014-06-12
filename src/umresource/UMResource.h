/**
 * @file UMResource.h
 * resource
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#pragma once

#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <iostream>
#include "UMMacro.h"

/// Uimac resource library
namespace umresource
{

class UMResource
{
	DISALLOW_COPY_AND_ASSIGN(UMResource);
public:
	~UMResource() {}

	static UMResource& instance() {
		static UMResource instance_;
		return instance_;
	}

	/**
	 * find resource data
	 */
	static const std::string& find_resource_data(UMResource& resource, const std::string& name);

	typedef std::vector<umstring> FilePathList;
	typedef std::string Buffer;
	typedef std::vector<Buffer> UnpackedDataList;
	typedef std::vector<umstring> UnpackedNameList;

	/**
	 * get default resource path
	 */
	static umstring default_resource_path();

	/**
	 * pack files to dst file
	 * @param [in] dst_absolute_path distination file path
	 * @param [in] src_absolute_path_list source file path list
	 */
	bool pack(const umstring& dst_absolute_path, const FilePathList& src_absolute_path_list);

	/**
	 * unpack files to dst directory
	 * @param [in] dst_absolute_path distination directory
	 * @param [in] src_absolute_path source file path
	 */
	bool unpack(const umstring& dst_absolute_path, const umstring& src_absolute_path);

	/**
	 * unpack files to memory
	 * @param [in] src_absolute_path source file path
	 */
	bool unpack_to_memory(const umstring& src_absolute_path);

	/**
	 * get unpacked data list
	 */
	UnpackedDataList& unpacked_data_list() { return unpacked_data_list_; }

	/**
	 * get unpacked name list
	 */
	UnpackedNameList& unpacked_name_list() { return unpacked_name_list_; }

private:
	UMResource();
	UnpackedDataList unpacked_data_list_;
	UnpackedNameList unpacked_name_list_;
};

} // umresource
