/**
 * @file UMDirectX11AbcPoint.cpp
 * any
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#ifdef WITH_DIRECTX

#include "UMDirectX11AbcPoint.h"

#include "UMAbcConvert.h"

namespace umabc
{
	using namespace Alembic::Abc;
	using namespace Alembic::AbcGeom;

void UMDirectX11AbcPoint::draw() const
{
	//if (!positions_) return;
	//const size_t position_size = positions_->size();
	//if (position_size <= 0) return;

	//const V3f *points = positions_->get();

	//const V3f *colors = NULL;
	//if (colors_ && colors_->size() == position_size)
	//{
	//	colors = colors_->get();
	//}
	//
	//const V3f *normals = NULL;
	//if (normals_ && normals_->size() == position_size)
	//{
	//	normals = normals_->get();
	//}

	//if (!normals)
	//{
	//	glDisable(GL_LIGHTING);
	//}
	//if (!colors)
	//{
	//	glColor3f(1.0f, 1.0f, 1.0f);
	//}
	//
	//glEnable( GL_POINT_SMOOTH );
	//glPointSize(2.0f);

	//glEnableClientState(GL_VERTEX_ARRAY);
	//if (normals)
	//{
	//	glEnableClientState(GL_NORMAL_ARRAY);
	//	glNormalPointer(GL_FLOAT, 0, ( const GLvoid * )normals);
	//}
	//if (colors)
	//{
	//	glEnableClientState(GL_COLOR_ARRAY);
	//	glColorPointer(3, GL_FLOAT, 0, ( const GLvoid * )colors);
	//}
	//glVertexPointer(3, GL_FLOAT, 0, (const GLvoid *)points);
	//glDrawArrays(GL_POINTS, 0, ( GLsizei )(position_size));
	//if (colors)
	//{
	//	glDisableClientState(GL_COLOR_ARRAY);
	//}
}

} // umabc

#endif // WITH_DIRECTX
