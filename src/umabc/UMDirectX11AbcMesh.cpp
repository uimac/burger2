/**
 * @file UMDirectX11AbcMesh.cpp
 * any
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#ifdef WITH_DIRECTX

#include "UMDirectX11AbcMesh.h"

#include "UMAbcMesh.h"
#include "UMAbcConvert.h"

#include "UMDirectX11ShaderManager.h"
#include "UMDirectX11Material.h"
#include "UMDirectX11Texture.h"
#include "UMDirectX11ShaderManager.h"
#include "UMDirectX11Mesh.h"

namespace umabc
{
	
	using namespace Alembic::Abc;
	using namespace Alembic::AbcGeom;
	
/**
 * constructor
 */
UMDirectX11AbcMesh::UMDirectX11AbcMesh(UMAbcMeshPtr abc_mesh)
	: abc_mesh_(abc_mesh)
	, dx_mesh_(std::make_shared<umdraw::UMDirectX11Mesh>())
{}

/**
 * destructor
 */
UMDirectX11AbcMesh::~UMDirectX11AbcMesh()
{
}

/** 
 * update normal
 */
void UMDirectX11AbcMesh::update_normal(
	ID3D11Device* device_pointer, 
	const std::vector<UMVec3ui >& triangle_index, 
	const std::vector<Imath::V3f>& normal,
	VaryingType type)
{
	if (!dx_mesh_) return;
	if (!device_pointer) return;
	ID3D11DeviceContext *device_context = NULL;
	device_pointer->GetImmediateContext(&device_context);
		
	const size_t index_size = static_cast<int>(triangle_index.size());

	// vertex varying
	if (type == eVertexVarying)
	{
		const size_t normal_size = normal.size();
		if (triangle_index.empty() || index_size * 3 == normal_size)
		{
			if (!dx_mesh_->normal_buffer_pointer())
			{
				D3D11_BUFFER_DESC desc;
				ZeroMemory(&desc, sizeof(desc));
				desc.ByteWidth      = static_cast<UINT>(sizeof(UMVec3f) * normal_size);
				desc.Usage          = D3D11_USAGE_DYNAMIC;
				desc.BindFlags      = D3D11_BIND_VERTEX_BUFFER;
				desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

				device_pointer->CreateBuffer(&desc, 0, dx_mesh_->p_normal_buffer_pointer());
				if (!dx_mesh_->normal_buffer_pointer()) {
					SAFE_RELEASE(device_context);
					return;
				}
			}
			
			D3D11_MAPPED_SUBRESOURCE resource;
			device_context->Map(dx_mesh_->normal_buffer_pointer(), 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
			Imath::V3f* dx_normal = (Imath::V3f*)resource.pData;
			if (triangle_index.empty())
			{
				// pure vertex varying
				const size_t face_size = normal_size / 3;
				for (size_t i = 0; i < face_size; ++i)
				{
					dx_normal[i * 3 + 0] = normal[i * 3 + 0];
					dx_normal[i * 3 + 1] = normal[i * 3 + 2];
					dx_normal[i * 3 + 2] = normal[i * 3 + 1];
				}
			}
			else if (index_size * 3 == normal.size())
			{
				// vertex varying but "vertex" is using vertexindex
				for (size_t i = 0; i < index_size; ++i)
				{
					const UMVec3ui& in = triangle_index.at(i);
					dx_normal[i * 3 + 0] = normal[i * 3 + 0];
					dx_normal[i * 3 + 1] = normal[i * 3 + 2];
					dx_normal[i * 3 + 2] = normal[i * 3 + 1];
				}
			}
			device_context->Unmap(dx_mesh_->normal_buffer_pointer(), 0);
			SAFE_RELEASE(device_context);
			return;
		}
	}

	// face varying
	{
		const size_t normal_size = index_size * 3;
		if (!dx_mesh_->normal_buffer_pointer())
		{
			D3D11_BUFFER_DESC desc;
			ZeroMemory(&desc, sizeof(desc));
			desc.ByteWidth      = static_cast<UINT>(sizeof(UMVec3f) * normal_size);
			desc.Usage          = D3D11_USAGE_DYNAMIC;
			desc.BindFlags      = D3D11_BIND_VERTEX_BUFFER;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

			device_pointer->CreateBuffer(&desc, 0, dx_mesh_->p_normal_buffer_pointer());
			if (!dx_mesh_->normal_buffer_pointer()) {
				SAFE_RELEASE(device_context);
				return;
			}
		}
		
		D3D11_MAPPED_SUBRESOURCE resource;
		device_context->Map(dx_mesh_->normal_buffer_pointer(), 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
		Imath::V3f* dx_normal = (Imath::V3f*)resource.pData;
		
		for (size_t i = 0; i < index_size; ++i)
		{
			const UMVec3ui& in = triangle_index.at(i);
			dx_normal[i * 3 + 0] = normal[in.x];
			dx_normal[i * 3 + 1] = normal[in.z];
			dx_normal[i * 3 + 2] = normal[in.y];
		}
		device_context->Unmap(dx_mesh_->normal_buffer_pointer(), 0);
		SAFE_RELEASE(device_context);
	}
}

/** 
 * update uv
 */
void UMDirectX11AbcMesh::update_uv(
	ID3D11Device* device_pointer, 
	const std::vector<UMVec3ui >& triangle_index, 
	const Alembic::AbcGeom::IV2fGeomParam::Sample& uv,
	VaryingType type)
{
	if (!dx_mesh_) return;
	if (!device_pointer) return;
	ID3D11DeviceContext *device_context = NULL;
	device_pointer->GetImmediateContext(&device_context);
	
	const V2f *uvs =  uv.getVals()->get();
	const size_t index_size = static_cast<int>(triangle_index.size());
	
	// vertex varying
	if (type == eVertexVarying)
	{
		const size_t uv_size = uv.getVals()->size();
		if (triangle_index.empty() || index_size * 3 == uv_size)
		{
			if (!dx_mesh_->uv_buffer_pointer())
			{
				D3D11_BUFFER_DESC desc;
				ZeroMemory(&desc, sizeof(desc));
				desc.ByteWidth      = static_cast<UINT>(sizeof(UMVec2f) * uv_size);
				desc.Usage          = D3D11_USAGE_DYNAMIC;
				desc.BindFlags      = D3D11_BIND_VERTEX_BUFFER;
				desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

				device_pointer->CreateBuffer(&desc, NULL, dx_mesh_->p_uv_buffer_pointer());
				if (!dx_mesh_->uv_buffer_pointer()) {
					SAFE_RELEASE(device_context);
					return;
				}
			}
			D3D11_MAPPED_SUBRESOURCE resource;
			device_context->Map(dx_mesh_->uv_buffer_pointer(), 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
			Imath::V2f* dx_uv = (Imath::V2f*) resource.pData;
			
			if (triangle_index.empty())
			{
				// pure vertex varying
				const size_t face_size = uv_size / 3;
				for (size_t i = 0; i < face_size; ++i)
				{
					dx_uv[i * 3 + 0] = uvs[i * 3 + 0];
					dx_uv[i * 3 + 1] = uvs[i * 3 + 2];
					dx_uv[i * 3 + 2] = uvs[i * 3 + 1];
					dx_uv[i * 3 + 0].y = 1.0f - dx_uv[i * 3 + 0].y;
					dx_uv[i * 3 + 1].y = 1.0f - dx_uv[i * 3 + 1].y;
					dx_uv[i * 3 + 2].y = 1.0f - dx_uv[i * 3 + 2].y;
				}
			}
			else if (index_size * 3 == uv_size)
			{
				// vertex varying but "vertex" is using vertexindex
				for (size_t i = 0; i < index_size; ++i)
				{
					const UMVec3ui& in = triangle_index.at(i);
					dx_uv[i * 3 + 0] = uvs[i * 3 + 0];
					dx_uv[i * 3 + 1] = uvs[i * 3 + 2];
					dx_uv[i * 3 + 2] = uvs[i * 3 + 1];
					dx_uv[i * 3 + 0].y = 1.0f - dx_uv[i * 3 + 0].y;
					dx_uv[i * 3 + 1].y = 1.0f - dx_uv[i * 3 + 1].y;
					dx_uv[i * 3 + 2].y = 1.0f - dx_uv[i * 3 + 2].y;
				}
			}
			device_context->Unmap(dx_mesh_->uv_buffer_pointer(), 0);
			SAFE_RELEASE(device_context);
			return;
		}
		// else try face varying
	}
	
	// face varying
	{
		const size_t uv_size = index_size * 3;
		if (!dx_mesh_->uv_buffer_pointer())
		{
			D3D11_BUFFER_DESC desc;
			ZeroMemory(&desc, sizeof(desc));
			desc.ByteWidth      = static_cast<UINT>(sizeof(UMVec2f) * uv_size);
			desc.Usage          = D3D11_USAGE_DYNAMIC;
			desc.BindFlags      = D3D11_BIND_VERTEX_BUFFER;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

			device_pointer->CreateBuffer(&desc, NULL, dx_mesh_->p_uv_buffer_pointer());
			if (!dx_mesh_->uv_buffer_pointer()) {
				SAFE_RELEASE(device_context);
				return;
			}
		}
		D3D11_MAPPED_SUBRESOURCE resource;
		device_context->Map(dx_mesh_->uv_buffer_pointer(), 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
		Imath::V2f* dx_uv = (Imath::V2f*) resource.pData;
		
		for (size_t i = 0; i < index_size; ++i)
		{
			const UMVec3ui& in = triangle_index.at(i);
			dx_uv[i * 3 + 0] = uvs[in.x];
			dx_uv[i * 3 + 1] = uvs[in.z];
			dx_uv[i * 3 + 2] = uvs[in.y];
			dx_uv[i * 3 + 0].y = 1.0f - dx_uv[i * 3 + 0].y;
			dx_uv[i * 3 + 1].y = 1.0f - dx_uv[i * 3 + 1].y;
			dx_uv[i * 3 + 2].y = 1.0f - dx_uv[i * 3 + 2].y;
		}

		device_context->Unmap(dx_mesh_->uv_buffer_pointer(), 0);
		SAFE_RELEASE(device_context);
	}
}

/**
 * update vertex
 */
void UMDirectX11AbcMesh::update_vertex(
	ID3D11Device* device_pointer, 
	const std::vector<UMVec3ui >& triangle_index, 
	const P3fArraySamplePtr& vertex)
{
	if (!dx_mesh_) return;
	if (!device_pointer) return;
	ID3D11DeviceContext *device_context = NULL;
	device_pointer->GetImmediateContext(&device_context);
	
	const V3f *points = vertex->get();
	if (triangle_index.empty())
	{
		vertex_size_ = static_cast<unsigned int>(vertex->size());
		if (!dx_mesh_->vertex_buffer_pointer())
		{
			D3D11_BUFFER_DESC desc;
			ZeroMemory(&desc, sizeof(desc));
			desc.ByteWidth      = static_cast<UINT>(sizeof(UMVec3f) * vertex_size_);
			desc.Usage          = D3D11_USAGE_DYNAMIC;
			desc.BindFlags      = D3D11_BIND_VERTEX_BUFFER;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

			device_pointer->CreateBuffer(&desc, NULL, dx_mesh_->p_vertex_buffer_pointer());
			if (!dx_mesh_->vertex_buffer_pointer()) {
				SAFE_RELEASE(device_context);
				return;
			}
		}

		D3D11_MAPPED_SUBRESOURCE resource;
		device_context->Map(dx_mesh_->vertex_buffer_pointer(), 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
		Imath::V3f* dx_vertex = (Imath::V3f*)resource.pData;

		const size_t face_size = vertex_size_ / 3;
		for (size_t i = 0; i < face_size; ++i)
		{
			dx_vertex[i * 3 + 0] = points[i * 3 + 0];
			dx_vertex[i * 3 + 1] = points[i * 3 + 2];
			dx_vertex[i * 3 + 2] = points[i * 3 + 1];
		}
		device_context->Unmap(dx_mesh_->vertex_buffer_pointer(), 0);
	}
	else
	{
		const size_t index_size = static_cast<int>(triangle_index.size());
		vertex_size_ = static_cast<unsigned int>(index_size * 3);
		if (!dx_mesh_->vertex_buffer_pointer())
		{
			D3D11_BUFFER_DESC desc;
			ZeroMemory(&desc, sizeof(desc));
			desc.ByteWidth      = static_cast<UINT>(sizeof(UMVec3f) * vertex_size_);
			desc.Usage          = D3D11_USAGE_DYNAMIC;
			desc.BindFlags      = D3D11_BIND_VERTEX_BUFFER;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

			device_pointer->CreateBuffer(&desc, NULL, dx_mesh_->p_vertex_buffer_pointer());
			if (!dx_mesh_->vertex_buffer_pointer()) {
				SAFE_RELEASE(device_context);
				return;
			}
		}

		D3D11_MAPPED_SUBRESOURCE resource;
		device_context->Map(dx_mesh_->vertex_buffer_pointer(), 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
		Imath::V3f* dx_vertex = (Imath::V3f*)resource.pData;
		
		for (size_t i = 0; i < index_size; ++i)
		{
			const UMVec3ui& in = triangle_index.at(i);
			dx_vertex[i * 3 + 0] = points[in.x];
			dx_vertex[i * 3 + 1] = points[in.z];
			dx_vertex[i * 3 + 2] = points[in.y];
		}
		device_context->Unmap(dx_mesh_->vertex_buffer_pointer(), 0);
	}
	SAFE_RELEASE(device_context);
}

///**
// * update vertex index
// */
//void UMDirectX11AbcMesh::update_vertex_index(
//	ID3D11Device* device_pointer, 
//	const std::vector<UMVec3ui >& triangle_index)
//{
//	if (!dx_mesh_) return;
//	if (!device_pointer) return;
//	ID3D11DeviceContext *device_context = NULL;
//	device_pointer->GetImmediateContext(&device_context);
//	
//	size_t triagnle_size = triangle_index.size();
//
//	if (!dx_mesh_->index_buffer_pointer())
//	{
//		D3D11_BUFFER_DESC desc;
//		ZeroMemory(&desc, sizeof(desc));
//		desc.ByteWidth      = static_cast<UINT>(sizeof(UMVec3i) * triagnle_size);
//		desc.Usage          = D3D11_USAGE_DYNAMIC;
//		desc.BindFlags      = D3D11_BIND_INDEX_BUFFER;
//		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
//
//		device_pointer->CreateBuffer(&desc, NULL, dx_mesh_->p_index_buffer_pointer());
//		if (!dx_mesh_->index_buffer_pointer()) {
//			SAFE_RELEASE(device_context);
//			return;
//		}
//	}
//
//	D3D11_MAPPED_SUBRESOURCE resource;
//	device_context->Map(dx_mesh_->index_buffer_pointer(), 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
//
//	UMVec3i* index = (UMVec3i*) resource.pData;
//	const std::vector<UMVec3ui >& face_list = triangle_index;
//	for (size_t i = 0, i_size = face_list.size(); i < i_size; ++i) {
//		// Alembic is CW
//		const UMVec3ui& in = triangle_index.at(i);
//		index[i].x = in.x;
//		index[i].y = in.z;
//		index[i].z = in.y;
//	}
//	device_context->Unmap(dx_mesh_->index_buffer_pointer(), 0);
//	SAFE_RELEASE(device_context);
//
//	triagnle_size_ = static_cast<unsigned int>(triagnle_size);
//}

/**
 * draw
 */
void UMDirectX11AbcMesh::draw(
	ID3D11Device* device_pointer) const 
{
	if (!dx_mesh_) return;
	if (!device_pointer) return;

	UMAbcMeshPtr abcm = abc_mesh();
	const std::string& object_name = abcm->name();

	dx_mesh_->draw(device_pointer, umdraw::UMDirectX11DrawParameterPtr());
}

} // umabc

#endif // WITH_DIRECTX

