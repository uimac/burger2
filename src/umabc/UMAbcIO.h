/**
 * @file UMAbc.h
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
#include <string>
#include <vector>
#include <functional>
#include "UMMacro.h"
#include "UMAbcSetting.h"
#include "UMAbcCallback.h"
#include "UMMaterial.h"

namespace umdraw
{
	class UMMesh;
	typedef std::shared_ptr<UMMesh> UMMeshPtr;
}

/// uimac alembic library
namespace umabc
{
	
class UMAbcScene;
typedef std::shared_ptr<UMAbcScene> UMAbcScenePtr;

class UMAbcMesh;
typedef std::shared_ptr<UMAbcMesh> UMAbcMeshPtr;

class UMAbcIO;
typedef std::shared_ptr<UMAbcIO> UMAbcIOPtr;

typedef std::vector<umdraw::UMMaterialMap> UMAbcSceneMaterialList;

class UMAbcIO
{
	DISALLOW_COPY_AND_ASSIGN(UMAbcIO);
public:

	UMAbcIO() {}
	~UMAbcIO() {}
	
	/**
	 * release all scenes. call this function before delete.
	 */
	void dispose();

	/**
	 * load 3d file to UMAbcScene
	 */
	UMAbcScenePtr load(std::u16string path, const UMAbcSetting& setting);
	
	/**
	 * save 3d file
	 */
	bool save(std::u16string path, UMAbcScenePtr scene, const UMAbcSetting& setting);

	/**
	 * load settings
	 */
	bool load_setting(std::string path, UMAbcSetting& setting);
	
	/**
	 * save settings
	 */
	bool save_setting(std::string path, const UMAbcSetting& setting);
	
	/** 
	 * import umabc materials
	 * @param [out] dst distination material list
	 * @param [in] abc_mtl_filepath material file path
	 */
	static bool import_material_map(
		umdraw::UMMaterialMap& dst, 
		const std::u16string& abc_mtl_filepath);
	
	/**
	 * convert abc mesh to mesh
	 * @param [in] src source abc mesh
	 * @retval UMMeshPtr converted mesh or none
	 */
	static umdraw::UMMeshPtr convert_abc_mesh_to_mesh(
		umabc::UMAbcMeshPtr src);
};

} // umabc
