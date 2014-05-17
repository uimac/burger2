/**
 * @file UMDirectX11Scene.h
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#pragma once

#include <dxgi.h>
#include <d3d11.h>
#include <string>
#include <memory>
#include "UMMacro.h"
#include "UMScene.h"
#include "UMListener.h"

#include "UMMeshGroup.h"
#include "UMDirectX11MeshGroup.h"
#include "UMDirectX11ShaderManager.h"
#include "UMDirectX11Texture.h"
#include "UMDirectX11Board.h"
#include "UMDirectX11Light.h"
#include "UMDirectX11Camera.h"
#include "UMDirectX11Line.h"
#include "UMDirectX11DrawParameter.h"
#include "UMTime.h"

namespace umdraw
{

class UMDirectX11Scene;
typedef std::shared_ptr<UMDirectX11Scene> UMDirectX11ScenePtr;
typedef std::weak_ptr<UMDirectX11Scene> UMDirectX11SceneWeakPtr;

/**
 * directx11 secne.
 */
class UMDirectX11Scene : public umbase::UMListener
{
	DISALLOW_COPY_AND_ASSIGN(UMDirectX11Scene);
public:

	UMDirectX11Scene();

	~UMDirectX11Scene();
	
	/**
	 * init scene
	 */
	bool init(ID3D11Device *device, ID3D11Texture2D* back_buffer, UMScenePtr scene);
	
	/**
	 * clear scene
	 */
	void clear(ID3D11Device *device_pointer);

	/**
	 * load scene
	 */
	bool load(ID3D11Device* device_pointer, UMScenePtr scene);
	
	/**
	 * update scene
	 */
	bool update(ID3D11Device* device_pointer);

	/**
	 * draw scene
	 */
	bool draw(ID3D11Device* device_pointer);

	/**
	 * get umdraw scene
	 */
	UMScenePtr scene() const;

	/**
	 * update event
	 */
	virtual void update(umbase::UMEventType event_type, umbase::UMAny& parameter);
	
private:
	class Impl;
	typedef std::unique_ptr<Impl> ImplPtr;
	ImplPtr impl_;
};

} // umdraw
