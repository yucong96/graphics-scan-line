#include "BaseTable.h"

using namespace std;
using namespace Eigen;

int BaseTable::transfer_to_pixel_pos(const GLfloat val, const string& mode) {
	if (mode == "x") {
		return (int)((val - left_bound) / d_width);
	}
	if (mode == "y") {
		return (int)((val - bottom_bound) / d_height);
	}
}

int BaseTable::transfer_to_pixel_pos(const Vector3f& point, const string& mode) {
	if (mode == "x") {
		return (int)((point[0] - left_bound) / d_width);
	}
	if (mode == "y") {
		return (int)((point[1] - bottom_bound) / d_height);
	}
}

double BaseTable::transfer_to_real_pos(const int val, const string& mode) {
	if (mode == "x") {
		return val*d_width + left_bound;
	}
	if (mode == "y") {
		return val * d_height + bottom_bound;
	}
}

