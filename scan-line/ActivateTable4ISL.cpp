#include "ActivateTable4ISL.h"

using namespace std;
using namespace Eigen;

void draw_line(Color c, int x1, int y1, int x2, int y2) {
	glColor3f((float)c.r / 255, (float)c.g / 255, (float)c.b / 255);
	glVertex2f(x1, y1);
	glColor3f((float)c.r / 255, (float)c.g / 255, (float)c.b / 255);
	glVertex2f(x2, y2);
}
void draw_line_with_color_z(int x1, int y1, double z1, int x2, int y2, double z2, double r) {
	glColor3f(1 + z1 / (2 * r), 0, -z1 / (2 * r));
	glVertex2f(x1, y1);
	glColor3f(1 + z2 / (2 * r), 0, -z2 / (2 * r));
	glVertex2f(x2, y2);
}

void ActivateTable4ISL::traverse_display(StaticTable& static_table, const View& view) {
	for (int h = height - 1; h >= 0; h--) {
		// add new act_poly & act_edge
		for (PolyNode* poly = static_table.poly_table[h].head(); poly != nullptr; poly = poly->next) {
			// add new poly
			ActivatePolyNode* act_poly = new ActivatePolyNode(poly);
			act_poly_list.push_back(act_poly);
			poly->act_poly_ptr = act_poly;
			// add edge from new poly
			add_act_edge_from_poly(static_table, poly, act_poly, h);
		}
		//for (EdgeNode* edge = static_table.edge_table[h].head(); edge != nullptr; edge = static_table.edge_table[h].head()) {
		//	// add edge from remaining edge_table // for concave poly
		//	PolyNode* poly = edge->poly_ptr;
		//	ActivatePolyNode* act_poly = poly->act_poly_ptr;
		//	if (act_poly->cy_remain == 0) {
		//		static_table.edge_table[h].erase(edge, "remove");
		//		continue;
		//	}
		//	add_act_edge_from_poly(static_table, poly, act_poly, h);
		//}

		// build intervals
		vector<Interval> intervals(2 * act_edge_list.size()); {
			int i = 0;
			for (ActivateEdgeNode *edge = act_edge_list.head(); edge != nullptr; edge = edge->next) {
				intervals[i++].x_l = edge->x_l;
				intervals[i++].x_l = edge->x_r;
			}
			sort(intervals.begin(), intervals.end(), [](const Interval& i1, const Interval& i2) -> bool {
				return i1.x_l < i2.x_l;
			});
			for (ActivateEdgeNode *edge = act_edge_list.head(); edge != nullptr; edge = edge->next) {
				bool flag = false;
				for (int i = 0; i < intervals.size()-1; i++) {
					if (edge->x_l <= intervals[i].x_l && edge->x_r >= intervals[i+1].x_l) {
						intervals[i].belongs.push_back(edge);
					}
				}
			}
		}

		// display
		glBegin(GL_LINES);
		if (intervals.size() == 0) {
			draw_line(background_color, 0, h, width-1, h);
		}
		else {
			for (int i = 0; i < (int)(intervals.size() - 1); i++) {
				if (i == 0 && intervals[i].x_l > left_bound) {
					int x = transfer_to_pixel_pos(intervals[i].x_l, "x");
					x = x < width ? x : width;
					draw_line(background_color, 0, h, x, h);
				}
				if (i == (int)intervals.size() - 2 && intervals[i+1].x_l < right_bound) {
					int x = transfer_to_pixel_pos(intervals[i+1].x_l, "x");
					x = x >= 0 ? x : 0;
					draw_line(background_color, x, h, width-1, h);
				}
				if (intervals[i].x_l > right_bound || intervals[i + 1].x_l < left_bound) {
					continue;
				}
				
				int start_x = transfer_to_pixel_pos(intervals[i].x_l, "x");
				start_x = start_x >= 0 ? start_x : 0;
				int stop_x = transfer_to_pixel_pos(intervals[i + 1].x_l, "x");
				stop_x = stop_x < width ? stop_x : width-1;
				if (intervals[i].belongs.size() == 0) { // background
					draw_line(background_color, start_x, h, stop_x, h);
				}
				else if (intervals[i].belongs.size() == 1) { // only one poly
					ActivateEdgeNode* act_edge = intervals[i].belongs[0];
					double interval_width = intervals[i + 1].x_l - intervals[i].x_l;
					double z_left = act_edge->z_l + act_edge->dz_x * (intervals[i].x_l - act_edge->x_l) / d_width;
					double z_right = act_edge->z_l + act_edge->dz_x * (intervals[i + 1].x_l - act_edge->x_l) / d_width;
					draw_line_with_color_z(start_x, h, z_left, stop_x, h, z_right, view.radius);
				}
				else {
					int min_z_idx = 0;
					double min_z_val = -1e6;
					for (int edge_idx = 0; edge_idx < intervals[i].belongs.size(); edge_idx++) {
						ActivateEdgeNode* edge = intervals[i].belongs[edge_idx];
						double interval_mid_x = (intervals[i + 1].x_l + intervals[i].x_l) / 2;
						double edge_mid_z = edge->z_l + edge->dz_x * (interval_mid_x - edge->x_l) / d_width;
						if (edge_mid_z > min_z_val) {
							min_z_val = edge_mid_z;
							min_z_idx = edge_idx;
						}
					}
					ActivateEdgeNode* act_edge = intervals[i].belongs[min_z_idx];
					double z_left = act_edge->z_l + act_edge->dz_x * (intervals[i].x_l - act_edge->x_l) / d_width;
					double z_right = act_edge->z_l + act_edge->dz_x * (intervals[i+1].x_l - act_edge->x_l) / d_width;
					draw_line_with_color_z(start_x, h, z_left, stop_x, h, z_right, view.radius);
				}
			}
		}
		glEnd();

		// move to next line
		for (ActivateEdgeNode *edge = act_edge_list.head(), *next_edge; edge != nullptr; edge = next_edge) {
			next_edge = edge->next;
			move_to_next_scan_line(edge, h);
		}
	}
}

void ActivateTable4ISL::reset() {
	act_poly_list.delete_all();
	act_edge_list.delete_all();
}