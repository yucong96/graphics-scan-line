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

void StaticTable::make_edge_node(PolyNode* poly, const Vector3f& v_top, const Vector3f& v_bottom, EdgeNode* enode) {
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
	enode->poly_ptr = poly;
}

Vector3f StaticTable::compute_cross_vertex(const Vector3f& v1, const Vector3f& v2, float y_line) {
	return (v1[1] - y_line) / (v1[1] - v2[1]) * (v2 - v1) + v1;
}

void StaticTable::add_obj(Object &obj) {
	for (size_t i = 0; i < obj.f_num; i++) {
		int v_num = obj.f_mat[i].size(); {
			if (v_num < 3) continue;
		}
		vector<int> &v_idx_vec = obj.f_mat[i];
		vector<Vector3f> v_vec(v_num); {
			for (int i = 0; i < v_num; i++) {
				v_vec[i] = obj.v_out_mat.block(0, v_idx_vec[i], 3, 1);
			}
		}
		vector<int> v_y_vec(v_num); {
			for (int i = 0; i < v_num; i++) {
				v_y_vec[i] = transfer_to_pixel_pos(v_vec[i], "y");
			}
		}

		vector<int> v_mapping_sorted(v_idx_vec); {
			for (int i = 0; i<v_num; i++) {
				v_mapping_sorted[i] = i;
			}
			sort(v_mapping_sorted.begin(), v_mapping_sorted.end(), [&](int v1_idx, int v2_idx) -> bool {
				return obj.v_out_mat(1, v_idx_vec[v1_idx]) > obj.v_out_mat(1, v_idx_vec[v2_idx]);
			});
		}
		vector<int> &map = v_mapping_sorted;
		// val: 1 4 5 2 3 -> 5 4 3 2 1
		// idx: 0 1 2 3 4 -> 2 1 4 3 0
		// idx-of-idx:       0 1 2 3 4

		if (v_y_vec[map[v_num - 1]] > height - 1) continue;
		if (v_y_vec[map[0]] < 0) continue;

		PolyNode* pnode = new PolyNode();
		pnode->coord2func(v_vec[0], v_vec[1], v_vec[2]); {
			if (pnode->c < 1e-6 && pnode->c > -1e-6) continue; // remove the poly pependicular to xOy
		}
		if (v_y_vec[map[0]] < height) {
			pnode->y = v_y_vec[map[0]];
			pnode->cy = v_y_vec[map[0]] - v_y_vec[map[v_num-1]];
		}
		else {
			pnode->y = height - 1;
			pnode->cy = height - 1 - v_y_vec[map[v_num - 1]];
		}
		// TODO: pnode->color
#ifdef RAND_FACE_COLOR
		pnode->color.r = obj.f_color(0, i);
		pnode->color.g = obj.f_color(1, i);
		pnode->color.b = obj.f_color(2, i);
#endif
		if (pnode->cy <= 0) {
			delete pnode;
			continue;
		}
		poly_table[pnode->y].push_back(pnode);
		
		for (int i = 0; i < v_num; i++) {
			EdgeNode* enode = new EdgeNode();
			int v_max_idx, v_min_idx;
			if (obj.v_out_mat(1, v_idx_vec[i]) > obj.v_out_mat(1, v_idx_vec[(i + 1) % v_num])) {
				v_max_idx = i;
				v_min_idx = (i + 1) % v_num;
			}
			else {
				v_max_idx = (i + 1) % v_num;
				v_min_idx = i;
			}

			if (v_y_vec[v_min_idx] >= height) {
				continue;
			}
			if (v_y_vec[v_max_idx] < height) {
				make_edge_node(pnode, v_vec[v_max_idx], v_vec[v_min_idx], enode);
			}
			else {
				Vector3f v_cross = compute_cross_vertex(v_vec[v_max_idx], v_vec[v_min_idx], top_bound);
				make_edge_node(pnode, v_cross, v_vec[v_min_idx], enode);
			}

			if (enode->cy == 0 || enode->y < 0) {
				delete enode;
			}
			else {
				pnode->edge_ptrs.push_back(enode);
				edge_table[enode->y].push_back(enode);
			}
		}

		//EdgeNode* enode1 = new EdgeNode();
		//EdgeNode* enode2 = new EdgeNode();
		//EdgeNode* enode3 = new EdgeNode();
		//size_t poly_y;
		//if (v_max_y < height) { // triangle is below the top_bound
		//	pnode->type = "3pt";
		//	poly_y = v_max_y;

		//	make_edge_node(pnode, v_max, v_mid, enode1);
		//	pnode->edge1_ptr = enode1;

		//	make_edge_node(pnode, v_max, v_min, enode2);
		//	pnode->edge2_ptr = enode2;

		//	make_edge_node(pnode, v_mid, v_min, enode3);
		//	if (enode3->y >= 0) {
		//		pnode->edge3_ptr = enode3;
		//	}
		//	else {
		//		pnode->edge3_ptr = nullptr;
		//	}
		//}
		//else if (v_mid_y < height) { // v_max is beyond the top_bound
		//	pnode->type = "2pt";
		//	poly_y = height - 1;
		//	Vector3f v_cross1 = compute_cross_vertex(v_max, v_mid, top_bound);
		//	Vector3f v_cross2 = compute_cross_vertex(v_max, v_min, top_bound);

		//	make_edge_node(pnode, v_cross1, v_mid, enode1);
		//	pnode->edge1_ptr = enode1;

		//	make_edge_node(pnode, v_cross2, v_min, enode2);
		//	pnode->edge2_ptr = enode2;

		//	make_edge_node(pnode, v_mid, v_min, enode3);
		//	if (enode3->y >= 0) {
		//		pnode->edge3_ptr = enode3;
		//	}
		//	else {
		//		pnode->edge3_ptr = nullptr;
		//	}
		//}
		//else { // only v_min is below the top_bound
		//	pnode->type = "1pt";
		//	poly_y = height - 1;
		//	Vector3f v_cross1 = compute_cross_vertex(v_max, v_min, top_bound);
		//	Vector3f v_cross2 = compute_cross_vertex(v_mid, v_min, top_bound);

		//	make_edge_node(pnode, v_cross1, v_min, enode1);
		//	pnode->edge1_ptr = enode1;

		//	make_edge_node(pnode, v_cross2, v_min, enode2);
		//	pnode->edge2_ptr = enode2;

		//	delete pnode->edge3_ptr;
		//	pnode->edge3_ptr = nullptr;
		//	enode3 = nullptr;
		//}
		//poly_table[poly_y].push_back(pnode);
/*
		if (pnode->edge1_ptr->cy == 0 && pnode->edge2_ptr->cy == 0) {
			poly_table[poly_y].erase(pnode, "delete");
		}*/
	}
};