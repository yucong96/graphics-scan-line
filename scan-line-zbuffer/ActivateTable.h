#pragma once

#include "alg_head.h"
#include "BaseTable.h"
#include "StaticTable.h"

class ActivateTable : public BaseTable {
private:
	void reset_buffers() {
		for (int i = 0; i < width; i++) {
			z_buffer[i] = -far_bound;
			frame_buffer[i] = background_color;
		}
	}
	void set_act_edge_left_and_right(const EdgeNode* left_node, const EdgeNode* right_node, ActivateEdgeNode* act_edge);
	void set_act_edge_from_two_edge(const EdgeNode* edge1, const EdgeNode* edge2, ActivateEdgeNode* act_edge);
public:
	std::vector<GLfloat> z_buffer;
	std::vector<Color> frame_buffer;
	unsigned char* total_frame_buffer;
	BiList<ActivatePolyNode> act_poly_list;
	BiList<ActivateEdgeNode> act_edge_list;

	Color background_color;

	ActivateTable() {}
	ActivateTable(const StaticTable& static_table) : BaseTable(static_table) {
		z_buffer.resize(width);
		frame_buffer.resize(width);
		total_frame_buffer = new unsigned char[3 * height*width];
	}
	void traverse(StaticTable& static_table, const View& view);
	void move_to_next_scan_line(ActivateEdgeNode* edge, int h);
	void move_to_total_buffer(int h) {
		for (size_t i = 0, sz = frame_buffer.size(); i < sz; i++) {
			total_frame_buffer[h*width * 3 + i * 3] = frame_buffer[i].r;
			total_frame_buffer[h*width * 3 + i * 3 + 1] = frame_buffer[i].g;
			total_frame_buffer[h*width * 3 + i * 3 + 2] = frame_buffer[i].b;
		}
	}
	void display() {
		glDrawPixels(width, height, GL_RGB, GL_UNSIGNED_BYTE, (void*)total_frame_buffer);
	}
	void reset();
};