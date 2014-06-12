/**
 * @file UMGUI.cpp
 * any
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#ifdef WITH_DIRECTX

#include "UMDirectX11GUIScene.h"

#include <memory>

#include "UMGUIScene.h"
#include "UMDirectX11.h"
#include "UMDirectX11Mesh.h"
#include "UMDirectX11Material.h"
#include "UMDirectX11GUIBoard.h"
#include "UMDirectX11Light.h"
#include "UMDirectX11ShaderManager.h"
#include "UMDirectX11IO.h"
#include "UMCamera.h"
#include "UMGUIBoard.h"

namespace umgui
{
	using namespace umdraw;

class UMDirectX11GUIScene::SceneImpl
{
	DISALLOW_COPY_AND_ASSIGN(SceneImpl);
public:
	SceneImpl() {}
	~SceneImpl() {}

	void init_object(ID3D11Device* device_pointer, UMGUIObjectPtr obj)
	{
		if (UMGUIBoardPtr board = std::dynamic_pointer_cast<UMGUIBoard>(obj))
		{
			if (UMDirectX11GUIBoardPtr dx_board = std::make_shared<UMDirectX11GUIBoard>(board))
			{
				dx_board->init(device_pointer);
				dx_board_list_.push_back(dx_board);
			}
		}
		UMGUIObjectList::iterator it = obj->mutable_children().begin();
		for (; it != obj->mutable_children().end(); ++it)
		{
			init_object(device_pointer, *it);
		}
	}

	bool init(ID3D11Device* device_pointer, UMGUIScenePtr gui_scene)
	{
		gui_scene_ = gui_scene;
		// init shader manager
		{
			dx_constant_shader_manager_ = std::make_shared<UMDirectX11ShaderManager>();
			dx_constant_shader_manager_->init(device_pointer, UMDirectX11ShaderManager::eConstants);

			dx_shader_manager_ = std::make_shared<UMDirectX11ShaderManager>();
			dx_shader_manager_->init(device_pointer, UMDirectX11ShaderManager::eModel);
		}

		init_object(device_pointer, gui_scene->root_object());

		if (umdraw::UMScenePtr scene = gui_scene->umdraw_scene())
		{
			UMDirectX11LightPtr dx_light = umdraw::UMDirectX11IO::convert_light_to_dx11_light(device_pointer, gui_scene->light());

			// camera
			UMDirectX11CameraPtr dx_camera = umdraw::UMDirectX11IO::convert_camera_to_dx11_camera(device_pointer, gui_scene->camera());
			dx_draw_parameter_ = std::make_shared<umdraw::UMDirectX11DrawParameter>();
			dx_draw_parameter_->set_camera(dx_camera);
			dx_draw_parameter_->set_light(dx_light);
			dx_draw_parameter_->set_shader_manager(dx_shader_manager_);
		}
		
		return true;
	}

	bool update(ID3D11Device* device_pointer)
	{
		// update camera
		if (UMDirectX11CameraPtr camera = dx_draw_parameter_->camera())
		{
			camera->update(device_pointer);
		}
		return true;
	}

	bool clear(ID3D11Device* device_pointer)
	{
		return true;
	}

	bool draw(ID3D11Device* device_pointer)
	{
		ID3D11DeviceContext *device_context_pointer = NULL;
		device_pointer->GetImmediateContext(&device_context_pointer);

		UMDirectX11ShaderManagerPtr shader_manager = dx_constant_shader_manager_;
		if (shader_manager)
		{
			// apply constant buffer
			const UMDirectX11ShaderManager::BufferPointerList& constant_buffer = shader_manager->constant_buffer_list();
			if (!constant_buffer.empty())
			{
				// put camera to constant buffer
				if (UMDirectX11CameraPtr camera = dx_draw_parameter_->camera())
				{
					ID3D11Buffer* constant = constant_buffer[0];
					device_context_pointer->UpdateSubresource(constant, 0, NULL, &(*camera), 0, 0);
					device_context_pointer->VSSetConstantBuffers( 0, 1, &constant );
				}

				// put light to constant buffer
				if (UMDirectX11LightPtr light = dx_draw_parameter_->light())
				{
					ID3D11Buffer* constant = constant_buffer[1];
					device_context_pointer->UpdateSubresource(constant, 0, NULL, &(*light), 0, 0);
					device_context_pointer->VSSetConstantBuffers( 1, 1, &constant );
					device_context_pointer->PSSetConstantBuffers( 1, 1, &constant );
				}
			}
		}

		for (UMDirectX11GUIBoardList::iterator it = dx_board_list_.begin(); it != dx_board_list_.end(); ++it)
		{
			(*it)->draw(device_pointer, dx_draw_parameter_);
		}

		SAFE_RELEASE(device_context_pointer);

		return true;
	}

	void resize(ID3D11Device* device_pointer, int width, int height)
	{
	}

private:
	UMGUIScenePtr gui_scene_;
	umdraw::UMDirectX11DrawParameterPtr dx_draw_parameter_;
	UMDirectX11ShaderManagerPtr dx_shader_manager_;
	UMDirectX11GUIBoardList dx_board_list_;
	UMDirectX11ShaderManagerPtr dx_constant_shader_manager_;
};

/**
 * constructor
 */
UMDirectX11GUIScene::UMDirectX11GUIScene()
	:  impl_(new UMDirectX11GUIScene::SceneImpl())
{
}

/**
 * destructor
 */
UMDirectX11GUIScene::~UMDirectX11GUIScene()
{
}

/**
 * initialize
 */
bool UMDirectX11GUIScene::init(UMGUIScenePtr scene)
{
	if (ID3D11Device* device_pointer = reinterpret_cast<ID3D11Device*>(umdraw::UMDirectX11::current_device_pointer()))
	{
		return impl_->init(device_pointer, scene);
	}
	return false;
}

/**
 * update
 */
bool UMDirectX11GUIScene::update()
{
	if (ID3D11Device* device_pointer = reinterpret_cast<ID3D11Device*>(umdraw::UMDirectX11::current_device_pointer()))
	{
		return impl_->update(device_pointer);
	}
	return false;
}

/**
 * create view/depth
 */
bool UMDirectX11GUIScene::clear()
{
	if (ID3D11Device* device_pointer = reinterpret_cast<ID3D11Device*>(umdraw::UMDirectX11::current_device_pointer()))
	{
		return impl_->clear(device_pointer);
	}
	return false;
}

/**
 * draw
 */
bool UMDirectX11GUIScene::draw()
{
	if (ID3D11Device* device_pointer = reinterpret_cast<ID3D11Device*>(umdraw::UMDirectX11::current_device_pointer()))
	{
		return impl_->draw(device_pointer);
	}
	return false;
}

/**
 * resize
 */
void UMDirectX11GUIScene::resize(int width, int height)
{
	if (ID3D11Device* device_pointer = reinterpret_cast<ID3D11Device*>(umdraw::UMDirectX11::current_device_pointer()))
	{
		return impl_->resize(device_pointer, width, height);
	}
}

} // umgui

#endif // WITH_DIRECTX
