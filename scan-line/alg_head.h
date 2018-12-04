#pragma once

#include <iostream>
#include <vector>
#include <list>
#include <set>
#include <string>

#include <Eigen/Dense>

#define FREEGLUT_STATIC
#include <GL/freeglut.h>

#include "../io/obj.h"
#include "../ds/BiList.h"

struct Color {
	unsigned char r, g, b;
};

inline bool equal(double a, double b) {
	if (a < b + 1e-5 && b < a + 1e-5) {
		return true;
	}
	else return false;
}

struct EdgeNode;
struct PolyNode;
struct ActivatePolyNode;
struct ActivateEdgeNode;
struct Interval;

struct EdgeNode {
	double x;	// for the top point
	double dx;	// dx in two adjacent scanning line
	int cy;		// cross scanning line num in y direction
	int y;
	double z;	// for the top point
	PolyNode* poly_ptr;
	EdgeNode *prev, *next;
	void reset() {}
};

struct PolyNode {
	double a, b, c, d;	// plane parameter
	int y;
	int cy;				// cross scanning line num in y direction
	Color color;
	std::vector<EdgeNode*> edge_ptrs;
	ActivatePolyNode* act_poly_ptr;
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
		for (int i = 0; i < edge_ptrs.size(); i++) {
			if (edge_ptrs[i] != nullptr) {
				delete edge_ptrs[i];
			}
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

struct Interval {
	double x_l;
	std::vector<ActivateEdgeNode*> belongs;
};