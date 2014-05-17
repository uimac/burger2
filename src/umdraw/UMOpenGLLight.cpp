/**
 * @file UMOpenGLLight.cpp
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#ifdef WITH_OPENGL

#include "UMOpenGLLight.h"
#include "UMLight.h"


namespace umdraw
{
	
UMOpenGLLight::~UMOpenGLLight()
{
}

/** 
 * initialize
 */
bool UMOpenGLLight::init()
{
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
bool UMOpenGLLight::update()
{
	UMLightPtr light = umlight();
	if (!light) return false;
	
	UMVec3d pos = light->position();
	position_ = UMVec4f((float)pos.x, (float)pos.y, (float)pos.z, 1.0f);
	return true;
}

/**
 * draw
 */
void UMOpenGLLight::draw()
{
}

} // umdraw

#endif // WITH_OPENGL
