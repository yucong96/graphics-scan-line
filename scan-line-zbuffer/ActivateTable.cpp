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

void ActivateTable::traverse(StaticTable& static_table, const View& view) {
	for (int i = height - 1; i >= 0; i--) {
		reset_buffers();
		for (PolyNode* poly = static_table.poly_table[i].head(); poly != nullptr; poly = poly->next) {
			// add new poly
			ActivatePolyNode* act_poly = new ActivatePolyNode(poly);
			act_poly_list.push_back(act_poly);
			// add edge from new poly
			ActivateEdgeNode* act_edge = new ActivateEdgeNode();
			act_edge->dz_x = -poly->a / poly->c * d_width;
			act_edge->dz_y = poly->b / poly->c * d_height;
			act_edge->poly_ptr = act_poly;
			if (poly->type == "3pt") {
				if (poly->edge1_ptr->cy != 0) { // add edge1 & edge2 into act_edge_list
					set_act_edge_from_two_edge(poly->edge1_ptr, poly->edge2_ptr, act_edge);
				}
				else { // edge1 is horizontal, so add edge2 & edge3 into act_edge_list
					set_act_edge_from_two_edge(poly->edge2_ptr, poly->edge3_ptr, act_edge);
				}

			}
			else if (poly->type == "2pt") {
				if (poly->edge1_ptr->cy != 0) { // add edge1 & edge2 into act_edge_list
					set_act_edge_from_two_edge(poly->edge1_ptr, poly->edge2_ptr, act_edge);
				}
				else { // edge1 is degenerated, so add edge2 & edge3 into act_edge_list
					set_act_edge_from_two_edge(poly->edge2_ptr, poly->edge3_ptr, act_edge);
				}
			}
			else { // "1pt"
				set_act_edge_from_two_edge(poly->edge1_ptr, poly->edge2_ptr, act_edge);
			}
			act_edge_list.push_back(act_edge);
		}
		// traverse edges in act_edge_list
		for (ActivateEdgeNode *edge = act_edge_list.head(), *next_edge = nullptr; edge != nullptr; edge = next_edge) {
			next_edge = edge->next;
			if (edge->x_l > right_bound || edge->x_r < left_bound) {
				move_to_next_scan_line(edge, i);
				continue;
			}
			int edge_l_pixel = transfer_to_pixel_pos(edge->x_l, "x");
			int edge_r_pixel = transfer_to_pixel_pos(edge->x_r, "x");
			size_t start_j = edge_l_pixel > 0 ? edge_l_pixel : 0;
			size_t stop_j = edge_r_pixel < width ? edge_r_pixel : width - 1;
			GLfloat z_val = edge->z_l;
			for (int j = start_j; j < stop_j; j++) {
				if (start_j == 0 && j == 0) {
					z_val = edge->z_l + (0 - edge_l_pixel) * edge->dz_x;
				}
				else if (j != start_j) {
					z_val += edge->dz_x;
				}
				if (z_val < -near_bound && z_val > z_buffer[j]) {
					z_buffer[j] = z_val;
#ifdef Z_BUFFER_COLOR
					frame_buffer[j].r = (unsigned char)((1.0 + z_val / (view.radius * 2)) * 255);
					frame_buffer[j].g = 0;
					frame_buffer[j].b = (unsigned char)((-z_val / (view.radius * 2)) * 255);
					if (frame_buffer[j].r > 180 && frame_buffer[j].b < 70) {
						cout << frame_buffer[j].r << " " << frame_buffer[j].g << " " << frame_buffer[j].b << endl;
					}
#else
					frame_buffer[j] = edge->poly_ptr->color;
#endif
				}
			}
			move_to_next_scan_line(edge, i);
		}
		move_to_total_buffer(i);
	}
}

void ActivateTable::move_to_next_scan_line(ActivateEdgeNode* edge, int h) {
	edge->cy_l--;
	edge->cy_r--;
	edge->poly_ptr->cy_remain--;
	if (edge->poly_ptr->cy_remain == 0 || edge->cy_l == 0 && edge->cy_r == 0 || h == 0) {
		act_poly_list.erase(edge->poly_ptr, "delete");
		act_edge_list.erase(edge, "delete");
		return;
	}
	else if (edge->cy_l == 0) {
		edge->x_r += edge->dx_r;
		EdgeNode* edge3 = edge->poly_ptr->static_poly_ptr->edge3_ptr;
		assert((edge3 != nullptr || edge3->y == h) && "ActivateTable::move_to_next_scan_line: edge3 error");
		edge->x_l = edge3->x;
		edge->cy_l = edge3->cy;
		edge->dx_l = edge3->dx;
		edge->z_l = edge3->z;
	}
	else if (edge->cy_r == 0) {
		edge->x_l += edge->dx_l;
		edge->z_l += edge->dz_x * (edge->dx_l / d_width) + edge->dz_y;
		EdgeNode* edge3 = edge->poly_ptr->static_poly_ptr->edge3_ptr;
		assert((edge3 != nullptr || edge3->y == height) && "ActivateTable::move_to_next_scan_line: edge3 error");
		edge->x_r = edge3->x;
		edge->cy_r = edge3->cy;
		edge->dx_r = edge3->dx;
	}
	else {
		edge->x_l += edge->dx_l;
		edge->x_r += edge->dx_r;
		edge->z_l += edge->dz_x * (edge->dx_l / d_width) + edge->dz_y;
	}
}

void ActivateTable::reset() {
	delete[] total_frame_buffer;
	act_poly_list.delete_all();
	act_edge_list.delete_all();
	z_buffer.clear();
	frame_buffer.clear();
	z_buffer.resize(width);
	frame_buffer.resize(width);
	total_frame_buffer = new unsigned char[3 * height*width];
}