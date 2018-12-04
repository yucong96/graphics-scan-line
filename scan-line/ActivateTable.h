#pragma once

#include "alg_head.h"
#include "BaseTable.h"
#include "StaticTable.h"

class ActivateTable : public BaseTable {
protected:
	void set_act_edge_left_and_right(const EdgeNode* left_node, const EdgeNode* right_node, ActivateEdgeNode* act_edge);
	void set_act_edge_from_two_edge(const EdgeNode* edge1, const EdgeNode* edge2, ActivateEdgeNode* act_edge);
	void add_act_edge_from_poly(StaticTable& static_table, PolyNode* poly, ActivatePolyNode* act_poly, int h);
	void move_to_next_scan_line(ActivateEdgeNode* edge, int h);
public:
	BiList<ActivatePolyNode> act_poly_list;
	BiList<ActivateEdgeNode> act_edge_list;

	Color background_color;

	ActivateTable() {}
	ActivateTable(const StaticTable& static_table) : BaseTable(static_table) {}
	virtual void traverse_display(StaticTable& static_table, const View& view) = 0;
	virtual void reset() = 0;
};