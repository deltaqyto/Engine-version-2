#pragma once
#ifndef MATH3D_H
#define MATH3D_H

struct vec3d {
	float x = 0;
	float y = 0;
	float z = 0;
	float q = 1;
};

struct matx3d {
	vec3d a;
	vec3d b;
	vec3d c;
	vec3d d;
};

struct ray3d {
	vec3d org;
	vec3d dir;
};

struct plane {
	vec3d org;
	vec3d norm;
};

int ray_plane_intersect(ray3d ray, plane plane, float* time, float* collisionx, float* collisiony, float* collisionz);

vec3d vec3d_by_const(vec3d a, float val);

float vec3d_dot(vec3d a, vec3d b);

vec3d matrix_by_vect(matx3d a, vec3d b);

vec3d to_vec(float comp_a, float comp_b, float comp_c, float comp_d);

vec3d vector_clip(float dist, vec3d* lineStart, vec3d* lineEnd);

ray3d seg_to_ray(vec3d a, vec3d b);

int normalize(vec3d* a);

int set_rotation(matx3d* input, vec3d angle, float anglex, float angley, float anglez);

int delta_rotate(matx3d* input, vec3d angle, float anglex, float angley, float anglez);

matx3d operator*(matx3d a, matx3d b);

vec3d operator*(matx3d a, vec3d b);

int get_inverse(matx3d* cam_matx);

#endif