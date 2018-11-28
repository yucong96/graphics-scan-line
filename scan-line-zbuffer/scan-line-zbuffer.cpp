#include "scan-line-zbuffer.h"

using namespace std;
using namespace Eigen;





void PolyNode::coord2func(const Object &obj, const size_t v1_idx, const size_t v2_idx, const size_t v3_idx) {
	float x1 = obj.v_out_mat(0, v1_idx), y1 = obj.v_out_mat(1, v1_idx), z1 = obj.v_out_mat(2, v1_idx);
	Vector3f edge1 = obj.v_out_mat.block(0, v1_idx, 3, 1) - obj.v_out_mat.block(0, v2_idx, 3, 1);
	Vector3f edge2 = obj.v_out_mat.block(0, v1_idx, 3, 1) - obj.v_out_mat.block(0, v3_idx, 3, 1);
	Vector3f norm = edge1.cross(edge2);
	a = norm[0];
	b = norm[1];
	c = norm[2];
	d = -(a*x1 + b*y1 + c*z1);
	if (d > 1e-10 || d < -1e-10) {
		a /= d;
		b /= d;
		c /= d;
		d = 1;
	}
	else if (c > 1e-10 || c < -1e-10) {
		a /= c;
		b /= c;
		c = 1;
		d = 0;
	}
	else if (b > 1e-10 || b < -1e-10) {
		a /= b;
		b = 1;
		c = d = 0;
	}
	else {
		a = 1;
		b = c = d = 0;
	}
}