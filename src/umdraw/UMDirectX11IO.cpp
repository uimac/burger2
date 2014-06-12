/**
 * @file UMDirectX11IO.cpp
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#ifdef WITH_DIRECTX

#include "UMDirectX11IO.h"

#include <string>
#include <assert.h>

#include "UMIO.h"
#include "UMStringUtil.h"
#include "UMPath.h"
#include "UMSoftwareIO.h"
#include "UMLine.h"
#include "UMDirectX11Line.h"

namespace
{
	using namespace umdraw;

	UMVec2f to_dx(const umio::UMVec2d& v) { return UMVec2f((float)v.x, (float)v.y); }

	UMVec3f to_dx(const umio::UMVec3d& v) { return UMVec3f((float)v.x, (float)v.y, (float)v.z); }

	UMVec4f to_dx(const umio::UMVec4d& v) { return UMVec4f((float)v.x, (float)v.y, (float)v.z, (float)v.w); } 

	//----------------------------------------------------------------------------

	/** 
	 * load index from umdraw to dx11
	 */
	void load_vertex_index(
		ID3D11DeviceContext* device_context,
		UMDirectX11MeshPtr dst_mesh, 
		UMMeshPtr src_mesh)
	{
		if (!device_context) return;
		D3D11_MAPPED_SUBRESOURCE resource;
		device_context->Map(dst_mesh->index_buffer_pointer(), 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);

		UMVec3ui* index = (UMVec3ui*) resource.pData;
		const UMMesh::Vec3iList& face_list = src_mesh->face_list();
		for (size_t i = 0, i_size = face_list.size(); i < i_size; ++i) {
			const UMVec3i& in = face_list.at(i);
			index[i].x = in.x;
			index[i].y = in.y;
			index[i].z = in.z;
		}
		device_context->Unmap(dst_mesh->index_buffer_pointer(), 0);
	}
	
	/**
	 * load vertex from umdraw to dx11
	 */
	void load_vertex(
		ID3D11DeviceContext* device_context,
		UMDirectX11MeshPtr dst_mesh, 
		UMMeshPtr src_mesh)
	{
		if (!device_context) return;
		D3D11_MAPPED_SUBRESOURCE resource;
		device_context->Map(dst_mesh->vertex_buffer_pointer(), 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);

		UMVec3f* dx_vertex = (UMVec3f*)resource.pData;
		const UMMesh::Vec3iList& face_list = src_mesh->face_list();
		const UMMesh::Vec3dList& vertex_list = src_mesh->vertex_list();
		const size_t face_size = face_list.size();
		if (face_size > 0)
		{
			for (size_t i = 0; i < face_size; ++i) {
				const UMVec3i& face = face_list.at(i);
				for (int k = 0; k < 3; ++k)
				{
					const UMVec3d& vertex = vertex_list.at(face[k]);
					dx_vertex[i * 3 + k].x = static_cast<float>(vertex.x);
					dx_vertex[i * 3 + k].y = static_cast<float>(vertex.y);
					dx_vertex[i * 3 + k].z = static_cast<float>(vertex.z);
				}
			}
		}
		else
		{
			const size_t vertex_size = vertex_list.size();
			for (size_t i = 0; i < vertex_size; ++i) {
				const UMVec3d& vertex = vertex_list.at(i);
				dx_vertex[i].x = static_cast<float>(vertex.x);
				dx_vertex[i].y = static_cast<float>(vertex.y);
				dx_vertex[i].z = static_cast<float>(vertex.z);
			}
		}
		device_context->Unmap(dst_mesh->vertex_buffer_pointer(), 0);
	}

	/**
	 * load normal from umdraw to dx11
	 */
	void load_normal(
		ID3D11DeviceContext* device_context,
		UMDirectX11MeshPtr dst_mesh, 
		UMMeshPtr src_mesh)
	{
		if (!device_context) return;
		D3D11_MAPPED_SUBRESOURCE resource;
		device_context->Map(dst_mesh->normal_buffer_pointer(), 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);

		UMVec3f* dx_normal = (UMVec3f*)resource.pData;
		const UMMesh::Vec3iList& face_list = src_mesh->face_list();
		const UMMesh::Vec3dList& normal_list = src_mesh->normal_list();
		const bool is_vertex_sized_normal = src_mesh->vertex_list().size() == src_mesh->normal_list().size();
		if (is_vertex_sized_normal)
		{
			const size_t face_size = face_list.size();
			if (face_size > 0)
			{
				for (size_t i = 0; i < face_size; ++i) {
					const UMVec3i& face = face_list.at(i);
					for (int k = 0; k < 3; ++k)
					{
						const UMVec3d& normal = normal_list.at(face[k]);
						dx_normal[i * 3 + k].x = static_cast<float>(normal.x);
						dx_normal[i * 3 + k].y = static_cast<float>(normal.y);
						dx_normal[i * 3 + k].z = static_cast<float>(normal.z);
					}
				}
			}
			else
			{
				const size_t normal_size = normal_list.size();
				for (size_t i = 0; i < normal_size; ++i) {
					const UMVec3d& normal = normal_list.at(i);
					dx_normal[i].x = static_cast<float>(normal.x);
					dx_normal[i].y = static_cast<float>(normal.y);
					dx_normal[i].z = static_cast<float>(normal.z);
				}
			}
		}
		else if (normal_list.size() == src_mesh->face_list().size() * 3)
		{
			for (size_t i = 0, i_size = face_list.size(); i < i_size; ++i) {
				for (int k = 0; k < 3; ++k)
				{
					const UMVec3d& normal = normal_list.at(i * 3 + k);
					dx_normal[i * 3 + k].x = static_cast<float>(normal.x);
					dx_normal[i * 3 + k].y = static_cast<float>(normal.y);
					dx_normal[i * 3 + k].z = static_cast<float>(normal.z);
				}
			}
		}

		device_context->Unmap(dst_mesh->normal_buffer_pointer(), 0);
	}
	
	/** 
	 * load uv from umdraw to dx11
	 */
	void load_uv(
		ID3D11DeviceContext* device_context,
		UMDirectX11MeshPtr dst_mesh, 
		UMMeshPtr src_mesh)
	{
		if (!device_context) return;
		D3D11_MAPPED_SUBRESOURCE resource;
		device_context->Map(dst_mesh->uv_buffer_pointer(), 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);

		UMVec2f* dx_uv = (UMVec2f*) resource.pData;
		const UMMesh::Vec2dList& uv_list = src_mesh->uv_list();
		
		for (size_t i = 0, i_size = uv_list.size(); i < i_size; ++i) {
			const UMVec2d& uv = uv_list.at(i);
			dx_uv[i].x = static_cast<float>(uv.x);
			dx_uv[i].y = static_cast<float>( 1.0 -uv.y);
		}
		device_context->Unmap(dst_mesh->uv_buffer_pointer(), 0);
	}

	/**
	 * load line from umdraw to dx11
	 */
	void load_line(
		ID3D11DeviceContext* device_context,
		UMDirectX11LinePtr dst_line,
		UMLinePtr src_line)
	{
		if (!device_context) return;
		D3D11_MAPPED_SUBRESOURCE resource;
		device_context->Map(dst_line->vertex_buffer_pointer(), 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
		
		UMVec3f* dx_vertex = (UMVec3f*)resource.pData;

		const UMLine::LineList& line_list = src_line->line_list();
		const size_t line_size = line_list.size() * 2;
		
		std::vector<UMVec3f> lines;
		lines.resize(line_size);
		UMLine::LineList::const_iterator it = line_list.begin();
		for (size_t i = 0, size = line_list.size(); i < size; ++i)
		{
			const UMLine::Line& umline = line_list.at(i);
			dx_vertex[i * 2 + 0].x = static_cast<float>(umline.p0.x);
			dx_vertex[i * 2 + 0].y = static_cast<float>(umline.p0.y);
			dx_vertex[i * 2 + 0].z = static_cast<float>(umline.p0.z);
			dx_vertex[i * 2 + 1].x = static_cast<float>(umline.p1.x);
			dx_vertex[i * 2 + 1].y = static_cast<float>(umline.p1.y);
			dx_vertex[i * 2 + 1].z = static_cast<float>(umline.p1.z);
		}
		device_context->Unmap(dst_line->vertex_buffer_pointer(), 0);

		dst_line->set_vertex_count(static_cast<unsigned int>(line_size));
	}

} // anonymouse namespace


namespace umdraw
{

/** 
 * import umdraw mesh list
 */
bool UMDirectX11IO::import_mesh_list(UMMeshList& dst, const umio::UMObjectPtr src, const umstring& absolute_file_path)
{
	return UMSoftwareIO::import_mesh_list(dst, src, absolute_file_path);
}

/**
 * convert umdraw mesh to directx11 mesh
 */
UMDirectX11MeshPtr UMDirectX11IO::convert_mesh_to_dx11_mesh(
	ID3D11Device *device_pointer,
	UMMeshPtr src)
{
	if (!device_pointer) return UMDirectX11MeshPtr();
	if (!src) return UMDirectX11MeshPtr();
	
	ID3D11DeviceContext* device_context(NULL);
	device_pointer->GetImmediateContext(&device_context);

	UMDirectX11MeshPtr mesh(std::make_shared<UMDirectX11Mesh>());
	
	//// create index buffer
	//if (src->face_list().size() > 0) {
	//	size_t size = src->face_list().size();

	//	D3D11_BUFFER_DESC desc;
	//	ZeroMemory(&desc, sizeof(desc));
	//	desc.ByteWidth      = static_cast<UINT>(sizeof(UMVec3i) * size);
	//	desc.Usage          = D3D11_USAGE_DYNAMIC;
	//	desc.BindFlags      = D3D11_BIND_INDEX_BUFFER;
	//	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	//	device_pointer->CreateBuffer(&desc, NULL, mesh->p_index_buffer_pointer());
	//	if (!mesh->index_buffer_pointer()) {
	//		SAFE_RELEASE(device_context);
	//		return UMDirectX11MeshPtr();
	//	} else {
	//		load_vertex_index(device_context, mesh, src);
	//	}
	//}

	// create vertex buffer
	if (src->vertex_list().size() > 0) {
		// create full triangle verts for uv
		size_t size = src->face_list().size() * 3;
		if (size == 0)
		{
			size = src->vertex_list().size();
		}

		D3D11_BUFFER_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.ByteWidth      = static_cast<UINT>(sizeof(UMVec3f) * size);
		desc.Usage          = D3D11_USAGE_DYNAMIC;
		desc.BindFlags      = D3D11_BIND_VERTEX_BUFFER;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		device_pointer->CreateBuffer(&desc, NULL, mesh->p_vertex_buffer_pointer());
		if (!mesh->vertex_buffer_pointer()) {
			SAFE_RELEASE(device_context);
			return UMDirectX11MeshPtr();
		} else {
			load_vertex(device_context, mesh, src);
		}
	}

	// create normal buffer
	if (src->normal_list().size() > 0) {
		// create full triangle normals for uv
		size_t size = src->face_list().size() * 3;
		if (size == 0)
		{
			size = src->vertex_list().size();
		}

		D3D11_BUFFER_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.ByteWidth      = static_cast<UINT>(sizeof(UMVec3f) * size);
		desc.Usage          = D3D11_USAGE_DYNAMIC;
		desc.BindFlags      = D3D11_BIND_VERTEX_BUFFER;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		device_pointer->CreateBuffer(&desc, 0, mesh->p_normal_buffer_pointer());	
		if (!mesh->normal_buffer_pointer()) {
			SAFE_RELEASE(device_context);
			return UMDirectX11MeshPtr();
		} else {
			load_normal(device_context, mesh, src);
		}
	}
		
	// create uv buffer
	if (src->uv_list().size() > 0) {
		size_t size = src->uv_list().size();

		D3D11_BUFFER_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.ByteWidth      = static_cast<UINT>(sizeof(UMVec2f) * size);
		desc.Usage          = D3D11_USAGE_DYNAMIC;
		desc.BindFlags      = D3D11_BIND_VERTEX_BUFFER;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		device_pointer->CreateBuffer(&desc, NULL, mesh->p_uv_buffer_pointer());
		if (!mesh->uv_buffer_pointer()) {
			SAFE_RELEASE(device_context);
			return UMDirectX11MeshPtr();
		} else {
			load_uv(device_context, mesh, src);
		}
	}

	// convert material
	if (src->material_list().size() > 0) {
		const int material_size = static_cast<int>(src->material_list().size());
		for (int i = 0; i < material_size; ++i)
		{
			UMMaterialPtr material = src->material_list().at(i);
			UMDirectX11MaterialPtr dx_material = UMDirectX11IO::convert_material_to_dx11_material(device_pointer, material);
			if (dx_material)
			{
				mesh->mutable_material_list().push_back(dx_material);
			}
		}
	}

	mesh->set_ummesh(src);
		
	SAFE_RELEASE(device_context);
	return mesh;
}

/**
 * convert umdraw mesh group to directx11 mesh
 */
UMDirectX11MeshGroupPtr UMDirectX11IO::convert_mesh_group_to_dx11_mesh_group(
	ID3D11Device *device_pointer,
	UMMeshGroupPtr src)
{
	if (!device_pointer) return UMDirectX11MeshGroupPtr();
	if (!src) return UMDirectX11MeshGroupPtr();
	
	UMDirectX11MeshGroupPtr group(std::make_shared<UMDirectX11MeshGroup>());
	group->init(device_pointer);
	group->set_ummesh_group(src);
	UMMeshList::const_iterator it = src->mesh_list().begin();
	for (; it != src->mesh_list().end(); ++it)
	{
		UMMeshPtr ummesh = *it;
		UMDirectX11MeshPtr mesh = UMDirectX11IO::convert_mesh_to_dx11_mesh(device_pointer, ummesh);
		if (!mesh) return  UMDirectX11MeshGroupPtr();

		mesh->set_ummesh(ummesh);
		group->mutable_dx11_mesh_list().push_back(mesh);
	}
	return group;
}

/**
 * convert umdraw line to directx11 line
 * @param [in] device directx11 device pointer
 * @param [in] src source umdraw mesh group
 * @retval UMDirectX11LinePtr converted dx11 line or none
 */
UMDirectX11LinePtr UMDirectX11IO::convert_line_to_dx11_line(
	ID3D11Device *device_pointer,
	UMLinePtr src)
{
	if (!device_pointer) return UMDirectX11LinePtr();
	if (!src) return UMDirectX11LinePtr();
	
	ID3D11DeviceContext* device_context(NULL);
	device_pointer->GetImmediateContext(&device_context);

	UMDirectX11LinePtr line(std::make_shared<UMDirectX11Line>());
	
	// create vertex buffer
	if (src->line_list().size() > 0) {
		// create full triangle verts for uv
		size_t size = src->line_list().size() * 2;

		D3D11_BUFFER_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.ByteWidth      = static_cast<UINT>(sizeof(UMVec3f) * size);
		desc.Usage          = D3D11_USAGE_DYNAMIC;
		desc.BindFlags      = D3D11_BIND_VERTEX_BUFFER;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		device_pointer->CreateBuffer(&desc, NULL, line->p_vertex_buffer_pointer());	
		if (!line->vertex_buffer_pointer()) {
			SAFE_RELEASE(device_context);
			return UMDirectX11LinePtr();
		} else {
			load_line(device_context, line, src);
		}
	}

	{
		// convert material
		if (src->material_list().size() > 0) {
			const int material_size = static_cast<int>(src->material_list().size());
			for (int i = 0; i < material_size; ++i)
			{
				UMMaterialPtr material = src->material_list().at(i);
				UMDirectX11MaterialPtr dx_material = UMDirectX11IO::convert_material_to_dx11_material(device_pointer, material);
				dx_material->set_shader_flags(UMVec4f(0, 1, 0, 0));
				if (dx_material)
				{
					line->mutable_material_list().push_back(dx_material);
				}
			}
		}
	}
	SAFE_RELEASE(device_context);
	return line;
}

/**
 * convert umdraw light to directx11 light
 */
UMDirectX11LightPtr UMDirectX11IO::convert_light_to_dx11_light(
	ID3D11Device *device_pointer,
	UMLightPtr src)
{
	if (!device_pointer) return UMDirectX11LightPtr();
	if (!src) return UMDirectX11LightPtr();

	UMDirectX11LightPtr light(std::make_shared<UMDirectX11Light>(src));
	light->init(device_pointer);
	return light;
}

/**
 * convert umdraw material to directx11 material
 */
UMDirectX11MaterialPtr UMDirectX11IO::convert_material_to_dx11_material(
	ID3D11Device *device_pointer,
	UMMaterialPtr src)
{
	if (!device_pointer) return UMDirectX11MaterialPtr();
	if (!src) return UMDirectX11MaterialPtr();

	UMDirectX11MaterialPtr material(std::make_shared<UMDirectX11Material>(src));
	material->init(device_pointer);
	return material;
}

/**
 * convert umdraw camera to directx11 camera
 */
UMDirectX11CameraPtr UMDirectX11IO::convert_camera_to_dx11_camera(
	ID3D11Device *device_pointer,
	UMCameraPtr src)
{
	if (!device_pointer) return UMDirectX11CameraPtr();
	if (!src) return UMDirectX11CameraPtr();

	UMDirectX11CameraPtr camera(std::make_shared<UMDirectX11Camera>(src));
	camera->init(device_pointer);
	return camera;
}

} // umdraw

#endif // WITH_DIRECTX
