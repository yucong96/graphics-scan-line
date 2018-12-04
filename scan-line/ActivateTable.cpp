#include "ActivateTable.h"

using namespace std;
using namespace Eigen;

void ActivateTable::set_act_edge_left_and_right(const EdgeNode* left_node, const EdgeNode* right_node, ActivateEdgeNode* act_edge) {
	act_edge->x_l = left_node->x, act_edge->x_r = right_node->x;
	act_edge->cy_l = left_node->cy, act_edge->cy_r = right_node->cy;
	act_edge->dx_l = left_node->dx, act_edge->dx_r = right_node->dx;
	act_edge->z_l = left_node->z;
}

void ActivateTable::set_act_edge_from_two_edge(const EdgeNode* edge1, const EdgeNode* edge2, ActivateEdgeNode* act_edge) {
	if (edge1->x < edge2->x) { // edge1 is on the left
		set_act_edge_left_and_right(edge1, edge2, act_edge);
	}
	else if (edge1->x > edge2->x) { // edge2 is on the left
		set_act_edge_left_and_right(edge2, edge1, act_edge);
	}
	else { // edge1 and edge2 have same top vertex
		if (edge1->dx < edge2->dx) { // edge1 is on the left
			set_act_edge_left_and_right(edge1, edge2, act_edge);
		}
		else { // edge2 is on the left
			set_act_edge_left_and_right(edge2, edge1, act_edge);
		}
	}
}

void ActivateTable::add_act_edge_from_poly(StaticTable& static_table, PolyNode* poly, ActivatePolyNode* act_poly, int h) {
	vector<EdgeNode*> &edges = poly->edge_ptrs;
	vector<EdgeNode*> selected_edges;
	for (int edge_idx = 0; edge_idx < edges.size(); edge_idx++) {
		if (edges[edge_idx]->y == h && edges[edge_idx]->cy != 0) {
			selected_edges.push_back(edges[edge_idx]);
		}
	}
	assert(selected_edges.size() == 2);
	ActivateEdgeNode* act_edge = new ActivateEdgeNode();
	act_edge->dz_x = -poly->a / poly->c * d_width;
	act_edge->dz_y = poly->b / poly->c * d_height;
	act_edge->poly_ptr = act_poly;
	set_act_edge_from_two_edge(selected_edges[0], selected_edges[1], act_edge);
	act_edge_list.push_back(act_edge);
}

void ActivateTable::move_to_next_scan_line(ActivateEdgeNode* edge, int h) {
	edge->cy_l--;
	edge->cy_r--;
	edge->poly_ptr->cy_remain--;
	if (edge->poly_ptr->cy_remain == 0 || h == 0) {
		act_poly_list.erase(edge->poly_ptr, "delete");
		act_edge_list.erase(edge, "delete");
		return;
	}

	if (edge->cy_l == 0) {
		EdgeNode* next_edge = nullptr;
		vector<EdgeNode*>& candidate_edges = edge->poly_ptr->static_poly_ptr->edge_ptrs;
		for (int i = 0; i < candidate_edges.size(); i++) {
			if (candidate_edges[i]->y == h - 1 && equal(candidate_edges[i]->x, edge->x_l + edge->dx_l)) {
				next_edge = candidate_edges[i];
				break;
			}
		}
		assert(next_edge != nullptr && "ActivateTable::move_to_next_scan_line: edge3 error");
		edge->x_l = next_edge->x;
		edge->cy_l = next_edge->cy;
		edge->dx_l = next_edge->dx;
		edge->z_l = next_edge->z;
	}
	else {
		edge->x_l += edge->dx_l;
		edge->z_l += edge->dz_x * (edge->dx_l / d_width) + edge->dz_y;
	}

	if (edge->cy_r == 0) {
		EdgeNode* next_edge = nullptr;
		vector<EdgeNode*>& candidate_edges = edge->poly_ptr->static_poly_ptr->edge_ptrs;
		for (int i = 0; i < candidate_edges.size(); i++) {
			if (candidate_edges[i]->y == h - 1 && equal(candidate_edges[i]->x, edge->x_r + edge->dx_r)) {
				next_edge = candidate_edges[i];
				break;
			}
		}
		assert(next_edge != nullptr && "ActivateTable::move_to_next_scan_line: edge3 error");
		edge->x_r = next_edge->x;
		edge->cy_r = next_edge->cy;
		edge->dx_r = next_edge->dx;
	}
	else {
		edge->x_r += edge->dx_r;
	}
}
