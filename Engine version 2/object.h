#pragma once
#ifndef OBJECT_H
#define OBJECT_H
#include "math3d.h"
#include "mesh.h"
#include <fstream>
#include <iostream>
#include "text.h"

struct tags {
	bool noclipping = false;
	bool smooth_shade = true;//No imp
	bool depth_test = true;	  // Still writes to buffer, no read
	bool fullbright = false;
	bool bfc = true;
	bool debug_color = false;
	bool show_object = true;
};

struct object_info {
	std::vector <float> verts; // Flattened array for speed
	std::vector <int> tris; // Flattened array for speed
	std::vector <vec3d> normals;
	std::vector <vec3d> colors;
	std::vector <tri3d> model_mesh; // Derived from verts and tris
	tags tags;
	vec3d model_org = {0, 0, 0, 1};
	vec3d model_rot = {0, 0, 0, 1};
	vec3d color = {255, 255, 0, 255};

	void set_zipped() { is_zipped = true; } // Set and get is_zipped
	bool get_zipped() { return is_zipped; }
private:
	bool is_zipped = false; // Set by function

	// Look at Pg 222
};

int zip_verts_tris(object_info* object);   //Move into object struct

int load_from_obj(object_info* object, std::string file_name, int type);

struct light {
	vec3d direction = { 0, 0, 0, 1 };
	vec3d color = { 255, 255, 255, 0 };  //RGB, last one unused
	vec3d position = { 0, 0, 0, 1 };
	float ambient = 0.1f;
	bool is_sun = false;
};

#endif