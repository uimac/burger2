/**
 * @file UMAbcMesh.cpp
 * any
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#include "UMAbcMesh.h"
#include "UMOpenGLAbcMesh.h"
#include "UMDirectX11AbcMesh.h"
#include "UMDirectX11.h"
#include "UMStringUtil.h"

#include "UMAbcConvert.h"

namespace umabc
{
	
	using namespace Alembic::Abc;
	using namespace Alembic::AbcGeom;

	
/**
 * create
 */
UMAbcMeshPtr UMAbcMesh::create(Alembic::AbcGeom::IPolyMesh poly_mesh)
{
	UMAbcMeshPtr instance = UMAbcMeshPtr(new UMAbcMesh(poly_mesh));
	instance->self_reference_ = instance;
	return instance;
}

/**
 * destructor
 */
UMAbcMesh::~UMAbcMesh()
{
}

/**
 * initialize
 */
bool UMAbcMesh::init(bool recursive)
{
	if (!is_valid()) return false;
	
	initial_bounds_prop_ = poly_mesh_.getSchema().getSelfBoundsProperty();

	size_t num_samples = poly_mesh_.getSchema().getNumSamples();
	if (num_samples > 0)
	{
		// get constant sample
		poly_mesh_.getSchema().get(initial_sample_);
		
		// if not consistant, we get time
		if (!poly_mesh_.getSchema().isConstant())
		{
			TimeSamplingPtr time = poly_mesh_.getSchema().getTimeSampling();
			min_time_ = static_cast<unsigned long>(time->getSampleTime(0)*1000);
			max_time_ = static_cast<unsigned long>(time->getSampleTime(num_samples-1)*1000);
		}
	}
	
	faceset_name_list_.clear();
	faceset_names_.clear();
	poly_mesh_.getSchema().getFaceSetNames(faceset_names_);
	
	ISampleSelector selector(current_time(), ISampleSelector::kNearIndex);

	for (int i = 0, size = static_cast<int>(faceset_names_.size()); i < size; ++i)
	{
		std::string name = faceset_names_.at(i);
		faceset_name_list_.push_back(umbase::UMStringUtil::utf8_to_utf16(name));
		
		IFaceSet faceset = poly_mesh_.getSchema().getFaceSet(name);
		IFaceSetSchema::Sample faceset_sample;
		faceset.getSchema().get(faceset_sample, selector);
		
		Int32ArraySamplePtr faces = faceset_sample.getFaces();
		size_t facesize = faces->size();
		faceset_[name] = faceset_sample;
	}

#ifdef WITH_OPENGL
	if (UMAbcMeshPtr self = self_reference_.lock())
	{
		opengl_mesh_ = UMOpenGLAbcMeshPtr(new UMOpenGLAbcMesh(self));
	}
#endif // WITH_OPENGL

#ifdef WITH_DIRECTX
	if (UMAbcMeshPtr self = self_reference_.lock())
	{
		directx_mesh_ = UMDirectX11AbcMeshPtr(new UMDirectX11AbcMesh(self));
	}
#endif // WITH_DIRECTX

	return UMAbcObject::init(recursive);
}

/**
 * set current time
 */
void UMAbcMesh::set_current_time(unsigned long time, bool recursive)
{
	if (!is_valid()) return;
	
	UMAbcObject::set_current_time(time, recursive);

	update_mesh_all();
}

/** 
 * update normal
 */
void UMAbcMesh::update_normal()
{
	if (!is_valid()) return;
	ISampleSelector selector(current_time(), ISampleSelector::kNearIndex);
	IN3fGeomParam normal_param = poly_mesh_.getSchema().getNormalsParam();
	
	// get scope and sample
	const GeometryScope scope = normal_param.getScope();
	if (scope == kVaryingScope
		|| scope == kVertexScope
		|| scope == kFacevaryingScope)
	{
		if (normal_param.isIndexed())
		{
			normal_param.getIndexed(normal_, selector);
		}
		else
		{
			normal_param.getExpanded(normal_, selector);
		}
	}

	bool is_vertex_varying = false;

	if (!normal_ || normal_.getVals()->size() <= 0)
	{
		// make vertex varying normals
		is_vertex_varying = true;
		original_normal_.resize(vertex_->size());
		for (size_t i = 0, isize = original_normal_.size(); i < isize; ++i)
		{
			original_normal_[i] = Imath::V3f(0);
		}
		for (size_t i = 0, isize = triangle_index_.size(); i < isize; ++i)
		{
			const UMVec3ui& index = triangle_index_.at(i);
			const V3f& v0 = (*vertex_)[index[0]];
			const V3f& v1 = (*vertex_)[index[1]];
			const V3f& v2 = (*vertex_)[index[2]];
			V3f normal = (v1-v0).cross(v2-v0);
			original_normal_[index[0]] += normal;
			original_normal_[index[1]] += normal;
			original_normal_[index[2]] += normal;
		}
		// normalize
		for (size_t i = 0, isize = vertex_->size(); i < isize; ++i)
		{
			original_normal_[i].normalize();
		}
	}
	else
	{
		const V3f *normals =  normal_.getVals()->get();
		size_t normal_size = normal_.getVals()->size();
		size_t vertex_size = vertex_->size();
		is_vertex_varying = normal_size == vertex_size;

		original_normal_.resize(normal_size);
		for (size_t i = 0; i < normal_size; ++i)
		{
			original_normal_[i] = normals[i];
			original_normal_[i].normalize();
		}
	}
	
#ifdef WITH_OPENGL
	if (opengl_mesh_ && !umdraw::UMDirectX11::current_device_pointer())
	{
		if (scope == kFacevaryingScope)
		{
			opengl_mesh_->update_normal(triangle_index_, original_normal_, UMOpenGLAbcMesh::eFaceVarying, is_cw_);
		}
		else
		{
			opengl_mesh_->update_normal(triangle_index_, original_normal_, UMOpenGLAbcMesh::eVertexVarying, is_cw_);
		}
	}
#endif // WITH_OPENGL

#ifdef WITH_DIRECTX
	if (directx_mesh_ && umdraw::UMDirectX11::current_device_pointer())
	{
		if (scope == kFacevaryingScope)
		{
			directx_mesh_->update_normal(
				reinterpret_cast<ID3D11Device*>(umdraw::UMDirectX11::current_device_pointer()),
				triangle_index_,
				original_normal_,
				UMDirectX11AbcMesh::eFaceVarying);
		}
		else
		{
			directx_mesh_->update_normal(
				reinterpret_cast<ID3D11Device*>(umdraw::UMDirectX11::current_device_pointer()),
				triangle_index_,
				original_normal_,
				UMDirectX11AbcMesh::eVertexVarying);
		}
	}
#endif // WITH_DIRECTX
}

/** 
 * update uv
 */
void UMAbcMesh::update_uv()
{
	if (!is_valid()) return;
	ISampleSelector selector(current_time(), ISampleSelector::kNearIndex);
	IV2fGeomParam uv_param = poly_mesh_.getSchema().getUVsParam();

	// get scope and sample
	const GeometryScope scope = uv_param.getScope();
	if (scope == kVaryingScope
		|| scope == kVertexScope
		|| scope == kFacevaryingScope)
	{
		if (uv_param.isIndexed())
		{
			uv_param.getIndexed(uv_, selector);
		}
		else
		{
			uv_param.getExpanded(uv_, selector);
		}
	}
	
#ifdef WITH_OPENGL
	if (opengl_mesh_ && !umdraw::UMDirectX11::current_device_pointer())
	{
		if (scope == kFacevaryingScope)
		{
			opengl_mesh_->update_uv(triangle_index_, uv_, UMOpenGLAbcMesh::eFaceVarying, is_cw_);
		}
		else
		{
			opengl_mesh_->update_uv(triangle_index_, uv_, UMOpenGLAbcMesh::eVertexVarying, is_cw_);
		}
	}
#endif // WITH_OPENGL

#ifdef WITH_DIRECTX
	if (directx_mesh_ && umdraw::UMDirectX11::current_device_pointer())
	{
		if (scope == kFacevaryingScope)
		{
			directx_mesh_->update_uv(
				reinterpret_cast<ID3D11Device*>(triangle_index_, umdraw::UMDirectX11::current_device_pointer()),
				triangle_index_,
				uv_,
				UMDirectX11AbcMesh::eFaceVarying);
		}
		else
		{
			directx_mesh_->update_uv(
				reinterpret_cast<ID3D11Device*>(triangle_index_, umdraw::UMDirectX11::current_device_pointer()),
				triangle_index_,
				uv_,
				UMDirectX11AbcMesh::eVertexVarying);
		}
	}
#endif // WITH_DIRECTX
}

/**
 * update vertex
 */
void UMAbcMesh::update_vertex(IPolyMeshSchema::Sample& sample)
{
	if (!is_valid()) return;
	
	P3fArraySamplePtr vertex = sample.getPositions();

	vertex_ = vertex;
	
#ifdef WITH_OPENGL
	if (opengl_mesh_ && !umdraw::UMDirectX11::current_device_pointer())
	{
		opengl_mesh_->update_vertex(triangle_index_, vertex_, is_cw_);
	}
#endif // WITH_OPENGL

#ifdef WITH_DIRECTX
	if (directx_mesh_ && umdraw::UMDirectX11::current_device_pointer())
	{
		directx_mesh_->update_vertex(
			reinterpret_cast<ID3D11Device*>(umdraw::UMDirectX11::current_device_pointer()),
			triangle_index_,
			vertex_);
	}
#endif // WITH_DIRECTX
}

/**
 * update vertex index
 */
void UMAbcMesh::update_vertex_index_by_faceset(IPolyMeshSchema::Sample& sample)
{
	P3fArraySamplePtr vertex = sample.getPositions();
	Int32ArraySamplePtr vertex_index = sample.getFaceIndices();
	Int32ArraySamplePtr face_count = sample.getFaceCounts();
	if (!vertex) return;
	if (!vertex_index) return;
	if (!face_count) return;
	
	vertex_index_ = vertex_index;
	face_count_ = face_count;

	// update index buffer
	triangle_index_.clear();
	faceset_polycount_list_.clear();
	faceset_name_list_.clear();
	const size_t vertex_size = vertex->size();
	const size_t vertex_index_size_ = vertex_index_->size();
	const size_t face_count_size = face_count_->size();
	if (vertex_size <= 0) return;
	if (vertex_index_size_ <= 0) return;
	if (face_count_size <= 0) return;

	size_t face_index_begin = 0;
	size_t face_index_end = 0;
	std::vector<int> face_begin_index_list;
	face_begin_index_list.reserve(face_count_size);
	for (size_t face = 0; face < face_count_size; ++face)
	{
		face_index_begin = face_index_end;
		size_t count = (*face_count_)[face];
		face_begin_index_list.push_back(face_index_begin);
		face_index_end = face_index_begin + count;
		
		//if (face_index_end > vertex_index_size_ ||
		//	face_index_end < face_index_begin)
		//{
		//	break;
		//}
	}
	
	faceset_names_.clear();
	poly_mesh_.getSchema().getFaceSetNames(faceset_names_);
		
	ISampleSelector selector(current_time(), ISampleSelector::kNearIndex);
	for (int i = 0, size = static_cast<int>(faceset_names_.size()); i < size; ++i)
	{
		std::string name = faceset_names_.at(i);
		faceset_name_list_.push_back(umbase::UMStringUtil::utf8_to_utf16(name));
		
		IFaceSet faceset = poly_mesh_.getSchema().getFaceSet(name);
		IFaceSetSchema::Sample faceset_sample;
		faceset.getSchema().get(faceset_sample, selector);
		Int32ArraySamplePtr faces = faceset_sample.getFaces();
		size_t facesize = faces->size();
		//Int32ArraySamplePtr faces = faceset_[name].getFaces();
		int pre_polygon_count = static_cast<int>(triangle_index_.size());
		
		for (int k = 0; k < faces->size(); ++k)
		{
			int face = faces->get()[k];
			size_t begin_index = face_begin_index_list[face];
			size_t count = (*face_count_)[face];
			size_t end_index = begin_index + count;

			bool is_good_face = true;
			for (size_t n = begin_index; n < end_index; ++n)
			{
				if (static_cast<size_t>((*vertex_index_)[n]) >= vertex_size)
				{
					is_good_face = false;
					break;
				}
			}
			if (!is_good_face) break;

			if (count > 2)
			{
				if (is_cw_)
				{
					// this is alembic default
					triangle_index_.push_back(
						UMVec3ui(
						(*vertex_index_)[begin_index + 0],
						(*vertex_index_)[begin_index + 1],
						(*vertex_index_)[begin_index + 2]));
				}
				else
				{
					// wrong data. flip.
					triangle_index_.push_back(
						UMVec3ui(
						(*vertex_index_)[begin_index + 0],
						(*vertex_index_)[begin_index + 2],
						(*vertex_index_)[begin_index + 1]));
				}

				for (size_t n = 3; n < count; ++n)
				{
					if (is_cw_)
					{
						triangle_index_.push_back(
							UMVec3ui(
								(*vertex_index_)[begin_index + 0],
								(*vertex_index_)[begin_index + n-1],
								(*vertex_index_)[begin_index + n]));
					}
					else
					{
						triangle_index_.push_back(
							UMVec3ui(
								(*vertex_index_)[begin_index + 0],
								(*vertex_index_)[begin_index + n],
								(*vertex_index_)[begin_index + n-1]));
					}
				}
			}
		}
		faceset_polycount_list_.push_back(static_cast<int>(triangle_index_.size()) - pre_polygon_count);
	}
		
	update_material();
}

/**
 * update vertex index
 */
void UMAbcMesh::update_vertex_index(IPolyMeshSchema::Sample& sample)
{
	if (!is_valid()) return;

	if (!faceset_names_.empty())
	{
		update_vertex_index_by_faceset(sample);
		return;
	}
	
	P3fArraySamplePtr vertex = sample.getPositions();
	Int32ArraySamplePtr vertex_index = sample.getFaceIndices();
	Int32ArraySamplePtr face_count = sample.getFaceCounts();
	if (!vertex) return;
	if (!vertex_index) return;
	if (!face_count) return;
	
	vertex_index_ = vertex_index;
	face_count_ = face_count;

	// update index buffer
	triangle_index_.clear();
	faceset_polycount_list_.clear();
	const size_t vertex_size = vertex->size();
	const size_t vertex_index_size_ = vertex_index_->size();
	const size_t face_count_size = face_count_->size();
	if (vertex_size <= 0) return;
	if (vertex_index_size_ <= 0) return;
	if (face_count_size <= 0) return;

	size_t face_index_begin = 0;
	size_t face_index_end = 0;

	for (size_t face = 0; face < face_count_size; ++face)
	{
		face_index_begin = face_index_end;
		size_t count = (*face_count_)[face];
		face_index_end = face_index_begin + count;

		if (face_index_end > vertex_index_size_ ||
			face_index_end < face_index_begin)
		{
			break;
		}

		bool is_good_face = true;
		for (size_t i = face_index_begin; i < face_index_end; ++i)
		{
			if (static_cast<size_t>((*vertex_index_)[i]) >= vertex_size)
			{
				is_good_face = false;
				break;
			}
		}
		if (!is_good_face) break;

		if (count > 2)
		{
			if (is_cw_)
			{
				// this is alembic default
				triangle_index_.push_back(
					UMVec3ui(
					(*vertex_index_)[face_index_begin + 0],
					(*vertex_index_)[face_index_begin + 1],
					(*vertex_index_)[face_index_begin + 2]));
			}
			else
			{
				// wrong data. flip.
				triangle_index_.push_back(
					UMVec3ui(
					(*vertex_index_)[face_index_begin + 0],
					(*vertex_index_)[face_index_begin + 2],
					(*vertex_index_)[face_index_begin + 1]));
			}

			for (size_t i = 3; i < count; ++i)
			{
				if (is_cw_)
				{
					triangle_index_.push_back(
						UMVec3ui(
							(*vertex_index_)[face_index_begin + 0],
							(*vertex_index_)[face_index_begin + i-1],
							(*vertex_index_)[face_index_begin + i]));
				}
				else
				{
					triangle_index_.push_back(
						UMVec3ui(
							(*vertex_index_)[face_index_begin + 0],
							(*vertex_index_)[face_index_begin + i],
							(*vertex_index_)[face_index_begin + i-1]));
				}
			}
		}
	}
}

/** 
 * update mesh all
 */
void UMAbcMesh::update_mesh_all()
{
	ISampleSelector selector(current_time(), ISampleSelector::kNearIndex);
	IPolyMeshSchema::Sample sample;

	if (poly_mesh_.getSchema().isConstant())
	{
		sample = initial_sample_;
	}
	else if (poly_mesh_.getSchema().getNumSamples() > 0)
	{
		poly_mesh_.getSchema().get(sample, selector);
	}

	P3fArraySamplePtr vertex = sample.getPositions();
	Int32ArraySamplePtr vertex_index = sample.getFaceIndices();
	Int32ArraySamplePtr face_count = sample.getFaceCounts();

	// update same size buffer
	if (vertex_ && vertex_index_ && face_count_)
	{
		if (vertex_->size() == vertex->size()
			&& vertex_index_->size() == vertex_index->size()
			&& face_count_->size() == face_count->size())
		{
			if (!faceset_names_.empty())
			{
				update_vertex_index(sample);
				//update_vertex(sample);
			}
			if (vertex_ == vertex)
			{
				update_normal();
				update_uv();
			}
			else
			{
				update_vertex(sample);
				update_normal();
				update_uv();
			}
			return;
		}
	}

	/// update different size buffer
	
	update_vertex_index(sample);
	update_vertex(sample);
	update_normal();
	update_uv();
}

void UMAbcMesh::update_material()
{
	umdraw::UMMaterialList sorted_material_list;
	sorted_material_list.reserve(material_list().size());

	for (int i = 0, size = static_cast<int>(faceset_names_.size()); i < size; ++i)
	{
		umstring name = faceset_name_list_.at(i);
		umdraw::UMMaterialList::iterator it = mutable_material_list().begin();
		for (; it != mutable_material_list().end(); ++it)
		{
			umdraw::UMMaterialPtr mat = *it;
			if (mat->name() == name)
			{
				if (mat->polygon_count() != faceset_polycount_list().at(i))
				{
					printf("diff %s, %s, %d, %d\n", umbase::UMStringUtil::utf16_to_utf8(name).c_str(), faceset_names_.at(i).c_str(), mat->polygon_count() ,faceset_polycount_list().at(i));
				}
				mat->set_polygon_count(faceset_polycount_list().at(i));
				sorted_material_list.push_back(mat);
				break;
			}
		}
	}
	if (material_list_.size() == sorted_material_list.size())
	{
		material_list_ = sorted_material_list;
	}
}

/**
 * update box
 */
void UMAbcMesh::update_box(bool recursive)
{
	if (!is_valid()) return;
	box_.init();

	if (initial_bounds_prop_ && initial_bounds_prop_.getNumSamples() > 0)
	{
		ISampleSelector selector(current_time(), ISampleSelector::kNearIndex);

		box_ = 
			UMAbcConvert::imath_box_to_um(
				initial_bounds_prop_.getValue(selector));
	}

	if (box_.is_empty() && vertex_)
	{
		size_t vertex_count = vertex_->size();
		for (size_t i = 0; i < vertex_count; ++i)
		{
			const Imath::V3f& p = (*vertex_)[i];
			box_.extend(UMAbcConvert::imath_vec_to_um(p));
		}
	}
}

/**
 * get polgon count
 */
int UMAbcMesh::polygon_count() const 
{
	return static_cast<int>(triangle_index_.size());
}

/**
 * draw
 */
void UMAbcMesh::draw(bool recursive, UMAbc::DrawType type)
{
	if (!is_valid()) return;
	if (triangle_index_.empty()) return;
	if (!vertex_) return;
	
#ifdef WITH_OPENGL
	if (type == UMAbc::eOpenGL && !umdraw::UMDirectX11::current_device_pointer())
	{
		if (opengl_mesh_)
		{
			opengl_mesh_->draw(umdraw::UMOpenGLDrawParameterPtr());
		}
	}
#endif // WITH_OPENGL

#ifdef WITH_DIRECTX
	if (type == UMAbc::eDirectX && umdraw::UMDirectX11::current_device_pointer())
	{
		if (directx_mesh_)
		{
			directx_mesh_->draw(
				reinterpret_cast<ID3D11Device*>(umdraw::UMDirectX11::current_device_pointer()));
		}
	}
#endif // WITH_DIRECTX
	
	UMAbcObject::draw(recursive, type);
}

/** 
 * get material from vertex index
 */
umdraw::UMMaterialPtr UMAbcMesh::material_from_face_index(int face_index) const
{
	int pos = 0;
	umdraw::UMMaterialList::const_iterator it = material_list_.begin();
	for (; it != material_list_.end(); ++it)
	{
		const int polygon_count = (*it)->polygon_count();
		if (face_index >= pos && face_index < (pos+polygon_count)) {
			return *it;
		}
		pos += polygon_count;
	}
	return umdraw::UMMaterialPtr();
}

} // umabc
