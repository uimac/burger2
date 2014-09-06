/**
 * @file UMSubdivisioncene.cpp
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#include "UMSubdivision.h"

#include <string>

#include "UMStringUtil.h"
#include "UMPath.h"
#include "UMMesh.h"
#include "UMSceneAccess.h"

#ifdef WITH_OSD
	#define and &&
	#define and_eq &=
	#define bitand &
	#define bitor |
	#define compl ~
	#define not !
	#define not_eq !=
	#define or ||
	#define or_eq |=
	#define xor ^
	#define xor_eq ^=

	#include <far/meshFactory.h>
	#include <osd/mesh.h>
	#include <osd/cpuComputeController.h>
	#include <osd/cpuComputeContext.h>
	#include <osd/cpuVertexBuffer.h>
#endif // WITH_OSD

namespace umrt
{
	
#ifdef WITH_OSD

namespace
{
	using namespace OpenSubdiv;

	// vertex for opensubdiv osdvertex
	// http://graphics.pixar.com/opensubdiv/docs/using_osd_hbr.html
	class UMSubdivVertex
	{
	public:
		umbase::UMVec3f pos;

		UMSubdivVertex() {}
		UMSubdivVertex(int pos) { ; }
		UMSubdivVertex(const umbase::UMVec3f& pos_) { pos = pos_; }
		UMSubdivVertex(const UMSubdivVertex& src) { pos = src.pos; }

		void AddWithWeight(const UMSubdivVertex& src, float weight, void * = 0) 
		{
			pos += src.pos * weight;
		}

		void AddVaryingWithWeight(const UMSubdivVertex&, float, void * = 0) {}

		void Clear(void * = 0) {
			pos[0] = pos[1] = pos[2] = 0.0;
		}

		void ApplyVertexEdit(HbrVertexEdit<UMSubdivVertex> const & edit)
		{
			const float* src = edit.GetEdit();
			int operation = edit.GetOperation();
			if (operation == HbrHierarchicalEdit<UMSubdivVertex>::Set)
			{
				pos[0] = src[0];
				pos[1] = src[1];
				pos[2] = src[2];
			}
			else if (operation == HbrHierarchicalEdit<UMSubdivVertex>::Add)
			{
				pos[0] += src[0];
				pos[1] += src[1];
				pos[2] += src[2];
			}
			else if (operation == HbrHierarchicalEdit<UMSubdivVertex>::Subtract)
			{
				pos[0] -= src[0];
				pos[1] -= src[1];
				pos[2] -= src[2];
			}
		}
		
		void ApplyMovingVertexEdit(const HbrMovingVertexEdit<UMSubdivVertex> &) {}

		void set_position(const UMVec3d& src) {
			pos[0] = static_cast<float>(src[0]);
			pos[1] = static_cast<float>(src[1]);
			pos[2] = static_cast<float>(src[2]);
		}
	};

	static HbrCatmarkSubdivision<UMSubdivVertex> scheme;
}

/**
 * subdivision implementation
 */
class UMSubdivision::SudivImpl
{
public:
	SudivImpl(umdraw::UMMeshPtr mesh)
		: mesh_(mesh)
	{}

	~SudivImpl() {
		delete divided_mesh_;
		divided_mesh_ = NULL;
	}

	/// create subdivided mesh
	umdraw::UMMeshPtr create_subdivided_mesh(unsigned int level)
	{
		if (!mesh_) return umdraw::UMMeshPtr();
		if (level == 0) return umdraw::UMMeshPtr();

		// find from cache
		if (divided_mesh_)
		{
			if (umdraw::UMMeshPtr result_mesh = create_result_mesh(divided_mesh_, level))
			{
				return result_mesh;
			}
		}

		// initialize base mesh
		if (!base_mesh_)
		{
			if (!init_base_mesh())
			{
				return umdraw::UMMeshPtr();
			}
		}
		
		// get divided mesh
		const int max_level = level;
		OpenSubdiv::FarMeshFactory<UMSubdivVertex> mesh_factory(&(*base_mesh_), max_level);
		divided_mesh_ = mesh_factory.Create();
		if (divided_mesh_)
		{
			if (umdraw::UMMeshPtr result_mesh = create_result_mesh(divided_mesh_, level))
			{
				return result_mesh;
			}
		}
		return umdraw::UMMeshPtr();
	}
	
private:
	umdraw::UMMeshPtr mesh_;
	typedef OpenSubdiv::FarMesh<UMSubdivVertex> DevidedMesh;
	typedef std::shared_ptr< OpenSubdiv::HbrMesh<UMSubdivVertex> > SubdivMeshPtr;
	SubdivMeshPtr base_mesh_;
	DevidedMesh* divided_mesh_;

	// int base mesh
	bool init_base_mesh()
	{
		// set up initial mesh 
		base_mesh_ = std::make_shared< OpenSubdiv::HbrMesh<UMSubdivVertex> >(&scheme);
		UMSubdivVertex vtx;
		const int vertex_size = static_cast<int>(mesh_->vertex_list().size());
		const int face_size = static_cast<int>(mesh_->face_list().size());
		// create vertex
		for (int i = 0; i < vertex_size; ++i)
		{
			OpenSubdiv::HbrVertex<UMSubdivVertex>* v;
			v = base_mesh_->NewVertex(i, vtx);
			v->GetData().set_position(mesh_->vertex_list().at(i));
		}
		// create face
		for(int i = 0; i < face_size; ++i)
		{
			const umbase::UMVec3i& index = mesh_->face_list().at(i);
			// create face
			base_mesh_->NewFace(3, &index[0], 0);
		}
		base_mesh_->Finish();
		const int disconnected = base_mesh_->GetNumDisconnectedVertices();
		if (disconnected > 0) {
			// find disconnected verts
			return false;
		}
		
		// vertex varying data
		base_mesh_->SetInterpolateBoundaryMethod(
			OpenSubdiv::HbrMesh<UMSubdivVertex>::k_InterpolateBoundaryEdgeOnly);

		//// face varying data
		//base_mesh_->SetFVarInterpolateBoundaryMethod(
		//	OpenSubdiv::HbrMesh<UMSubdivVertex>::k_InterpolateBoundaryEdgeOnly);
		return true;
	}

	
	// create result mesh
	umdraw::UMMeshPtr create_result_mesh(DevidedMesh* divided_mesh, unsigned int level)
	{
		if (!divided_mesh) return umdraw::UMMeshPtr();
		// result mesh
		umdraw::UMMeshPtr result = std::make_shared<umdraw::UMMesh>();
		
		const std::vector<UMSubdivVertex>& vertex = divided_mesh->GetVertices();
		const int veretx_size = static_cast<int>(vertex.size());
		const unsigned int* face = divided_mesh->GetPatchTables()->GetFaceVertices();
		const int face_size = divided_mesh->GetPatchTables()->GetNumFaces();
		// important: base vertex index changed by level!
		const unsigned int base_vertex_index = divided_mesh->GetSubdivisionTables()->GetNumVerticesTotal(level - 1);
		
		OpenSubdiv::OsdCpuComputeContext* context
			= OpenSubdiv::OsdCpuComputeContext::Create(reinterpret_cast<const FarMesh<OsdVertex>* >(divided_mesh));
		OpenSubdiv::OsdCpuComputeController* controller
			= new OpenSubdiv::OsdCpuComputeController();
		OpenSubdiv::OsdCpuVertexBuffer *vertex_buffer
			= OpenSubdiv::OsdCpuVertexBuffer::Create(3, divided_mesh->GetNumVertices());
		vertex_buffer->UpdateData(&(vertex[0].pos[0]), 0, divided_mesh->GetNumVertices());

		// vertex will replace subdivided verts.
		controller->Refine(context, divided_mesh->GetKernelBatches(), vertex_buffer);

		// assing refined verts
		result->mutable_vertex_list().resize(veretx_size);
		float * refined_vertex = vertex_buffer->BindCpuBuffer() + (3 * base_vertex_index);
		for (size_t i = 0; i < veretx_size; ++i)
		{
			result->mutable_vertex_list().at(i) = UMVec3d(
				refined_vertex[i * 3 + 0],
				refined_vertex[i * 3 + 1],
				refined_vertex[i * 3 + 2]);
		}
		// assign faces as triangle
		const int triangle_count = face_size * 2;
		result->mutable_face_list().resize(triangle_count);
		for (int i = 0; i < triangle_count; ++i)
		{
			unsigned int f0 = face[ i * 4 + 0 ] - base_vertex_index;
			unsigned int f1 = face[ i * 4 + 1 ] - base_vertex_index;
			unsigned int f2 = face[ i * 4 + 2 ] - base_vertex_index;
			unsigned int f3 = face[ i * 4 + 3 ] - base_vertex_index;
			result->mutable_face_list().at(i * 2 + 0) = UMVec3i(f0, f1, f2);
			result->mutable_face_list().at(i * 2 + 1) = UMVec3i(f0, f2, f3);
		}

		// TODO: calc normals by subdiv
		result->create_normals(true);
		result->mutable_material_list().push_back(mesh_->mutable_material_list().at(0));
		result->mutable_material_list().at(0)->set_polygon_count(triangle_count);
		result->update_box();

		delete vertex_buffer;
		delete context;
		delete controller;
		return result;
	}
};
#else

/**
 * subdivision implementation
 */
class UMSubdivision::SudivImpl
{
public:
	SudivImpl(umdraw::UMMeshPtr mesh){}
	~SudivImpl() {}
};

#endif // WITH_OSD

/**
 * constructor
 */
UMSubdivision::UMSubdivision(umdraw::UMMeshPtr mesh)
	: impl_(new UMSubdivision::SudivImpl(mesh))
{
}

/**
 * destructor
 */
UMSubdivision::~UMSubdivision()
{
}

/**
 * get subdivided mesh
 */
umdraw::UMMeshPtr UMSubdivision::subdivided_mesh(unsigned int level)
{
#ifdef WITH_OSD
	return impl_->create_subdivided_mesh(level);
#else
	return umdraw::UMMeshPtr();
#endif
}

} // umrt
