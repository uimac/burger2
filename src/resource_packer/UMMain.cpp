/**
 * @file UMResource.cpp
 * resource creator
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <snappy.h>
#include <exception>
#include <assert.h>
#include "UMResource.h"
#include "UMPath.h"
#include "UMStringUtil.h"
#include "UMTime.h"

using namespace umbase;

static std::u16string resource_path(const std::string& file_name)
{
	return UMPath::resource_absolute_path(UMStringUtil::utf8_to_utf16(file_name));
}

// main
int main()
{
	typedef std::vector<std::u16string> FileList;
	FileList files;
	
	files.push_back(resource_path("common.hlsl"));
	files.push_back(resource_path("model_vs.hlsl"));
	files.push_back(resource_path("model_ps.hlsl"));
	files.push_back(resource_path("UMModelShader.vs"));
	files.push_back(resource_path("UMModelShader.fs"));
	files.push_back(resource_path("UMPointShader.vs"));
	files.push_back(resource_path("UMPointShader.fs"));
	files.push_back(resource_path("UMBoardShader.vs"));
	files.push_back(resource_path("UMBoardShader.fs"));
	files.push_back(resource_path("UMColorCircle.vs"));
	files.push_back(resource_path("UMColorCircle.fs"));
	files.push_back(resource_path("UMModelGeometryPass.vs"));
	files.push_back(resource_path("UMModelGeometryPass.fs"));
	files.push_back(resource_path("UMBoardLightPass.vs"));
	files.push_back(resource_path("UMBoardLightPass.fs"));
	files.push_back(resource_path("icon/pen.png"));
	files.push_back(resource_path("cabbage.png"));
	files.push_back(resource_path("icon/hoge.png"));
	files.push_back(resource_path("icon/piyo.png"));
	files.push_back(resource_path("icon/round_rect.png"));

	//files.push_back(resource_path("out2/alembic_file.abc"));
	//files.push_back(resource_path("particle1.abc"));
	//files.push_back(resource_path("miku_alegro.bos"));
	//files.push_back(resource_path("cornellbox_nolight.bos"));
	//files.push_back(resource_path("bunny.bos"));
	
	std::u16string out_file = resource_path("cabbage_resource.pack");

	umresource::UMResource::instance().pack(out_file, files);

	// unpack test
	umresource::UMResource::instance().unpack_to_memory(out_file);
	
	return 0;
}
