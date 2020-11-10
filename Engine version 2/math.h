#pragma once
#ifndef MATH_H
#define MATH_H

struct vector {
	float x;
	float y;
	float q = 0;
};

struct vector2d {
	float x;
	float y;
};

typedef vector point;

struct ray {
	point org;
	vector dir;
};

struct seg {
	point start;
	point end;
};

int seg_seg_intersect(seg a, seg b, float* seg1_time, float* seg2_time, float* crossx, float* crossy);

vector vect_sub(vector a, vector b);

vector vect_add(vector a, vector b);

vector vect_div_value(vector a, float value);

vector vect_by_value(vector a, float value);

point ray_pos_from_t(ray a, float t);

float vect_dot_vect(vector a, vector b);

float vect_dot_vect(vector2d a, vector2d b);

float flat_cross_product(vector2d a, vector2d b);

#endif

