#include "SDL.h"
#include "math.h"
#include "math3d.h"
#include "graphics.h"
#include "mesh.h"
#include "object.h"
#include <iostream>
#include <vector>    

//ShowWindow( GetConsoleWindow(), SW_HIDE );  <- hide console

int main(int argc, char* argv[]) {

	const int half_screen_size[2] = { 320, 240 };   // Half size saved for speed
	const SDL_WindowFlags flags = SDL_WINDOW_SHOWN;
	int mouse_position[2] = { 0, 0 };

	SDL_Window* window = NULL;
	SDL_Surface* screenSurface = NULL;
	SDL_Renderer* renderer = NULL;

	// Create cube
	object_info cube;
	cube.verts = { -0.5, 0.5, 1, -0.5, 0.5, 0, 0.5, -0.5, 0, 0.5, 0.5, 1, -0.5, -0.5, 0, 0.5, -0.5, 1, -0.5, -0.5, 1 };
	cube.tris = { 0, 3, 1, 3, 2, 1, 1, 2, 4, 3, 5, 2, 0, 5, 3, 0, 6, 5, 6, 0, 1, 6, 1, 4 };
	cube.model_org = { 5, 0, 0, 1 };
	cube.model_rot = { 0, 0, 180, 1 };
	cube.tags.fullbright = true;
	cube.tags.debug_color = true;
	cube.tags.bfc = false;
	cube.tags.show_object = true;
	zip_verts_tris(&cube);

	object_info dragon;
	load_from_obj(&dragon, "dragondown.obj", 0);
	dragon.model_org = { 5, -5, 5, 1 };
	dragon.model_rot = { 180, 90, 90, 1 };
	dragon.color = { 252, 177, 3, 255};
	dragon.tags.fullbright = true;
	dragon.tags.show_object = false;
	dragon.tags.bfc = false;
	zip_verts_tris(&dragon);

	object_info ground;
	ground.verts = { -10, -10, 1, 10, -10, 1, -10, 10, 1, 10, 10, 1 };
	ground.tris = { 0, 1, 2, 1, 3, 2 };
	ground.model_org = { 0, 0, 0, 1 };
	ground.model_rot = { 0, 0, 0, 1 };
	ground.color = { 206, 252, 3, 255};
	ground.tags.fullbright = true;
	ground.tags.show_object = false;
	zip_verts_tris(&ground);

	object_info axes;					   
	load_from_obj(&axes, "axes.obj", 0);
	axes.color = { 255, 255, 255, 255 };
	axes.model_org = { 5, 0, 0, 1 };
	axes.model_rot = { 0, 0, 0, 1 };
	axes.tags.fullbright = true;
	axes.tags.bfc = false;
	axes.tags.debug_color = true;
	axes.tags.show_object = false;
	zip_verts_tris(&axes);

	frust frustrum;
	frustrum.hor_res = half_screen_size[0] * 2;
	frustrum.ver_res = half_screen_size[1] * 2;
	frustrum.near = 1;
	frustrum.far = 10;

	camera camera;
	camera.frustrum = frustrum;
	camera.camera_pos = { 0, 0, 0, 1 };

	light light;
	light.is_sun = true;
	light.direction = { 1, 1, 0, 0 };

	int dir[] = { 0 };
	float speed = 0.1;

	float time = 0;
	bool rotate = true;
	bool mouse_enable = true;

	std::vector<float> depth_buffer = {};
	depth_buffer.resize(4 * (static_cast<__int64>(half_screen_size[0]) + 1) * (static_cast<__int64>(half_screen_size[1]) + 1));

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

					case SDLK_q:
						cube.model_rot.z += 5;
						dragon.model_rot.z += 5;
						break;

					case SDLK_e:
						cube.model_rot.z -= 5;
						dragon.model_rot.z -= 5;
						break;

					case SDLK_w:
						cube.model_rot.x += 5;
						dragon.model_rot.x += 5;
						break;

					case SDLK_s:
						cube.model_rot.x -= 5;
						dragon.model_rot.x -= 5;
						break;

					case SDLK_a:
						cube.model_rot.y += 5;
						dragon.model_rot.y += 5;
						break;

					case SDLK_d:
						cube.model_rot.y -= 5;
						dragon.model_rot.y -= 5;
						break;

					case SDLK_p: //Debug print, unmapped so far
						mouse_enable = true;
						break;

					case SDLK_o: //Debug print, unmapped so far
						mouse_enable = false;
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
			}
			else {
				set_cam_rotation(&camera.angles, 0, 0);
			}
			clear_depth_buffer(depth_buffer, &half_screen_size[0], &half_screen_size[1]);
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

			full_convert_obj(renderer, cube, camera, depth_buffer, half_screen_size[0], half_screen_size[1], light);
			full_convert_obj(renderer, dragon, camera, depth_buffer, half_screen_size[0], half_screen_size[1], light);
			full_convert_obj(renderer, ground, camera, depth_buffer, half_screen_size[0], half_screen_size[1], light);
			full_convert_obj(renderer, axes, camera, depth_buffer, half_screen_size[0], half_screen_size[1], light);
			//draw_buffer(renderer, depth_buffer, half_screen_size[0], half_screen_size[1]);

			SDL_RenderPresent(renderer);
		}
	}
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
	SDL_Quit();

	return 0;
}
