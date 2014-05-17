/**
 * @file UMCamera.cpp
 * a Camera
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#include "UMCamera.h"

#include "UMMathTypes.h"
#include "UMMath.h"
#include "UMVector.h"
#include "UMMatrix.h"
#include "UMAny.h"

namespace umdraw
{
	
UMCamera::~UMCamera()
{
}

void UMCamera::init(bool is_ortho, int width, int height)
{
	view_matrix_.identity();
	projection_matrix_.identity();
	view_projection_matrix_.identity();
	generate_ray_x_scale_ = UMVec3d(0);
	generate_ray_y_scale_ = UMVec3d(0);
	generate_ray_adder_ = UMVec3d(0);
	
	is_ortho_ = is_ortho;
	aspect_ = (static_cast<double>(width) / static_cast<double>(height));
	fov_y_ = 45.0;
	near_ = 1.0;
	far_ = 1000.0;
	position_ = UMVec3d(0,  15, 50);
	target_ = UMVec3d(0, 15, 0);
	up_ = UMVec3d(0, 1, 0);
	theta_ = 0.0;
	phi_ = 0.0;
	inverted_width_ = 1.0 / static_cast<double>(width);
	inverted_height_ = 1.0 / static_cast<double>(height);
	
	if (is_ortho)
	{
		umbase::um_matrix_ortho_rh(view_matrix_, 
			static_cast<double>(width), 
			static_cast<double>(height), 
			near_, 
			far_);
	}
	else
	{
		umbase::um_matrix_look_at_rh(view_matrix_, position_, target_, up_);
		//um_matrix_perspective_rh(projection_matrix_, (double)width, (double)height, near_, far_);
		umbase::um_matrix_perspective_fov_rh(projection_matrix_, umbase::um_to_radian(fov_y_), aspect_, near_, far_);
	}

	update();
}

/**
 * update
 */
void UMCamera::update()
{
	view_projection_matrix_ = view_matrix() * projection_matrix();
}

/**
 * rotate camera
 */
void UMCamera::rotate(double mx, double my)
{
	if (is_ortho_) return;
	if (fabs(mx) <= 2 && fabs(my) <= 2) return;

	theta_ -= mx;
	phi_ -= my;
	if ( phi_ >= 90.0f ) phi_ = 89.0f;
	if ( phi_ <= -90.0f ) phi_ = -89.0f;
	
	double radius = (target_ - position_).length();
	double t = umbase::um_to_radian(theta_);
	double p = umbase::um_to_radian(phi_);

	position_ = target_;
	position_.x += -radius * sin(t) * cos(p);
	position_.y += radius * sin(p);
	position_.z += radius * cos(t) * cos(p);

	//view_matrix_.identity();
	um_matrix_look_at_rh(view_matrix_, position_, target_, up_);
}

/**
 * zoom camera
 */
void UMCamera::zoom(double mx, double my)
{
	if (is_ortho_) return;
	if (fabs(mx) <= 2 && fabs(my) <= 2) return;

	fov_y_ -= my * 0.2;

	if (fov_y_ > 175) {
		fov_y_ = 175;
	}
	if (fov_y_ < 5) {
		fov_y_ = 5;
	}

	umbase::um_matrix_perspective_fov_rh(projection_matrix_, umbase::um_to_radian(fov_y_), aspect_, near_, far_);
}


void UMCamera::update_from_node()
{
	UMMat44d transform = global_transform();

	UMVec3d x (transform.m[0][0], transform.m[0][1], transform.m[0][2]);
	UMVec3d y (transform.m[1][0], transform.m[1][1], transform.m[1][2]);
	UMVec3d z (transform.m[2][0], transform.m[2][1], transform.m[2][2]);
	position_.x = transform.m[3][0];
	position_.y = transform.m[3][1];
	position_.z = transform.m[3][2];
	
	up_ = -x.cross(z);
	target_ = position_ - z * 100;

	um_matrix_look_at_rh(view_matrix_, position_, target_, up_);
}

/**
 * dolly camera
 * @param [in] mx x-axis mouse move
 * @param [in] my y-axis mouse move
 */
void UMCamera::dolly(double mx, double my)
{
	if (is_ortho_) return;
	if (fabs(mx) <= 2 && fabs(my) <= 2) return;
	
	double dolly_value = -my * 0.2;
		
	double radius = (target_ - position_).length() + dolly_value;
	if (radius < 1) return;
	double t = umbase::um_to_radian(theta_);
	double p = umbase::um_to_radian(phi_);
	
	position_ = target_;
	position_.x += -radius * sin(t) * cos(p);
	position_.y += radius * sin(p);
	position_.z += radius * cos(t) * cos(p);
	um_matrix_look_at_rh(view_matrix_, position_, target_, up_);
}

/**
 * move camera
 */
void UMCamera::pan(double mx, double my)
{
	if (is_ortho_) return;
	if (fabs(mx) <= 2 && fabs(my) <= 2) return;
	
	double pan_value = 0.2;

	UMVec3d mvertial = (up_).normalized() * -my * pan_value;
	UMVec3d mhorizon = (target_ - position_).cross(up_).normalized() * mx * pan_value;
	position_ += mvertial + mhorizon;
	target_ += mvertial + mhorizon;
	um_matrix_look_at_rh(view_matrix_, position_, target_, up_);
}

} // umdraw
