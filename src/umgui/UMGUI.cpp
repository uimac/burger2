/**
 * @file UMGUI.cpp
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#include "UMGUI.h"

#include <string>
#include <assert.h>
#include "UMStringUtil.h"
#include "UMPath.h"
#include "UMGUIScene.h"
#include "UMDirectX11GUIScene.h"
#include "UMOpenGLGUIScene.h"

namespace umgui
{
	using namespace umbase;

/**
 * create instance
 */
UMGUIPtr UMGUI::create(DrawType type)
{
#ifdef WITH_DIRECTX
	if (type == eDirectX)
	{
		UMGUIPtr drawer = std::make_shared<UMDirectX11GUIScene>();
		drawer->draw_type_ = type;
		return drawer;
	}
#endif // WITH_DIRECTX

#ifdef WITH_OPENGL
	if (type == eOpenGL)
	{
		UMGUIPtr drawer = std::make_shared<UMOpenGLGUIScene>();
		drawer->draw_type_ = type;
		return drawer;
	}
#endif // WITH_OPENGL
	return UMGUIPtr();
}

/**
 * destructor
 */
UMGUI::~UMGUI()
{
}

} // umgui
