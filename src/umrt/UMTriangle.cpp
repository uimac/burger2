/**
 * @file UMTriangle.cpp
 * a triangle
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#include "UMTriangle.h"
#include "UMVector.h"

#ifdef WITH_ALEMBIC
	#include "UMAbcMesh.h"
#endif

namespace umrt
{
	using namespace umdraw;
	

/**
 * @param [in] mesh mesh
 * @param [in] vertex_index vertex index
 * @param [in] face_index face index
 */
UMTrianglePtr UMTriangle::create(umdraw::UMMeshPtr mesh, const UMVec3i& vertex_index, int face_index)
{
	UMTrianglePtr triangle(std::make_shared<UMTriangle>());
	triangle->mesh_ = mesh;
	triangle->vertex_index_ = vertex_index;
	triangle->face_index_ = face_index;
	triangle->update_box();
	return triangle;
}

/**
 * @param [in] mesh mesh
 * @param [in] vertex_index vertex index
 * @param [in] face_index face index
 */
UMTrianglePtr UMTriangle::create_from_abc_mesh(umabc::UMAbcMeshPtr mesh, const UMVec3i& vertex_index, int face_index)
{
	UMTrianglePtr triangle(std::make_shared<UMTriangle>());
	triangle->abc_mesh_ = mesh;
	triangle->vertex_index_ = vertex_index;
	triangle->face_index_ = face_index;
	triangle->update_box();
	return triangle;
}
	
/**
 * ray triangle intersection static version
 */
bool UMTriangle::intersects(
	const UMVec3d& a,
	const UMVec3d& b,
	const UMVec3d& c,
	const UMRay& ray)
{
	const UMVec3d& ray_dir = ray.direction();
	const UMVec3d& ray_orig = ray.origin();
	
	UMVec3d ab = b - a;
	UMVec3d ac = c - a;
	UMVec3d n = ab.cross(ac);

	// ray is parallel or no reach
	double d = (-ray_dir).dot(n);
	if (d < 0) return false;
	
	UMVec3d ao = ray_orig - a;
	double t = ao.dot(n);
	if (t < 0) return false;
	
	double inv_dir = 1.0 / d;
	double distance = t * inv_dir;
	if (distance < FLT_EPSILON) return false;

	// inside triangle ?
	UMVec3d barycentric = (-ray_dir).cross(ao);
	double v = ac.dot(barycentric);
	if (v < 0 || v > d) return false;
	double w = -ab.dot(barycentric);
	if (w < 0 || (v + w) > d) return false;

	return true;
}

/**
 * ray triangle intersection static version
 */
bool UMTriangle::intersects(
	const UMVec3d& a,
	const UMVec3d& b,
	const UMVec3d& c,
	const UMRay& ray,
	UMShaderParameter& parameter)
{
	
	const UMVec3d& ray_dir(ray.direction());
	const UMVec3d& ray_orig(ray.origin());
	
	UMVec3d ab = b - a;
	UMVec3d ac = c - a;
	UMVec3d n = ab.cross(ac);

	// ray is parallel or no reach
	double d = (-ray_dir).dot(n);
	if (d < 0) return false;
	
	UMVec3d ao = ray_orig - a;
	double t = ao.dot(n);
	if (t < 0) return false;

	double inv_dir = 1.0 / d;
	double distance = t * inv_dir;
	if (distance < ray.tmin()) return false;
	if (distance > ray.tmax()) return false;

	// inside triangle ?
	UMVec3d barycentric = (-ray_dir).cross(ao);
	double v = ac.dot(barycentric);
	if (v < 0 || v > d) return false;
	double w = -ab.dot(barycentric);
	if (w < 0 || (v + w) > d) return false;

	// v
	parameter.uvw.y = v * inv_dir;
	// w
	parameter.uvw.z = w * inv_dir;
	// u
	parameter.uvw.x = 1.0 - parameter.uvw.y - parameter.uvw.z;
	
	parameter.distance = distance;
	parameter.intersect_point = ray_orig + ray_dir * distance;
	//parameter.normal = n.normalized();

	return true;
}

/**
 * ray triangle intersection
 */
bool UMTriangle::intersects(const UMRay& ray, UMShaderParameter& parameter) const
{
	UMMeshPtr me = mesh();
	if (me)
	{
		// 3 points
		const UMVec3d& v0 = me->vertex_list()[vertex_index_.x];
		const UMVec3d& v1 = me->vertex_list()[vertex_index_.y];
		const UMVec3d& v2 = me->vertex_list()[vertex_index_.z];

		if (intersects(v0, v1, v2, ray, parameter))
		{
			parameter.face_index = face_index_;

			const UMVec3d& n0 = me->normal_list()[vertex_index_.x];
			const UMVec3d& n1 = me->normal_list()[vertex_index_.y];
			const UMVec3d& n2 = me->normal_list()[vertex_index_.z];
			parameter.normal = (n0 * parameter.uvw.x + n1 * parameter.uvw.y + n2 * parameter.uvw.z).normalized();

			if (UMMaterialPtr material = me->material_from_face_index(face_index_))
			{
				parameter.material = material;

				const UMVec4d& diffuse = material->diffuse();
				parameter.color.x = diffuse.x;
				parameter.color.y = diffuse.y;
				parameter.color.z = diffuse.z;
				parameter.emissive = material->emissive().xyz() * material->emissive_factor();
				if (!me->uv_list().empty() && !material->texture_list().empty()) {
					// uv
					const int base = face_index_ * 3;
					const UMVec2d& uv0 = me->uv_list()[base + 0];
					const UMVec2d& uv1 = me->uv_list()[base + 1];
					const UMVec2d& uv2 = me->uv_list()[base + 2];
					UMVec2d uv = UMVec2d(
						uv0 * parameter.uvw.x +
						uv1 * parameter.uvw.y +
						uv2 * parameter.uvw.z);
					uv.x = umbase::um_clip(uv.x);
					uv.y = umbase::um_clip(uv.y);
					UMImagePtr texture = material->texture_list()[0];
					const int x = static_cast<int>(texture->width() * uv.x);
					const int y = static_cast<int>(texture->height() * uv.y);
					const int pixel = y * texture->width() + x;
					const UMVec4d& pixel_color = texture->list()[pixel];
					parameter.uv = uv;
					parameter.color.x *= pixel_color.x;
					parameter.color.y *= pixel_color.y;
					parameter.color.z *= pixel_color.z;
				}
			}
			return true;
		}
	}
#ifdef WITH_ALEMBIC
	if (umabc::UMAbcMeshPtr me = abc_mesh())
	{
		// 3 points
		const Imath::V3f& v0 = me->vertex()->get()[vertex_index_.x];
		const Imath::V3f& v1 = me->vertex()->get()[vertex_index_.y];
		const Imath::V3f& v2 = me->vertex()->get()[vertex_index_.z];

		if (intersects(
			UMVec3d(v0.x, v0.y, v0.z), 
			UMVec3d(v1.x, v1.y, v1.z), 
			UMVec3d(v2.x, v2.y, v2.z), ray, parameter))
		{
			const Imath::V3f& in0 = me->normals()[vertex_index_.x];
			const Imath::V3f& in1 = me->normals()[vertex_index_.y];
			const Imath::V3f& in2 = me->normals()[vertex_index_.z];
			const UMVec3d n0(in0.x, in0.y, in0.z);
			const UMVec3d n1(in1.x, in1.y, in1.z);
			const UMVec3d n2(in2.x, in2.y, in2.z);
			parameter.normal = (n0 * parameter.uvw.x + n1 * parameter.uvw.y + n2 * parameter.uvw.z).normalized();

			if (UMMaterialPtr material = me->material_from_face_index(face_index_))
			{
				parameter.material = material;

				const UMVec4d& diffuse = material->diffuse();
				parameter.color.x = diffuse.x;
				parameter.color.y = diffuse.y;
				parameter.color.z = diffuse.z;
				parameter.emissive = material->emissive().xyz() * material->emissive_factor();
				if (me->uv().getVals()->get() && !material->texture_list().empty()) {
					// uv
					const int base = face_index_ * 3;
					const Imath::V2f& uv0 = me->uv().getVals()->get()[base + 0];
					const Imath::V2f& uv1 = me->uv().getVals()->get()[base + 1];
					const Imath::V2f& uv2 = me->uv().getVals()->get()[base + 2];
					UMVec2d uv = UMVec2d(
						UMVec2d(uv0.x, uv0.y) * parameter.uvw.x +
						UMVec2d(uv1.x, uv1.y) * parameter.uvw.y +
						UMVec2d(uv2.x, uv2.y) * parameter.uvw.z);
					uv.x = umbase::um_clip(uv.x);
					uv.y = umbase::um_clip(uv.y);
					const UMImagePtr texture = material->texture_list()[0];
					const int x = static_cast<int>(texture->width() * uv.x);
					const int y = static_cast<int>(texture->height() * uv.y);
					const int pixel = y * texture->width() + x;
					const UMVec4d& pixel_color = texture->list()[pixel];
					parameter.uv = uv;
					parameter.color.x *= pixel_color.x;
					parameter.color.y *= pixel_color.y;
					parameter.color.z *= pixel_color.z;
				}
			}
			return true;
		}
	}
#endif
	return false;
}

/**
 * ray triangle intersection
 */
bool UMTriangle::intersects(const UMRay& ray) const
{
	if (UMMeshPtr me = mesh())
	{
		// 3 points
		const UMVec3d& v0 = me->vertex_list()[vertex_index_.x];
		const UMVec3d& v1 = me->vertex_list()[vertex_index_.y];
		const UMVec3d& v2 = me->vertex_list()[vertex_index_.z];
		return intersects(v0, v1, v2, ray);
	}
#ifdef WITH_ALEMBIC
	else if (umabc::UMAbcMeshPtr me = abc_mesh())
	{
		// 3 points
		const Imath::V3f& v0 = me->vertex()->get()[vertex_index_.x];
		const Imath::V3f& v1 = me->vertex()->get()[vertex_index_.y];
		const Imath::V3f& v2 = me->vertex()->get()[vertex_index_.z];
		return intersects(
			UMVec3d(v0.x, v0.y, v0.z), 
			UMVec3d(v1.x, v1.y, v1.z), 
			UMVec3d(v2.x, v2.y, v2.z), 
			ray);
	}
#endif
	return false;
}

/**
 * update box
 */
void UMTriangle::update_box()
{
	if (UMMeshPtr me = mesh())
	{
		box_.init();
		const UMVec3d& v0 = me->vertex_list()[vertex_index_.x];
		const UMVec3d& v1 = me->vertex_list()[vertex_index_.y];
		const UMVec3d& v2 = me->vertex_list()[vertex_index_.z];
		box_.extend(v0);
		box_.extend(v1);
		box_.extend(v2);
	}
#ifdef WITH_ALEMBIC
	else if (umabc::UMAbcMeshPtr me = abc_mesh())
	{
		box_.init();
		const Imath::V3f& v0 = me->vertex()->get()[vertex_index_.x];
		const Imath::V3f& v1 = me->vertex()->get()[vertex_index_.y];
		const Imath::V3f& v2 = me->vertex()->get()[vertex_index_.z];
		box_.extend(UMVec3d(v0.x, v0.y, v0.z));
		box_.extend(UMVec3d(v1.x, v1.y, v1.z));
		box_.extend(UMVec3d(v2.x, v2.y, v2.z));
	}
#endif
}

} // umrt
