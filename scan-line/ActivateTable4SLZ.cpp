#include "ActivateTable4SLZ.h"

using namespace std;
using namespace Eigen;

void ActivateTable4SLZ::traverse_display(StaticTable& static_table, const View& view) {
	for (int h = height - 1; h >= 0; h--) {
		reset_buffers();
		for (PolyNode* poly = static_table.poly_table[h].head(); poly != nullptr; poly = poly->next) {
			// add new poly
			ActivatePolyNode* act_poly = new ActivatePolyNode(poly);
			act_poly_list.push_back(act_poly);
			// add edge from new poly
			add_act_edge_from_poly(static_table, poly, act_poly, h);
		}
		// traverse edges in act_edge_list
		for (ActivateEdgeNode *edge = act_edge_list.head(), *next_edge = nullptr; edge != nullptr; edge = next_edge) {
			next_edge = edge->next;
			if (edge->x_l > right_bound || edge->x_r < left_bound) {
				move_to_next_scan_line(edge, h);
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
			move_to_next_scan_line(edge, h);
		}
		move_to_total_buffer(h);
	}
	glDrawPixels(width, height, GL_RGB, GL_UNSIGNED_BYTE, (void*)total_frame_buffer);
}

void ActivateTable4SLZ::reset() {
	delete[] total_frame_buffer;
	act_poly_list.delete_all();
	act_edge_list.delete_all();
	z_buffer.clear();
	frame_buffer.clear();
	z_buffer.resize(width);
	frame_buffer.resize(width);
	total_frame_buffer = new unsigned char[3 * height*width];
}