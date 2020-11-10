#pragma once
#ifndef OBJECT_H
#define OBJECT_H
#include <vector>
#include "math3d.h"
#include "mesh.h"

struct tags {
	bool noclipping = false;
	bool smooth_shade = true;
	bool depth_test = true;
};

struct object_info {
	std::vector <float> verts; // Flattened array for speed
	std::vector <float> tris; // Flattened array for speed
	std::vector <vec3d> normals;
	std::vector <vec3d> colors;
	std::vector <tri3d> model_mesh; // Derived from verts and tris
	tags tags;
	vec3d model_org;
	vec3d model_rot;

	void set_zipped() { is_zipped = true; } // Set and get is_zipped
	bool get_zipped() { return is_zipped; }
private:
	bool is_zipped = false; // Set by function

	// Look at Pg 222
};

int zip_verts_tris(object_info* object);   //Move into object struct

#endif