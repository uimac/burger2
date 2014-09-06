/**
 * @file UMDirectX11.cpp
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#include "UMDirectX11.h"
#ifdef WITH_DIRECTX

#include <dxgi.h>
#include <d3d11.h>

//#include <shellapi.h>
//#include <delayimp.h>

#include "UMDirectX11Scene.h"
#include "UMPath.h"
#include "UMStringUtil.h"
#include "UMAny.h"
#include "UMListenerConnector.h"

#include "UMScene.h"

namespace umdraw
{

UMDirectXDevice UMDirectX11::dx_device_(NULL);

class UMDirectX11Impl : public umbase::UMListenerConnector
{
public:
	UMDirectX11Impl()
		:
	// IDXGI
	dxgi_factory_pointer_(NULL),
	dxgi_adapter_pointer_(NULL),
	dxgi_device_pointer_(NULL),
	dxgi_swap_chain_pointer_(NULL),
	// ID3D11
	device_pointer_(NULL),
	device_context_pointer_(NULL),
	depth_stencil_state_pointer_(NULL),
	rasterizaer_state_pointer_(NULL),
	blend_state_(NULL),
	d3d11_debug_pointer_(NULL),
	dx_scene_(std::make_shared<UMDirectX11Scene>())
	{}
	
	~UMDirectX11Impl()
	{
		// goto window mode
		if (dxgi_swap_chain_pointer_)
		{
			BOOL fullscreen;
			dxgi_swap_chain_pointer_->GetFullscreenState(&fullscreen, NULL);
			if (fullscreen)
			{
				dxgi_swap_chain_pointer_->SetFullscreenState(FALSE, NULL);
			}
		}

		// IDXGI
		SAFE_RELEASE(dxgi_swap_chain_pointer_);
		SAFE_RELEASE(dxgi_factory_pointer_);
		SAFE_RELEASE(dxgi_adapter_pointer_);
		SAFE_RELEASE(dxgi_device_pointer_);
		// ID3D11
		SAFE_RELEASE(device_pointer_);
		SAFE_RELEASE(device_context_pointer_);
		SAFE_RELEASE(depth_stencil_state_pointer_);
		SAFE_RELEASE(rasterizaer_state_pointer_);
		SAFE_RELEASE(blend_state_);
	
		//if (d3d11_debug_pointer_)
		//{
		//	d3d11_debug_pointer_->ReportLiveDeviceObjects(D3D11_RLDO_SUMMARY | D3D11_RLDO_DETAIL);
		//}

		SAFE_RELEASE(d3d11_debug_pointer_);
	}

	/**
	 * initialize
	 */
	virtual bool init(HWND hWnd,  umdraw::UMScenePtr scene);
	
	/**
	 * update
	 */
	bool update();

	/**
	 * draw frame
	 */
	bool draw();

	/**
	 * clear view/depth
	 */
	bool clear();

	/**
	 * resize
	 */
	void resize(int width, int height)
	{
	}

	/**
	 * get umdraw scene
	 */
	UMScenePtr scene() const {
		return dx_scene_->scene();
	}

	ID3D11Device* device_pointer() { return device_pointer_; }
private:

	HWND handle_;

	//UMDirectX11WeakPtr self_reference_;

	/**
	 * initialize devices
	 */
	bool init_devices(HWND hWnd,  umdraw::UMScenePtr scene);

	// DXGI
	IDXGIFactory *dxgi_factory_pointer_;
	IDXGIAdapter *dxgi_adapter_pointer_;
	IDXGIDevice1 *dxgi_device_pointer_;
	
	// ID3D11
	ID3D11Device* device_pointer_;
	ID3D11DeviceContext *device_context_pointer_;
	ID3D11DepthStencilState *depth_stencil_state_pointer_;
	ID3D11RasterizerState *rasterizaer_state_pointer_;
	ID3D11BlendState* blend_state_;

	ID3D11Debug* d3d11_debug_pointer_;
	
	IDXGISwapChain *dxgi_swap_chain_pointer_;
	DXGI_SWAP_CHAIN_DESC dxgi_swap_chain_desc_;

	UMDirectX11ScenePtr dx_scene_;
};

/**
 * initialize
 */
bool UMDirectX11Impl::init(HWND hWnd, umdraw::UMScenePtr scene)
{
	if (!init_devices(hWnd, scene))
	{
		return false;
	}

	if (device_pointer_ && device_context_pointer_)
	{
		// rasterizer state
		{
			D3D11_RASTERIZER_DESC desc;
			ZeroMemory( &desc, sizeof( D3D11_RASTERIZER_DESC ) );
			desc.CullMode = D3D11_CULL_BACK; // backface culling
			//desc.CullMode = D3D11_CULL_NONE; 
			desc.FrontCounterClockwise =TRUE; // CCW
			//desc.FillMode = D3D11_FILL_WIREFRAME;
			desc.FillMode = D3D11_FILL_SOLID; 
			desc.DepthClipEnable = TRUE;
			device_pointer_->CreateRasterizerState( &desc, &rasterizaer_state_pointer_ );
			device_context_pointer_->RSSetState(rasterizaer_state_pointer_);
		}

		// depth stencil state
		{
			D3D11_DEPTH_STENCIL_DESC desc;
			ZeroMemory( &desc, sizeof( D3D11_DEPTH_STENCIL_DESC ) );
			desc.DepthEnable	= TRUE;
			desc.DepthWriteMask	= D3D11_DEPTH_WRITE_MASK_ALL;
			desc.DepthFunc	= D3D11_COMPARISON_LESS;
			desc.StencilEnable	= FALSE;
			device_pointer_->CreateDepthStencilState( &desc, &depth_stencil_state_pointer_ );
		}

		// init member
		handle_ = hWnd;
		
		mutable_event_list().push_back(scene->camera_change_event());
		connect(dx_scene_);

		return true;
	}
	return false;
}

/**
 * initialize devices Impl
 * @param [in] hWnd main window handle
 */
bool UMDirectX11Impl::init_devices(HWND hWnd,  umdraw::UMScenePtr scene)
{
	D3D_FEATURE_LEVEL FeatureLevels[] = {
		D3D_FEATURE_LEVEL_11_0,		// DirectX11.0 GPU
		D3D_FEATURE_LEVEL_10_1,		// DirectX10.1 GPU
		D3D_FEATURE_LEVEL_10_0,		// DirectX10.0 GPU
		D3D_FEATURE_LEVEL_9_3,		// DirectX9/ShaderModel3 GPU
		D3D_FEATURE_LEVEL_9_2,		// DirectX9/ShaderModel2 GPU
		D3D_FEATURE_LEVEL_9_1,		// DirectX9/ShaderModel2 GPU
	};

	// create device
	{
		D3D_FEATURE_LEVEL feature_level;
		HRESULT hr = D3D11CreateDevice(
			// adapter
			NULL,
			D3D_DRIVER_TYPE_HARDWARE,
			// module
			NULL,
			D3D11_CREATE_DEVICE_DEBUG,
			FeatureLevels,
			sizeof(FeatureLevels) / sizeof(D3D_FEATURE_LEVEL),
			D3D11_SDK_VERSION,
			&device_pointer_,
			&feature_level,
			&device_context_pointer_ );

		if FAILED( hr )
		{
			return false;
		}
	}

	// get dxgi adapter, factory
	{
		// get IDXGIDevice from ID3D11Device
		if FAILED(device_pointer_->QueryInterface( __uuidof(IDXGIDevice1), (void**)&dxgi_device_pointer_))
		{
			return false;
		}

		//if FAILED(device_pointer_->QueryInterface( __uuidof(ID3D11Debug), (void**)&d3d11_debug_pointer_))
		//{
		//	return false;
		//}
	
		// get adapter from IDXGIDevice
		if FAILED(dxgi_device_pointer_->GetAdapter( &dxgi_adapter_pointer_))
		{
			return false;
		}
	
		// get factory from IDXGIDevice
		if FAILED(dxgi_adapter_pointer_->GetParent( __uuidof(IDXGIFactory), (void**)&dxgi_factory_pointer_))
		{
			return false;
		}
	}
	
	// set viewport
	{
		D3D11_VIEWPORT vp;
		vp.Width = (FLOAT)scene->width();
		vp.Height = (FLOAT)scene->height();
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		device_context_pointer_->RSSetViewports( 1, &vp );
	}
	
	// blending state
	{
		D3D11_BLEND_DESC BlendStateDesc;
		BlendStateDesc.AlphaToCoverageEnable = FALSE;
		BlendStateDesc.IndependentBlendEnable = FALSE;
		for(int i=0; i < 8; i++)
		{
			BlendStateDesc.RenderTarget[i].BlendEnable = TRUE;
			BlendStateDesc.RenderTarget[i].SrcBlend = D3D11_BLEND_SRC_ALPHA;
			BlendStateDesc.RenderTarget[i].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
			BlendStateDesc.RenderTarget[i].BlendOp = D3D11_BLEND_OP_ADD;
			BlendStateDesc.RenderTarget[i].SrcBlendAlpha = D3D11_BLEND_ONE;
			BlendStateDesc.RenderTarget[i].DestBlendAlpha = D3D11_BLEND_ZERO;
			BlendStateDesc.RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP_ADD;
			BlendStateDesc.RenderTarget[i].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		}
		device_pointer_->CreateBlendState(&BlendStateDesc, &blend_state_);

		// set blendding state
		float blend_factor[4] = {D3D11_BLEND_ZERO, D3D11_BLEND_ZERO, D3D11_BLEND_ZERO, D3D11_BLEND_ZERO};
		device_context_pointer_->OMSetBlendState(blend_state_, blend_factor, 0xffffffff);
	}
	
	// swap chan parameters
	{
		dxgi_swap_chain_desc_.BufferDesc.Width = scene->width();
		dxgi_swap_chain_desc_.BufferDesc.Height = scene->height();
		dxgi_swap_chain_desc_.BufferDesc.Format	= DXGI_FORMAT_R8G8B8A8_UNORM;
		dxgi_swap_chain_desc_.BufferDesc.RefreshRate.Denominator = 1;
		dxgi_swap_chain_desc_.BufferDesc.RefreshRate.Numerator= 60;
		dxgi_swap_chain_desc_.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		dxgi_swap_chain_desc_.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		// multi sampling count
		dxgi_swap_chain_desc_.SampleDesc.Count = 1;
		// multi sampling quality
		dxgi_swap_chain_desc_.SampleDesc.Quality = 0;
		// backbuffer
		dxgi_swap_chain_desc_.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		dxgi_swap_chain_desc_.BufferCount = 3;
		// output window
		dxgi_swap_chain_desc_.OutputWindow = hWnd;
		// window mode ?
		dxgi_swap_chain_desc_.Windowed = 1;
		// swap
		dxgi_swap_chain_desc_.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		dxgi_swap_chain_desc_.Flags	= DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		// create swap chan from D3D11Device
		if FAILED(dxgi_factory_pointer_->CreateSwapChain(
				device_pointer_, 
				&dxgi_swap_chain_desc_, 
				&dxgi_swap_chain_pointer_))
		{
			return false;
		}
	}

	// get back buffer
	ID3D11Texture2D *back_buffer = NULL;
	if FAILED(dxgi_swap_chain_pointer_->GetBuffer(
			0,
			__uuidof(ID3D11Texture2D),
			(LPVOID*)&back_buffer))
	{
		return false;
	}
	
	// create scene
	if (!dx_scene_->init(device_pointer_, back_buffer, scene)) return false;
	if (!dx_scene_->load(device_pointer_, scene)) return false;
	
	if (back_buffer)
	{
		back_buffer->Release();
		back_buffer = NULL;
	}
	return true;
}

/**
 * update impl
 */
bool UMDirectX11Impl::update()
{
	// update scene
	if (!dx_scene_->update(device_pointer_))
	{
		return false;
	}

	return true;
}

/**
 * draw impl
 */
bool UMDirectX11Impl::draw()
{
	// draw scene
	if (!dx_scene_->draw(device_pointer_))
	{
		return false;
	}

	return true;
}

bool UMDirectX11Impl::clear()
{
	if (dxgi_swap_chain_pointer_)
	{
		dxgi_swap_chain_pointer_->Present(1, 0);
	}
	dx_scene_->clear(device_pointer_);
	return true;
}

/**
 * create instance
 */
UMDirectX11Ptr UMDirectX11::create()
{
	UMDirectX11Ptr viewer = UMDirectX11Ptr(new UMDirectX11);
	return viewer;
}

/**
 * constructor
 */
UMDirectX11::UMDirectX11()
	:  impl_(std::make_shared<UMDirectX11Impl>())
{
}

/**
 * destructor
 */
UMDirectX11::~UMDirectX11()
{
	UMDirectX11::dx_device_ = NULL;
}

/**
 * update
 */
bool UMDirectX11::update()
{
	UMDirectX11::dx_device_ = reinterpret_cast<UMDirectXDevice>(impl_->device_pointer());
	return impl_->update();
}

/*
 * draw
 */
bool UMDirectX11::draw()
{
	UMDirectX11::dx_device_ = reinterpret_cast<UMDirectXDevice>(impl_->device_pointer());
	return impl_->draw();
}


/**
 * clear view/depth
 */
bool UMDirectX11::clear()
{
	return impl_->clear();
}

/**
 * resize
 */
void UMDirectX11::resize(int width, int height)
{
	impl_->resize(width, height);
}

bool UMDirectX11::init(void* hWnd, UMScenePtr scene)
{
	if (impl_->init( *(reinterpret_cast<HWND*>(&hWnd)), scene))
	{
		UMDirectX11::dx_device_ = reinterpret_cast<UMDirectXDevice>(impl_->device_pointer());
		return true;
	}
	return false;
}

/**
 * get umdraw scene
 */
UMScenePtr UMDirectX11::scene() const
{
	return impl_->scene();
}


} // umdraw

#else

namespace umdraw
{
	UMDirectXDevice UMDirectX11::dx_device_(NULL);
}

#endif // WITH_DIRECTX
