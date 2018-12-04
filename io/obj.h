#pragma once

#include "../head.h"
#include <set>

// Support format:
// v x, y, z, [w]
// ...
// [vn vnx, vny, vnz]
// [...]
// [vt vtx, vty, vtz]
// [...]
// # only for triangles
// f v1[/vt1/vn1] v2[/vt2/vn2] v3[/vt3/vn3]
// ...

class Object {
private:
	void obj_count(std::ifstream &input);
	void obj_fill_mat(std::ifstream &input);

public:
	size_t channel;
	size_t v_num, vn_num, vt_num, f_num;

	Eigen::MatrixXf v_mat;
	Eigen::MatrixXf v_out_mat;
	Eigen::MatrixXf vn_mat;
	Eigen::MatrixXf vt_mat;
	std::vector<std::vector<int>> f_mat;
	std::vector<std::vector<int>> fn_mat;
#ifdef RAND_FACE_COLOR
	Eigen::MatrixXi f_color;
#endif

	Object() {
		channel = 3;
		v_num = 0;
		vn_num = 0;
		vt_num = 0;
		f_num = 0;
	}
	void obj2tri(const std::string &obj_file);
	Eigen::Vector3f bound_box();
	void apply_mat(const Eigen::Matrix4f &mat, const std::string &mode);
};
