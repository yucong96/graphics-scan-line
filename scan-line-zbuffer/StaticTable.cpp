#include "StaticTable.h"

using namespace std;
using namespace Eigen;

bool larger(const Object& obj, const size_t v1_idx, const size_t v2_idx, const string& mode) {
	if (mode == "y") {
		return obj.v_out_mat(1, v1_idx) >= obj.v_out_mat(1, v2_idx);
	}
	else {
		assert(0 && "mode do not exist");
	}
}
void sort_vertex_on_y(const Object &obj, const size_t v1_idx, const size_t v2_idx, const size_t v3_idx, size_t& v_max_idx, size_t& v_mid_idx, size_t& v_min_idx) {
	if (larger(obj, v1_idx, v2_idx, "y") && larger(obj, v1_idx, v3_idx, "y")) {
		v_max_idx = v1_idx;
		if (larger(obj, v2_idx, v3_idx, "y")) {
			v_mid_idx = v2_idx;
			v_min_idx = v3_idx;
		}
		else {
			v_mid_idx = v3_idx;
			v_min_idx = v2_idx;
		}
	}
	else if (larger(obj, v2_idx, v3_idx, "y")) {
		v_max_idx = v2_idx;
		if (larger(obj, v1_idx, v3_idx, "y")) {
			v_mid_idx = v1_idx;
			v_min_idx = v3_idx;
		}
		else {
			v_mid_idx = v3_idx;
			v_min_idx = v1_idx;
		}
	}
	else {
		v_max_idx = v3_idx;
		if (larger(obj, v1_idx, v2_idx, "y")) {
			v_mid_idx = v1_idx;
			v_min_idx = v2_idx;
		}
		else {
			v_mid_idx = v2_idx;
			v_min_idx = v1_idx;
		}
	}
}

void StaticTable::make_edge_node(const PolyNode* poly, const Vector3f& v_top, const Vector3f& v_bottom, EdgeNode* enode) {
	enode->x = v_top[0];
	enode->y = transfer_to_pixel_pos(v_top, "y"); {
		assert(enode->y <= height);
		if (enode->y == height) enode->y = height - 1;
	}
	enode->cy = enode->y - transfer_to_pixel_pos(v_bottom, "y");
	if (enode->cy != 0) {
		enode->dx = (v_bottom[0] - v_top[0]) / enode->cy;
	}
	else {
		enode->dx = 0;
	}
	enode->z = v_top[2];
	//edge_table[enode->y].push_back(enode);
}

Vector3f StaticTable::compute_cross_vertex(const Vector3f& v1, const Vector3f& v2, float y_line) {
	return (v1[1] - y_line) / (v1[1] - v2[1]) * (v2 - v1) + v1;
}

void StaticTable::add_obj(Object &obj) {
	for (size_t i = 0; i < obj.f_num; i++) {
		size_t v1_idx = obj.f_mat(0, i);
		size_t v2_idx = obj.f_mat(1, i);
		size_t v3_idx = obj.f_mat(2, i);
		size_t v_max_idx, v_mid_idx, v_min_idx;
		sort_vertex_on_y(obj, v1_idx, v2_idx, v3_idx, v_max_idx, v_mid_idx, v_min_idx);
		Vector3f v_max = obj.v_out_mat.block(0, v_max_idx, 3, 1);
		Vector3f v_mid = obj.v_out_mat.block(0, v_mid_idx, 3, 1);
		Vector3f v_min = obj.v_out_mat.block(0, v_min_idx, 3, 1);
		int v_max_y = transfer_to_pixel_pos(v_max, "y");
		int v_mid_y = transfer_to_pixel_pos(v_mid, "y");
		int v_min_y = transfer_to_pixel_pos(v_min, "y");

		if (v_min_y > height - 1) {
			continue;
		}
		if (v_max_y < 0) {
			continue;
		}

		PolyNode* pnode = new PolyNode();
		pnode->coord2func(v_max, v_mid, v_min); {
			if (pnode->c < 1e-6 && pnode->c > -1e-6) continue; // remove the poly pependicular to xOy
		}
		if (v_max_y < height) {
			pnode->cy = v_max_y - v_min_y;
		}
		else {
			pnode->cy = height - v_min_y;
		}
		// TODO: pnode->color
		pnode->color.r = rand() % 255;
		pnode->color.g = rand() % 255;
		pnode->color.b = rand() % 255;
#ifdef RAND_FACE_COLOR
		pnode->color.r = obj.f_color(0, i);
		pnode->color.g = obj.f_color(1, i);
		pnode->color.b = obj.f_color(2, i);
#endif

		EdgeNode* enode1 = new EdgeNode();
		EdgeNode* enode2 = new EdgeNode();
		EdgeNode* enode3 = new EdgeNode();
		size_t poly_y;
		if (v_max_y < height) { // triangle is below the top_bound
			pnode->type = "3pt";
			poly_y = v_max_y;

			make_edge_node(pnode, v_max, v_mid, enode1);
			pnode->edge1_ptr = enode1;

			make_edge_node(pnode, v_max, v_min, enode2);
			pnode->edge2_ptr = enode2;

			make_edge_node(pnode, v_mid, v_min, enode3);
			if (enode3->y >= 0) {
				pnode->edge3_ptr = enode3;
			}
			else {
				pnode->edge3_ptr = nullptr;
			}
		}
		else if (v_mid_y < height) { // v_max is beyond the top_bound
			pnode->type = "2pt";
			poly_y = height - 1;
			Vector3f v_cross1 = compute_cross_vertex(v_max, v_mid, top_bound);
			Vector3f v_cross2 = compute_cross_vertex(v_max, v_min, top_bound);

			make_edge_node(pnode, v_cross1, v_mid, enode1);
			pnode->edge1_ptr = enode1;

			make_edge_node(pnode, v_cross2, v_min, enode2);
			pnode->edge2_ptr = enode2;

			make_edge_node(pnode, v_mid, v_min, enode3);
			if (enode3->y >= 0) {
				pnode->edge3_ptr = enode3;
			}
			else {
				pnode->edge3_ptr = nullptr;
			}
		}
		else { // only v_min is below the top_bound
			pnode->type = "1pt";
			poly_y = height - 1;
			Vector3f v_cross1 = compute_cross_vertex(v_max, v_min, top_bound);
			Vector3f v_cross2 = compute_cross_vertex(v_mid, v_min, top_bound);

			make_edge_node(pnode, v_cross1, v_min, enode1);
			pnode->edge1_ptr = enode1;

			make_edge_node(pnode, v_cross2, v_min, enode2);
			pnode->edge2_ptr = enode2;

			delete pnode->edge3_ptr;
			pnode->edge3_ptr = nullptr;
			enode3 = nullptr;
		}
		poly_table[poly_y].push_back(pnode);

		if (pnode->edge1_ptr->cy == 0 && pnode->edge2_ptr->cy == 0) {
			/*edge_table[enode1->y].erase(enode1, "delete");
			edge_table[enode2->y].erase(enode2, "delete");
			if (enode3 != nullptr) {
			edge_table[enode3->y].erase(enode3, "delete");
			}*/
			poly_table[poly_y].erase(pnode, "delete");
		}
	}
};