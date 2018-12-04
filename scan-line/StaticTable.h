#pragma once

#include "alg_head.h"
#include "BaseTable.h"

class StaticTable : public BaseTable {
private:
	void make_edge_node(PolyNode* poly, const Eigen::Vector3f& v_top, const Eigen::Vector3f& v_bottom, EdgeNode* enode);
	Eigen::Vector3f compute_cross_vertex(const Eigen::Vector3f& v1, const Eigen::Vector3f& v2, float y_line);
public:
	std::vector<BiList<PolyNode>> poly_table;
	std::vector<BiList<EdgeNode>> edge_table;
	size_t poly_num;

	StaticTable() {}
	StaticTable(GLfloat _left, GLfloat _right, GLfloat _bottom, GLfloat _top, GLfloat _near, GLfloat _far, int _width, int _height)
		: BaseTable(_left, _right, _bottom, _top, _near, _far, _width, _height) {
		poly_num = 0;
		poly_table.resize(height);
		edge_table.resize(height);
	}
	void add_obj(Object &obj);
	void reset() {
		poly_num = 0;
		for (auto poly_list : poly_table) {
			poly_list.delete_all();
		}
		poly_table.clear();
		edge_table.clear();
		poly_table.resize(height);
		edge_table.resize(height);
	}
};
