/**
 * @file UMDirectX11Light.cpp
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#ifdef WITH_DIRECTX

#include "UMDirectX11Light.h"

namespace umdraw
{

UMDirectX11Light::~UMDirectX11Light()
{
}

/** 
 * initialize
 */
bool UMDirectX11Light::init(ID3D11Device *device_pointer)
{
	if (!device_pointer) return false;
	UMLightPtr light = umlight();
	if (!light) return false;

	UMVec3d pos = light->position();
	position_ = UMVec4f((float)pos.x, (float)pos.y, (float)pos.z, 1.0f);
	//UMVec3d dir = (-light->position()).normalized();
	//direction_ = UMVec4f((float)dir.x, (float)dir.y, (float)dir.z, 1.0);
	color_ = UMVec4f(1.0f, 1.0f, 1.0f, 1.0f);
	ambient_color_ = UMVec4f(0.1f, 0.1f, 0.1f, 1.0f);

	return true;
}

/** 
 * update
 */
bool UMDirectX11Light::update(ID3D11Device *device_pointer)
{
	if (!device_pointer) return false;
	
	UMLightPtr light = umlight();
	if (!light) return false;
	
	UMVec3d pos = light->position();
	position_ = UMVec4f((float)pos.x, (float)pos.y, (float)pos.z, 1.0f);
	return true;
}

/**
 * draw
 */
void UMDirectX11Light::draw(ID3D11Device* device_pointer)
{
}

} // umdraw

#endif // WITH_DIRECTX
