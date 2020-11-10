#pragma once
#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "SDL.h"
#include "mesh.h"
#include <iostream>
#include "mesh.h"
#include <algorithm>
#include "math.h"
#include "math3d.h"
#include <vector>
#include "object.h"

struct frust {
	float horfov = 90;
	float verfov = 90;
	float hor_res = 640;
	float ver_res = 480;
	float near = 0.1;
	float far = 50;
};

struct camera {
	vec3d camera_pos = { 0, 0, -1, 1 };
	vec3d angles = { 0, 0, 0, 1 };
	matx3d matrix = {
	{1, 0, 0, 0},
	{0, 1, 0, 0},
	{0, 0, 1, 0},
	{0, 0, 0, 1} };
	frust frustrum;
};

namespace clip_tags {
	typedef int OutCode;

	const int INSIDE = 0; // 000000
	const int LEFT = 1;   // 000001
	const int RIGHT = 2;  // 000010
	const int BOTTOM = 4; // 000100
	const int TOP = 8;    // 001000
	const int NEAR = 16;  // 010000
	const int FAR = 32; // 100000
}

int to_point(SDL_Point* out, vec3d point);

SDL_Window* win_make_window(int display_width, int display_height, SDL_WindowFlags flags);

int clear_depth_buffer(std::vector<float>& buffer, const int* sizex, const int* sizey);

int draw_tri(SDL_Renderer* renderer, std::vector<float>& buffer_out, tri3d triangle, int x_width, int y_width);

int convert_tri(tri3d triangle, camera camera, tri3d* converted_tri);

void camera_matrix(matx3d* cam_matx, vec3d camera_pos);

void object_matrix(matx3d* object_matrix, vec3d object_pos, vec3d object_rot);

void projection_matrix(matx3d* projection_matx, camera camera, int type);

int full_convert_obj(SDL_Renderer* renderer, object_info object, camera camera, std::vector<float>& depth_buffer, int half_screen_x, int half_screen_y);

int clip_far(tri3d* converted_tri1, tri3d* converted_tri2, vec3d depth);

int clip_near(tri3d* converted_tri1, tri3d* converted_tri2, vec3d depth);

#endif