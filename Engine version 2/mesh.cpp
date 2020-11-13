#include "mesh.h"

int get_normal(tri3d* tri) {
	vec3d ab = { tri->p2.x - tri->p1.x, tri->p2.y - tri->p1.y, tri->p2.z - tri->p1.z, 1 };
	vec3d ac = { tri->p3.x - tri->p1.x, tri->p3.y - tri->p1.y, tri->p3.z - tri->p1.z, 1 };
	tri->normal = vec3d_norm_cross(ab, ac);
	return 0;
}