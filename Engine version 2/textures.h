#pragma once
#ifndef TEXTURES_H
#define TEXTURES_H
#include <vector>
#include <iostream>
#include "bitmap.h"

struct color {
	int r = 255;
	 int g = 255;
	 int b = 255;
	 int a = 255;
};

struct texture {
	int width = 0;
	int height = 0;
	texture() {
		width = 0;
		height = 0;
	}
	bool texture_set() { return texture_is_set; }
	int load_texture(const char* path) {
		bitmap.read(path);
		texture_is_set = true;
		width = bitmap.bmp_info_header.width;
		height = bitmap.bmp_info_header.height;
		return 0;
	}
	int set_window() {
		window_is_set = true;
		return 0;
	}
	int clear_window() {
		window_is_set = false;
		return 0;
	}
	color get_pixel(int x, int y){
		return { bitmap.data.at(3 * (x + (height - y - 1) * (width)) + 2), 
			bitmap.data.at(3 * (x + (height - y - 1) * (width)) + 1), 
			bitmap.data.at(3 * (x + (height - y - 1) * (width))) };
	}
private:
	BMP bitmap;
	bool texture_is_set = false;
	bool window_is_set = false;
};

#endif