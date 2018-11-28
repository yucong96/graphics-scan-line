#include "obj.h"

#include <fstream>
#include <sstream>

using namespace std;
using namespace Eigen;

#define REPORT_LOCATION __FILE__ << " " << __LINE__ << ": "

inline bool is_line_invalid(const string &line) {
	return (line.empty() || line[0] == 13 || line[0] == '#');
}

inline void report_err(const string &info) {
	cerr << "Error in " << REPORT_LOCATION << info << endl;
	assert(0);
}

void Object::obj_count(ifstream &input) {
	string line, word;

	while (getline(input, line)) {
		if (is_line_invalid(line)) continue;
		istringstream instream(line);
		instream >> word;
		if (word == "v" || word == "V") {
			v_num++;
		}
		else if (word == "f" || word == "F") {
			f_num++;
		}
		else if (word == "vn" || word == "VN") {
			vn_num++;
		}
		else if (word == "vt" || word == "VT") {
			vt_num++;
		}
		word.clear();
	}
	input.clear();
	input.seekg(0, ios::beg);

	if (v_num == 0 || f_num == 0) {
		report_err("no vertex or face data found");
	}
}

void Object::obj_fill_mat(ifstream &input) {
	string line, word;

	size_t v_idx = 0, f_idx = 0, vn_idx = 0, vt_idx = 0;
	while (getline(input, line)) {
		if (is_line_invalid(line)) continue;
		istringstream instream(line);
		instream >> word;
		if (word == "v" || word == "V") {
			instream >> v_mat(0, v_idx) >> v_mat(1, v_idx) >> v_mat(2, v_idx);
			double w = 1.0;
			if (instream >> w) {
				if (w < 1e-6) {
					report_err("error occured when read vertex coordinates");
				}
				for (size_t i = 0; i < 3; i++) {
					v_mat(i, v_idx) /= w;
				}	
			}
			v_mat(3, v_idx) = 1;
			v_idx++;
		}
		else if (word == "vn" || word == "VN") {
			instream >> vn_mat(0, vn_idx) >> vn_mat(1, vn_idx) >> vn_mat(2, vn_idx);
			vn_idx++;
		}
		else if (word == "vt" || word == "VT") {
			instream >> vt_mat(0, vt_idx) >> vt_mat(1, vt_idx) >> vt_mat(2, vn_idx);
			vt_idx++;
		}
		else if (word == "f" || word == "F") {
			//only triangle mesh supported
			string pair[3], test;
			instream >> pair[0] >> pair[1] >> pair[2] >> test;
			if (!test.empty() || !instream.eof()) {
				report_err("only triangle mesh supported");
			}
			//get vertex id in the this triangle face
			for (size_t i = 0; i < 3; ++i) {
				string vertex_str = pair[i].substr(0, pair[i].find('/'));
				long unsigned int v_index;
				sscanf_s(vertex_str.c_str(), "%lu", &v_index);
				v_index--; // for .obj, index start from 1
				f_mat(i, f_idx) = v_index;
				if (v_index >= v_num) {
					report_err("vertex index in face exceed limit");
				}

				string vn_str = pair[i].substr(pair[i].rfind('/') + 1, pair[i].size());
				if (vn_str.size() == 0) continue;
				long unsigned int vn_index;
				sscanf_s(vn_str.c_str(), "%lu", &vn_index);
				vn_index--;
				fn_mat(i, f_idx) = vn_index;
				if (vn_index >= vn_num) {
					report_err("vertex normal index in face exceed limit");
				}
			}
			f_idx++;
		}
		word.clear();
	}
}

void Object::obj2tri(const string &obj_file) {
	ifstream input(obj_file);

	if (!input) {
		assert(0 && "null ifstream");
	}

	obj_count(input);

	v_mat.resize(4, v_num);
	v_out_mat.resize(4, v_num);
	f_mat.resize(3, f_num);
	vn_mat.resize(3, vn_num);
	vt_mat.resize(3, vt_num);
	fn_mat.resize(3, f_num);

	obj_fill_mat(input);

#ifdef RAND_FACE_COLOR
	Vector3f v_mat_max = v_mat.block(0, 0, 3, v_num).cwiseAbs().rowwise().maxCoeff();
	f_color.resize(3, f_num);
	for (size_t i = 0; i < f_num; i++) {
		size_t v1_idx = f_mat(0, i);
		size_t v2_idx = f_mat(1, i);
		size_t v3_idx = f_mat(2, i);
		Vector3f f_avg = (v_mat.block(0, v1_idx, 3, 1) + v_mat.block(0, v2_idx, 3, 1) + v_mat.block(0, v3_idx, 3, 1)) / 3;
		for (size_t j = 0; j < 3; j++) {
			f_color(j, i) = (int)((f_avg[j] / v_mat_max[j] + 0.5) * 255);
			f_color(1, i) = 0;
		}
	}
#endif
}

Vector3f Object::bound_box() {
	Vector3f bound = v_mat.block(0, 0, 3, v_num).cwiseAbs().rowwise().maxCoeff();
	return bound;
}

void Object::apply_mat(const Matrix4f &mat, const string &mode) {
	if (mode == "new") {
		v_out_mat = mat * v_mat;
	}
	else if (mode == "append") {
		v_out_mat = mat * v_out_mat;
	}
	else {
		assert(0 && "undefined mode");
	}
}
