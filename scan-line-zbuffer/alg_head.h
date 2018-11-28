#pragma once

#include <iostream>
#include <vector>
#include <list>
#include <set>
#include <string>

#include <Eigen/Dense>

#include "../io/obj.h"
#include "../ds/BiList.h"

struct Color {
	unsigned char r, g, b;
};

struct EdgeNode {
	double x;	// for the top point
	double dx;	// dx in two adjacent scanning line
	int cy;		// cross scanning line num in y direction
	int y;
	double z;	// for the top point
	EdgeNode *prev, *next;
};

struct PolyNode {
	double a, b, c, d;	// plane parameter
	int cy;				// cross scanning line num in y direction
	Color color;
	std::string type;
	EdgeNode *edge1_ptr, *edge2_ptr, *edge3_ptr;
	PolyNode *prev, *next;
	void coord2func(const Eigen::Vector3f &v1, const Eigen::Vector3f &v2, const Eigen::Vector3f &v3) {
		Eigen::Vector3f edge1 = v1 - v2;
		Eigen::Vector3f edge2 = v1 - v3;
		Eigen::Vector3f norm = edge1.cross(edge2);
		a = norm[0];
		b = norm[1];
		c = norm[2];
		d = -(a*v1[0] + b*v1[1] + c*v1[2]);
		if (d > 1e-10 || d < -1e-10) {
			a /= d;
			b /= d;
			c /= d;
			d = 1;
		}
		else if (c > 1e-10 || c < -1e-10) {
			a /= c;
			b /= c;
			c = 1;
			d = 0;
		}
		else if (b > 1e-10 || b < -1e-10) {
			a /= b;
			b = 1;
			c = d = 0;
		}
		else {
			a = 1;
			b = c = d = 0;
		}
	}
	void reset() {
		if (edge1_ptr != nullptr) {
			delete edge1_ptr;
		}
		if (edge2_ptr != nullptr) {
			delete edge2_ptr;
		}
		if (edge3_ptr != nullptr) {
			delete edge3_ptr;
		}
	}
};

struct ActivatePolyNode {
	double a, b, c, d;	// plane parameter
	int cy_remain;		// remaining cross scanning line num in y direction
	Color color;
	PolyNode* static_poly_ptr;
	ActivatePolyNode *prev, *next;
	ActivatePolyNode(PolyNode* poly) {
		a = poly->a, b = poly->b, c = poly->c, d = poly->d;
		color = poly->color;
		cy_remain = poly->cy;
		static_poly_ptr = poly;
	}
	void reset() {}
};

struct ActivateEdgeNode {
	double x_l, x_r;	// for left & right intersection
	double dx_l, dx_r;	// dx for left & right intersection
	int cy_l, cy_r;		// remaining cross scanning line num in y direction
	double z_l;			// z for left intersection
	double dz_x, dz_y;	// dz in x & y direction
	ActivatePolyNode *poly_ptr;
	ActivateEdgeNode *prev, *next;
	void reset() {}
};

