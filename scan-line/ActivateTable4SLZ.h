#pragma once

#include "alg_head.h"
#include "BaseTable.h"
#include "StaticTable.h"
#include "ActivateTable.h"

class ActivateTable4SLZ : public ActivateTable {
private:
	void reset_buffers() {
		for (int i = 0; i < width; i++) {
			z_buffer[i] = -far_bound;
			frame_buffer[i] = background_color;
		}
	}
	void move_to_total_buffer(int h) {
		for (size_t i = 0, sz = frame_buffer.size(); i < sz; i++) {
			total_frame_buffer[h*width * 3 + i * 3] = frame_buffer[i].r;
			total_frame_buffer[h*width * 3 + i * 3 + 1] = frame_buffer[i].g;
			total_frame_buffer[h*width * 3 + i * 3 + 2] = frame_buffer[i].b;
		}
	}
public:
	std::vector<GLfloat> z_buffer;
	std::vector<Color> frame_buffer;
	unsigned char* total_frame_buffer;
	
	Color background_color;

	ActivateTable4SLZ() {}
	ActivateTable4SLZ(const StaticTable& static_table) : ActivateTable(static_table) {
		background_color = { 0, 255, 0 };
		z_buffer.resize(width);
		frame_buffer.resize(width);
		total_frame_buffer = new unsigned char[3 * height*width];
	}
	
	void traverse_display(StaticTable& static_table, const View& view);
	void reset();
};