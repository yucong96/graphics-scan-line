#pragma once

#include "alg_head.h"
#include "BaseTable.h"
#include "StaticTable.h"
#include "ActivateTable.h"

class ActivateTable4ISL : public ActivateTable {
public:
	Color background_color;

	ActivateTable4ISL() {}
	ActivateTable4ISL(const StaticTable& static_table) : ActivateTable(static_table) {}
	void traverse_display(StaticTable& static_table, const View &view);
	void reset();
};