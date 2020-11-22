#include "SDL.h"
#include "math.h"
#include "math3d.h"
#include "graphics.h"
#include "mesh.h"
#include "object.h"
#include "textures.h"
#include <iostream>
#include <vector>    

//ShowWindow( GetConsoleWindow(), SW_HIDE );  <- hide console

int main(int argc, char* argv[]) {

	const int half_screen_size[2] = { 320 * 1, 240 * 1 };   // Half size saved for speed
	const SDL_WindowFlags flags = SDL_WINDOW_SHOWN;
	int mouse_position[2] = { half_screen_size[0], half_screen_size[1] };

	SDL_Window* window = NULL;
	SDL_Surface* screenSurface = NULL;
	SDL_Renderer* renderer = NULL;

	texture cube_tex;
	cube_tex.load_texture("crate.bmp");
	texture axe;
	axe.load_texture("axes.bmp");
	texture hangar_tex;
	hangar_tex.load_texture("hangar.bmp");


	object_info cube;
	{
		cube.add_texture(&cube_tex);
		load_from_obj(&cube, "cube.obj", 0);
		cube.model_org = { 5, 0, 0, 1 };
		cube.model_rot = { 0, 0, 0, 1 };
		cube.tags.fullbright = true;
		cube.tags.debug_color = true;
		cube.tags.bfc = false;
		cube.tags.show_object = false;
		zip_verts_tris(&cube);
	}

	object_info cube_sliced;
	{
		cube_sliced.verts = { -0.5, 0.5, 1, -0.5, 0.5, 0, 0.5, -0.5, 0, 0.5, 0.5, 1, -0.5, -0.5, 0, 0.5, -0.5, 1, -0.5, -0.5, 1 };
		cube_sliced.tris = { 0, 3, 1, 3, 2, 1, 1, 2, 4, 3, 5, 2, 0, 5, 3, 0, 6, 5, 6, 0, 1, 6, 1, 4 };
		cube_sliced.model_org = { 5, 0, 0, 1 };
		cube_sliced.model_rot = { 0, 0, 180, 1 };
		cube_sliced.tags.fullbright = true;
		cube_sliced.tags.debug_color = true;
		cube_sliced.tags.bfc = false;
		cube_sliced.tags.show_object = false;
		zip_verts_tris(&cube_sliced);
	}

	object_info dragon;
	{
		load_from_obj(&dragon, "dragondown.obj", 1);
		dragon.model_org = { 5, -5, 5, 1 };
		dragon.model_rot = { 180, 90, 90, 1 };
		dragon.color = { 252, 177, 3, 255 };
		dragon.tags.fullbright = false;
		dragon.tags.show_object = false;
		dragon.tags.bfc = false;
		zip_verts_tris(&dragon);
	}

	object_info ground;
	{
		ground.verts = { -10, -10, 1, 10, -10, 1, -10, 10, 1, 10, 10, 1 };
		ground.tris = { 0, 1, 2, 1, 3, 2 };
		ground.model_org = { 0, 0, 0, 1 };
		ground.model_rot = { 0, 0, 0, 1 };
		ground.color = { 206, 252, 3, 255 };
		ground.tags.fullbright = true;
		ground.tags.show_object = false;
		zip_verts_tris(&ground); 
	}

	object_info axes;
	{
		axes.add_texture(&axe);
		load_from_obj(&axes, "axes.obj", 0);
		axes.color = { 255, 255, 255, 255 };
		axes.model_org = { 5, 0, 0, 1 };
		axes.model_rot = { 0, 0, 0, 1 };
		axes.tags.fullbright = true;
		axes.tags.bfc = false;
		axes.tags.debug_color = true;
		axes.tags.show_object = false;
		axes.tags.use_texture = true;
		zip_verts_tris(&axes);
	}

	object_info hangar;
	{
		hangar.add_texture(&hangar_tex);
		load_from_obj(&hangar, "hangar.obj", 0);
		hangar.color = { 0, 255, 255, 255 };
		hangar.model_org = { 0, 0, 0, 1 };
		hangar.model_rot = { 180, 0, 0, 1 };
		hangar.tags.fullbright = true;
		hangar.tags.bfc = false;
		hangar.tags.debug_color = false;
		hangar.tags.show_object = true;
		hangar.tags.use_texture = true;
		zip_verts_tris(&hangar);
	}

	frust frustrum;
	{
		frustrum.hor_res = half_screen_size[0] * 2.5;
		frustrum.ver_res = half_screen_size[1] * 2;
		frustrum.horfov = 90;
		frustrum.near = 0.25;
		frustrum.far = 15;
	}

	camera camera;
	camera.frustrum = frustrum;
	camera.camera_pos = { 0, 0, 0, 1 };

	light light;
	light.is_sun = true;
	light.direction = { 1, 1, 0, 0 };

	int dir[] = { 0 };
	float speed = 0.5;

	float time = 0;
	bool rotate = true;
	bool mouse_enable = true;

	std::vector<float> depth_buffer = {};
	depth_buffer.resize(4 * static_cast<__int64>(half_screen_size[0]) * static_cast<__int64>(half_screen_size[1]));

	std::vector<float> color_buffer = {};
	color_buffer.resize(12 * static_cast<__int64>(half_screen_size[0]) * static_cast<__int64>(half_screen_size[1]));

	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError()); // Catch startup errors

	else {
		SDL_Event event_handle;
		window = win_make_window(half_screen_size[0] * 2, half_screen_size[1] * 2, flags);
		screenSurface = SDL_GetWindowSurface(window);
		renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

		bool quit = false;
		while (!quit) {
			SDL_GetMouseState(&(mouse_position[0]), &(mouse_position[1]));
			while (SDL_PollEvent(&event_handle) != 0) {
				if (event_handle.type == SDL_QUIT) {
					quit = true;
				}
				//User presses a key
				else if (event_handle.type == SDL_KEYDOWN)
				{
					//Select surfaces based on key press
					switch (event_handle.key.keysym.sym)
					{
					case SDLK_UP:
						dir[0] = 1;
						break;

					case SDLK_DOWN:
						dir[0] = -1;
						break;

					case SDLK_p: //Debug print, unmapped so far
						print_vec3d(camera.camera_pos);
						break;

					case SDLK_o: //Debug print, unmapped so far
						print_vec3d(camera.camera_vect);
						break;

					case SDLK_x: //Debug stop
						quit = true;
						break;
					}
				}
			}
			if (rotate) {
				time += 0.02;
			}

			light.direction = { 0, sin(time), cos(time), 0 };
			if (mouse_enable) {
				set_cam_rotation(&camera.angles, (mouse_position[1] / (float)(half_screen_size[1] * 2) - 0.5f) * 180, (mouse_position[0] / (float)(half_screen_size[0] * 2) - 0.5f) * 360);
			} else {
				set_cam_rotation(&camera.angles, 0, 0);
			}
			clear_buffer(depth_buffer);
			clear_buffer(color_buffer);
			setup_render(&camera, renderer);

			if (dir[0] == 1) {
				camera.camera_pos.x += camera.camera_vect.x * speed;
				camera.camera_pos.y += camera.camera_vect.y * speed;
				camera.camera_pos.z += camera.camera_vect.z * speed;
			} else if (dir[0] == -1) {
				camera.camera_pos.x -= camera.camera_vect.x * speed;
				camera.camera_pos.y -= camera.camera_vect.y * speed;
				camera.camera_pos.z -= camera.camera_vect.z * speed;
			}
			dir[0] = 0;

			full_convert_obj(color_buffer, cube_sliced, camera, depth_buffer, half_screen_size[0], half_screen_size[1], light);
			full_convert_obj(color_buffer, dragon, camera, depth_buffer, half_screen_size[0], half_screen_size[1], light);
			full_convert_obj(color_buffer, ground, camera, depth_buffer, half_screen_size[0], half_screen_size[1], light);
			full_convert_obj(color_buffer, axes, camera, depth_buffer, half_screen_size[0], half_screen_size[1], light);
			full_convert_obj(color_buffer, cube, camera, depth_buffer, half_screen_size[0], half_screen_size[1], light);
			full_convert_obj(color_buffer, hangar, camera, depth_buffer, half_screen_size[0], half_screen_size[1], light);
			draw_buffer(renderer, color_buffer, half_screen_size[0], half_screen_size[1], 1);

			SDL_RenderPresent(renderer);
		}
	}
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
	SDL_Quit();

	return 0;
}
