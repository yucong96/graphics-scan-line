#pragma once

#include "alg_head.h"
#include "../opengl/View.h"

class BaseTable {
public:
	GLfloat left_bound, right_bound, bottom_bound, top_bound, near_bound, far_bound;
	int height, width;
	GLfloat d_height, d_width;

	int transfer_to_pixel_pos(const Eigen::Vector3f& point, const std::string& mode);
	int transfer_to_pixel_pos(const GLfloat val, const std::string& mode);
	double transfer_to_real_pos(const int val, const std::string& mode);
	BaseTable() {}
	BaseTable(GLfloat _left, GLfloat _right, GLfloat _bottom, GLfloat _top, GLfloat _near, GLfloat _far, int _width, int _height) {
		left_bound = _left;
		right_bound = _right;
		bottom_bound = _bottom;
		top_bound = _top;
		near_bound = _near;
		far_bound = _far;
		width = _width;
		height = _height;
		d_width = (_right - _left) / width;
		d_height = (_top - _bottom) / height;
	}
	BaseTable(const BaseTable& table) {
		left_bound = table.left_bound;
		right_bound = table.right_bound;
		bottom_bound = table.bottom_bound;
		top_bound = table.top_bound;
		near_bound = table.near_bound;
		far_bound = table.far_bound;
		width = table.width;
		height = table.height;
		d_width = (right_bound - left_bound) / width;
		d_height = (top_bound - bottom_bound) / height;
	}
};


