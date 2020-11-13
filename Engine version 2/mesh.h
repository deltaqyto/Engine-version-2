#pragma once
#ifndef MESH_H
#define MESH_H
#include "math3d.h"

struct tri3d {
	vec3d p1;
	vec3d p2;
	vec3d p3;
	vec3d normal;
};

int get_normal(tri3d* tri);

#endif

