#pragma once
#ifndef MESH_H
#define MESH_H
#include "math3d.h"
#include "math.h"
#include "textures.h"

struct tri3d {
	vec3d p1 = { 0, 0, 0, 1 };
	vec3d p2 = { 0, 0, 0, 1 };
	vec3d p3 = { 0, 0, 0, 1 };
	vec3d normal = { 0, 0, 0, 1 };
	vector tx = { 0, 0, 0 };
	vector ty = { 0, 0, 0 };
	color cp1 = { 0, 0, 0, 255 };
	color cp2 = { 0, 0, 0, 255 };
	color cp3 = { 0, 0, 0, 255 };
};

int get_normal_from_tri(tri3d* tri);

#endif

