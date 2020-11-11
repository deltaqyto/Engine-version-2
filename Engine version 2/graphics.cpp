#include "graphics.h"

int to_point(SDL_Point* out, vec3d point) {
	out->x = point.x;
	out->y = point.y;
	return 0;
}

SDL_Window* win_make_window(int display_width, int display_height, SDL_WindowFlags flags) {
	// Returns an SDL window given screen size and flags
	SDL_Window* window = NULL;
	window = SDL_CreateWindow("Test window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, display_width, display_height, flags);
	if (window == NULL) {
		printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
	}
	return window;
}

int clear_depth_buffer(std::vector<float>& buffer, const int* sizex, const int* sizey) {  // Time this function
	const __int64 double_sizex = static_cast<__int64>(*sizex) * 2;
	const __int64 double_sizey = static_cast<__int64>(*sizey) * 2;
	for (size_t i = double_sizey - 1; i != 0; i--) {
		for (size_t q = double_sizex - 1; q != 0; q--) {
			buffer[i * double_sizey + q] = 0.0f;
		}
	}
	return 0;
}

int draw_tri(SDL_Renderer* renderer, std::vector<float>& buffer_out, tri3d triangle, int half_x_width, int half_y_width) { // depthmap is a linear array. Buffer out is pointing to the first value
	tri3d scaled_tri = triangle;
	scaled_tri.p1.x = scaled_tri.p1.x * half_x_width + half_x_width;
	scaled_tri.p2.x = scaled_tri.p2.x * half_x_width + half_x_width;
	scaled_tri.p3.x = scaled_tri.p3.x * half_x_width + half_x_width;

	scaled_tri.p1.y = scaled_tri.p1.y * half_y_width + half_y_width;
	scaled_tri.p2.y = scaled_tri.p2.y * half_y_width + half_y_width;
	scaled_tri.p3.y = scaled_tri.p3.y * half_y_width + half_y_width;

	int x = std::min(std::min(scaled_tri.p1.x, scaled_tri.p2.x), scaled_tri.p3.x);
	int y = std::min(std::min(scaled_tri.p1.y, scaled_tri.p2.y), scaled_tri.p3.y);
	int wx = std::max(std::max(scaled_tri.p1.x, scaled_tri.p2.x), scaled_tri.p3.x) - x;
	int wy = std::max(std::max(scaled_tri.p1.y, scaled_tri.p2.y), scaled_tri.p3.y) - y;
	vector2d ac;
	ac.x = scaled_tri.p3.x - scaled_tri.p1.x;
	ac.y = scaled_tri.p3.y - scaled_tri.p1.y;
	vector2d ab;
	ab.x = scaled_tri.p2.x - scaled_tri.p1.x;
	ab.y = scaled_tri.p2.y - scaled_tri.p1.y;
	float cc = vect_dot_vect(ac, ac);
	float cb = vect_dot_vect(ac, ab);
	float cp = 0;
	float bb = vect_dot_vect(ab, ab);
	float bp = 0;
	float invDenom = 1 / (cc * bb - pow(cb, 2));
	float u = 0;
	float v = 0;
	float initx = wx;
	float inity = wy;
	float trimx = x;
	float trimy = y;

	if (x + wx > half_x_width * 2) {
		wx = half_x_width * 2 - x;
	}
	if (y + wy > half_y_width * 2) {
		wy = half_y_width * 2 - y;
	}
	if (wx < 0) {
		wx = 0;
	}
	if (wy < 0) {
		wy = 0;
	}
	if (x < 0) {
		// half_x_width = (int)(half_x_width * 2 + x) / 2;
		x = 0;
	}
	if (y < 0) {
		// half_y_width = (int)(half_y_width * 2 + y) / 2;
		y = 0;
	}
	if (wx > 640) {
		wx = 640;
	}
	if (wy > 480) {
		wy = 480;
	}

	trimx = x - trimx;
	trimy = y - trimy;

	// Debug print
	//printf("%g\n", x);
	//printf("%g\n", camera_pos.z);
	//printf("-----------------\n\n");
	// For each pixel draw color * check value

	float x_dif = x - scaled_tri.p1.x;
	float y_dif = y - scaled_tri.p1.y;
	int full_y_width = half_y_width * 2;
	float twoarea = (ab.x * ac.y - ab.y * ac.x);
	float b3 = 0;
	float barycentric_depth_weights[3] = { scaled_tri.p1.z, scaled_tri.p2.z, scaled_tri.p3.z };
	float depth_map_value = 0;

	for (size_t i = wy; i != 0; i--) {
		for (size_t q = wx; q != 0; q--) {
			vector2d ap;
			ap.x = q + x_dif;
			ap.y = i + y_dif;
			cp = vect_dot_vect(ac, ap);
			bp = vect_dot_vect(ab, ap);
			u = (bb * cp - cb * bp) * invDenom;
			v = (cc * bp - cb * cp) * invDenom;
			// b1 = u, b2 = v
			b3 = abs(1 - u - v);
			depth_map_value = b3 * barycentric_depth_weights[0] + v * barycentric_depth_weights[1] + u * barycentric_depth_weights[2];
			if (u >= 0 && v >= 0 && u + v < 1 && buffer_out[(y + i) * full_y_width + x + q] < depth_map_value) {
				// SDL_SetRenderDrawColor(renderer, b1 * barycentric_depth_weights[0] * 10, b2 * barycentric_depth_weights[1] * 10, b3 * barycentric_depth_weights[2] * 10, 255);
				// SDL_SetRenderDrawColor(renderer, depth_map_value, depth_map_value, depth_map_value, 255);
				SDL_RenderDrawPoint(renderer, (x + q), (y + i));
				buffer_out[(y + i) * full_y_width + x + q] = depth_map_value;
			}
		}
	}
	return 0;
}

int convert_tri(tri3d triangle, camera camera, tri3d* converted_tri) {
	vec3d converted_p1;
	vec3d converted_p2;
	vec3d converted_p3;
	vec3d camera_pos = camera.camera_pos;
	matx3d cam_matx = camera.matrix;
	frust cam_frust = camera.frustrum;

	plane near;
	near.norm.x = 0;
	near.norm.y = 0;
	near.norm.z = -1;
	near.norm.q = 0;

	near.org.x = 0;
	near.org.y = 0;
	near.org.z = cam_frust.near;
	near.org.q = 0;

	plane far;
	far.norm.x = 0;
	far.norm.y = 0;
	far.norm.z = 1;
	far.norm.q = 0;

	far.org.x = 0;
	far.org.y = 0;
	far.org.z = cam_frust.far;
	far.org.q = 0;

	vec3d null = { 0, 0, 0, 1 };

	camera_matrix(&cam_matx, camera_pos); // test for issues
	float tan_angle = tan(cam_frust.horfov * 0.0087266);   // const is = Pi/360 = 3.1415/360
	// Tris in obj coords
	// convert to world coord
	// convert to cam coord    cam is at 0, 0, 0 and pointing into the +z axis
	// Perspective project     at this point, the cam frust forms a -1 to 1 cube and points are mapped as such
	// clip near and far  
	// clip sides
	// On page 340

	//Convert to cam coords
	converted_p1 = triangle.p1;
	converted_p1.q = 1;
	converted_tri->p1 = matrix_by_vect(cam_matx, converted_p1);

	converted_p2 = triangle.p2;
	converted_p2.q = 1;
	converted_tri->p2 = matrix_by_vect(cam_matx, converted_p2);

	converted_p3 = triangle.p3;
	converted_p3.q = 1;
	converted_tri->p3 = matrix_by_vect(cam_matx, converted_p3);

	// Debug print
	//printf("%g\n", converted_tri->p3.z);
	//printf("%g\n", camera_pos.z);
	//printf("-----------------\n\n");

	//Basic near far clipping - to be updated
	// Quit if out of range
	if (converted_tri->p1.z > cam_frust.far || converted_tri->p1.z < cam_frust.near) {
		return 1;
	}

	// Perspective transform
	converted_p1 = converted_tri->p1;
	converted_p1.x = converted_p1.x / (tan_angle * converted_p1.z);
	converted_p1.y = converted_p1.y / (tan_angle * converted_p1.z);
	converted_p1.z = converted_p1.z / far.org.z;

	converted_p2 = converted_tri->p2;
	converted_p2.x = converted_p2.x / (tan_angle * converted_p2.z);
	converted_p2.y = converted_p2.y / (tan_angle * converted_p2.z);
	converted_p2.z = converted_p2.z / far.org.z;

	converted_p3 = converted_tri->p3;
	converted_p3.x = converted_p3.x / (tan_angle * converted_p3.z);
	converted_p3.y = converted_p3.y / (tan_angle * converted_p3.z);
	converted_p3.z = converted_p3.z / far.org.z;

	// Clip properly
	// Compute the bit code for a point (x, y) using the clip
	// bounded diagonally by (xmin, ymin), and (xmax, ymax)

	// ASSUME THAT xmax, xmin, ymax and ymin are global constants.

	// Output coords should be from -1 to 1

	// Prep depth buffer
	converted_p1.z = 1 / converted_p1.z;
	converted_p2.z = 1 / converted_p2.z;
	converted_p3.z = 1 / converted_p3.z;

	// Set Q value
	converted_p1.q = 1;
	converted_p2.q = 1;
	converted_p3.q = 1;

	// Save output
	converted_tri->p1 = converted_p1;
	converted_tri->p2 = converted_p2;
	converted_tri->p3 = converted_p3;

	return 0; // 1 signifies no output, 2 signifies clipped output

}

void camera_matrix(matx3d* cam_matx, vec3d camera_pos) {
	cam_matx->a.q = camera_pos.x;
	cam_matx->b.q = camera_pos.y;
	cam_matx->c.q = camera_pos.z;
	cam_matx->d.q = 1;

}

void object_matrix(matx3d* object_matrix, vec3d object_pos, vec3d object_rot) {  //rotation in degs

	float x_rot = object_rot.x * 0.0174f;
	float y_rot = (object_rot.y) * 0.0174f;
	float z_rot = (object_rot.z) * 0.0174f;// 3.14/180 -> radian conversion

	matx3d x = {
	{1, 0, 0, 0},
	{0, cos(x_rot), -sin(x_rot), 0},
	{0, sin(x_rot), cos(x_rot), 0},
	{0, 0, 0, 1}
	};

	matx3d y = {
	{cos(y_rot), 0, sin(y_rot), 0},
	{0, 1, 0, 0},
	{-sin(y_rot), 0, cos(y_rot), 0},
	{0, 0, 0, 1}
	};

	matx3d z = {
	{cos(z_rot), -sin(z_rot), 0, 0},
	{sin(z_rot), cos(z_rot), 0, 0},
	{0, 0, 1, 0},
	{0, 0, 0, 1}
	};

	matx3d q = {
	{object_pos.x, 0, 0, 0},
	{0, object_pos.y, 0, 0},
	{0, 0, object_pos.z, 0},
	{0, 0, 0, 1}
	};

	matx3d final_angle = x * y * z;

	object_matrix->a.x = final_angle.a.x;
	object_matrix->a.y = final_angle.a.y;
	object_matrix->a.z = final_angle.a.z;
	object_matrix->a.q = object_pos.x;

	object_matrix->b.x = final_angle.b.x;
	object_matrix->b.y = final_angle.b.y;
	object_matrix->b.z = final_angle.b.z;
	object_matrix->b.q = object_pos.y;

	object_matrix->c.x = final_angle.c.x;
	object_matrix->c.y = final_angle.c.y;
	object_matrix->c.z = final_angle.c.z;
	object_matrix->c.q = object_pos.z;

	object_matrix->d.x = final_angle.d.x;
	object_matrix->d.y = final_angle.d.y;
	object_matrix->d.z = final_angle.d.z;
	object_matrix->d.q = 1;
}

void projection_matrix(matx3d* projection_matx, camera camera, int type) {   // Possibly move this inside the camera struct
	projection_matx->a.x = 1;
	projection_matx->a.y = 0;
	projection_matx->a.z = 0;
	projection_matx->a.q = 0;

	projection_matx->b.x = 0;
	projection_matx->b.y = 1;
	projection_matx->b.z = 0;
	projection_matx->b.q = 0;

	projection_matx->c.x = 0;
	projection_matx->c.y = 0;
	projection_matx->c.z = 1;
	projection_matx->c.q = 0;

	projection_matx->d.x = 0;
	projection_matx->d.y = 0;
	projection_matx->d.z = 0;
	projection_matx->d.q = 1;
}

int full_convert_obj(SDL_Renderer* renderer, object_info object, camera camera, std::vector<float>& depth_buffer, int half_screen_x, int half_screen_y) {
	// Check validity
	if (!object.get_zipped()) { return 1; } // Unzipped mesh error

	// Make matrices
	matx3d cam_world = camera.matrix;
	matx3d obj_world;
	matx3d cam_proj;

	camera_matrix(&cam_world, camera.camera_pos);
	object_matrix(&obj_world, object.model_org, object.model_rot);
	projection_matrix(&cam_proj, camera, 0);
	get_inverse(&cam_world);
	matx3d cam_obj = (cam_proj * cam_world) * obj_world;

	tri3d converted_tris[4];
	short int num_tris = 1;
	short int num_tris2 = 1;
	vec3d depth;
	vec3d converted;
	float htan_angle = tan(camera.frustrum.horfov / 2);
	float vtan_angle = tan(camera.frustrum.verfov / 2);

	// foreach
	for (size_t i = object.model_mesh.size() - 1; i != -1; i--)
	{
		// Transform
		converted_tris[0].p1 = cam_obj * object.model_mesh[i].p1;
		converted_tris[0].p2 = cam_obj * object.model_mesh[i].p2;
		converted_tris[0].p3 = cam_obj * object.model_mesh[i].p3;


		// Persp transform
		converted = converted_tris[0].p1;
		converted.x = converted.x / (htan_angle * converted.z);
		converted.y = converted.y / (vtan_angle * converted.z);
		converted.z -= camera.frustrum.near;
		converted.z /= camera.frustrum.far;
		depth.x = converted.z;
		converted.q = 1;
		converted_tris[0].p1 = converted;

		converted = converted_tris[0].p2;
		converted.x = converted.x / (htan_angle * converted.z);
		converted.y = converted.y / (vtan_angle * converted.z);
		converted.z -= camera.frustrum.near;
		converted.z /= camera.frustrum.far;
		depth.y = converted.z;
		converted.q = 1;
		converted_tris[0].p2 = converted;

		converted = converted_tris[0].p3;
		converted.x = converted.x / (htan_angle * converted.z); // add in resolution term
		converted.y = converted.y / (vtan_angle * converted.z);
		converted.z -= camera.frustrum.near;
		converted.z /= camera.frustrum.far;
		depth.z = converted.z;
		converted.q = 1;
		converted_tris[0].p3 = converted;


		// Clip and store tris so that the tris are packed together
		if (!object.tags.noclipping) {
			num_tris = clip_far(&converted_tris[0], &converted_tris[1], depth);
			if (num_tris == 2) {
				num_tris2 = clip_near(&converted_tris[0], &converted_tris[2], depth);
				if (num_tris2 == 2) {
					num_tris += clip_near(&converted_tris[1], &converted_tris[3], depth);
				}
				else {
					num_tris += clip_near(&converted_tris[1], &converted_tris[2], depth);
				}
			}
			else if (num_tris == 1) {
				num_tris = clip_near(&converted_tris[0], &converted_tris[1], depth);
			}
		}
		// Draw to screen
		SDL_SetRenderDrawColor(renderer, 255 - i * 255 / object.model_mesh.size(), i * 255 / object.model_mesh.size(), i * 255 / 2 / object.model_mesh.size(), 255); // Move back
		switch (num_tris) { //Support for 2 cutting planes, giving 4 tris, abusing switch case as a goto function
		case 4:
			converted_tris[3].p1.z = 1 / converted_tris[3].p1.z;
			converted_tris[3].p2.z = 1 / converted_tris[3].p2.z;
			converted_tris[3].p3.z = 1 / converted_tris[3].p3.z;
			draw_tri(renderer, depth_buffer, converted_tris[3], half_screen_x, half_screen_y);
		case 3:
			converted_tris[2].p1.z = 1 / converted_tris[2].p1.z;
			converted_tris[2].p2.z = 1 / converted_tris[2].p2.z;
			converted_tris[2].p3.z = 1 / converted_tris[2].p3.z;
			draw_tri(renderer, depth_buffer, converted_tris[2], half_screen_x, half_screen_y);
		case 2:
			converted_tris[1].p1.z = 1 / converted_tris[1].p1.z;
			converted_tris[1].p2.z = 1 / converted_tris[1].p2.z;
			converted_tris[1].p3.z = 1 / converted_tris[1].p3.z;
			draw_tri(renderer, depth_buffer, converted_tris[1], half_screen_x, half_screen_y);
		case 1:
			converted_tris[0].p1.z = 1 / converted_tris[0].p1.z;
			converted_tris[0].p2.z = 1 / converted_tris[0].p2.z;
			converted_tris[0].p3.z = 1 / converted_tris[0].p3.z;
			draw_tri(renderer, depth_buffer, converted_tris[0], half_screen_x, half_screen_y);
		default:
			break;
		}
	}
	return 0;
}

// Finite difference normal method - fast
clip_tags::OutCode get_code(vec3d input, float limit) {
	clip_tags::OutCode code = clip_tags::INSIDE;
	if (input.z < limit)           // to the back of clip window  // Make this equal 0
		code |= clip_tags::NEAR;
	else if (input.z > 1)      // to the front of clip window
		code |= clip_tags::FAR;
	return code;
}
int clip_far(tri3d* converted_tri1, tri3d* converted_tri2, vec3d depth) {  // Return num of active tris
// Converted tri 1 holds input
	tri3d tmptri;
	tri3d tmptri2;

	tmptri.p1.x = converted_tri1->p1.x;
	tmptri.p1.y = converted_tri1->p1.y;
	tmptri.p1.z = converted_tri1->p1.z;

	tmptri.p2.x = converted_tri1->p2.x;
	tmptri.p2.y = converted_tri1->p2.y;
	tmptri.p2.z = converted_tri1->p2.z;

	tmptri.p3.x = converted_tri1->p3.x;
	tmptri.p3.y = converted_tri1->p3.y;
	tmptri.p3.z = converted_tri1->p3.z;

	// Setup
	clip_tags::OutCode outcode1 = get_code(tmptri.p1, 0.1f);
	clip_tags::OutCode outcode2 = get_code(tmptri.p2, 0.1f);
	clip_tags::OutCode outcode3 = get_code(tmptri.p3, 0.1f);

	int farsum = (outcode1 & clip_tags::FAR) + (outcode2 & clip_tags::FAR) + (outcode3 & clip_tags::FAR);

	if (farsum == 96) return 0; // Far clip
	if (farsum == 0) return 1; // No clip

	int split_tri = 1;

	// Clip far
	switch (farsum)
	{
	case 32:
		split_tri = 2;
		if (outcode1 & clip_tags::FAR) {
			converted_tri2->p1 = vector_clip(1.0f, &tmptri.p3, &tmptri.p1);
			converted_tri2->p2 = vector_clip(1.0f, &tmptri.p2, &tmptri.p1);
			converted_tri2->p3.x = tmptri.p3.x;
			converted_tri2->p3.y = tmptri.p3.y;
			converted_tri2->p3.z = tmptri.p3.z;

			converted_tri1->p1.x = converted_tri2->p2.x;
			converted_tri1->p1.y = converted_tri2->p2.y;
			converted_tri1->p1.z = converted_tri2->p2.z;

			converted_tri1->p2.x = tmptri.p2.x;
			converted_tri1->p2.y = tmptri.p2.y;
			converted_tri1->p2.z = tmptri.p2.z;

			converted_tri1->p3.x = tmptri.p3.x;
			converted_tri1->p3.y = tmptri.p3.y;
			converted_tri1->p3.z = tmptri.p3.z;
		}
		else if (outcode2 & clip_tags::FAR) {
			converted_tri2->p1 = vector_clip(1.0f, &tmptri.p1, &tmptri.p2);
			converted_tri2->p2 = vector_clip(1.0f, &tmptri.p3, &tmptri.p2);
			converted_tri2->p3.x = tmptri.p3.x;
			converted_tri2->p3.y = tmptri.p3.y;
			converted_tri2->p3.z = tmptri.p3.z;

			converted_tri1->p1.x = tmptri.p1.x;
			converted_tri1->p1.y = tmptri.p1.y;
			converted_tri1->p1.z = tmptri.p1.z;

			converted_tri1->p2.x = converted_tri2->p1.x;
			converted_tri1->p2.y = converted_tri2->p1.y;
			converted_tri1->p2.z = converted_tri2->p1.z;

			converted_tri1->p3.x = tmptri.p3.x;
			converted_tri1->p3.y = tmptri.p3.y;
			converted_tri1->p3.z = tmptri.p3.z;
		}
		else {
			converted_tri2->p2 = vector_clip(1.0f, &tmptri.p2, &tmptri.p3);
			converted_tri2->p3 = vector_clip(1.0f, &tmptri.p1, &tmptri.p3);
			converted_tri2->p1.x = tmptri.p1.x;
			converted_tri2->p1.y = tmptri.p1.y;
			converted_tri2->p1.z = tmptri.p1.z;

			converted_tri1->p1.x = tmptri.p1.x;
			converted_tri1->p1.y = tmptri.p1.y;
			converted_tri1->p1.z = tmptri.p1.z;

			converted_tri1->p2.x = tmptri.p2.x;
			converted_tri1->p2.y = tmptri.p2.y;
			converted_tri1->p2.z = tmptri.p2.z;

			converted_tri1->p3.x = converted_tri2->p2.x;
			converted_tri1->p3.y = converted_tri2->p2.y;
			converted_tri1->p3.z = converted_tri2->p2.z;

		}
		break;
	case 64:
		if (outcode1 & clip_tags::FAR && outcode2 & clip_tags::FAR) {
			converted_tri1->p1 = vector_clip(1.0f, &tmptri.p3, &tmptri.p1);
			converted_tri1->p2 = vector_clip(1.0f, &tmptri.p3, &tmptri.p2);

			converted_tri1->p3.x = tmptri.p3.x;
			converted_tri1->p3.y = tmptri.p3.y;
			converted_tri1->p3.z = tmptri.p3.z;
		}
		else if (outcode1 & clip_tags::FAR && outcode3 & clip_tags::FAR) {
			converted_tri1->p1 = vector_clip(1.0f, &tmptri.p2, &tmptri.p1);

			converted_tri1->p2.x = tmptri.p2.x;
			converted_tri1->p2.y = tmptri.p2.y;
			converted_tri1->p2.z = tmptri.p2.z;

			converted_tri1->p3 = vector_clip(1.0f, &tmptri.p2, &tmptri.p3);
		}
		else {
			converted_tri1->p1.x = tmptri.p1.x;
			converted_tri1->p1.y = tmptri.p1.y;
			converted_tri1->p1.z = tmptri.p1.z;

			converted_tri1->p2 = vector_clip(1.0f, &tmptri.p1, &tmptri.p2);
			converted_tri1->p3 = vector_clip(1.0f, &tmptri.p1, &tmptri.p3);

		}
		break;

	default:
		break;
	}

	return split_tri;
}

int clip_near(tri3d* converted_tri1, tri3d* converted_tri2, vec3d depth) {  // Return num of active tris
// Converted tri 1 holds input
	tri3d tmptri;
	tri3d tmptri2;

	tmptri.p1.x = converted_tri1->p1.x;
	tmptri.p1.y = converted_tri1->p1.y;
	tmptri.p1.z = converted_tri1->p1.z;

	tmptri.p2.x = converted_tri1->p2.x;
	tmptri.p2.y = converted_tri1->p2.y;
	tmptri.p2.z = converted_tri1->p2.z;

	tmptri.p3.x = converted_tri1->p3.x;
	tmptri.p3.y = converted_tri1->p3.y;
	tmptri.p3.z = converted_tri1->p3.z;

	// Setup
	clip_tags::OutCode outcode1 = get_code(tmptri.p1, 0.1f);
	clip_tags::OutCode outcode2 = get_code(tmptri.p2, 0.1f);
	clip_tags::OutCode outcode3 = get_code(tmptri.p3, 0.1f);

	int nearsum = (outcode1 & clip_tags::NEAR) + (outcode2 & clip_tags::NEAR) + (outcode3 & clip_tags::NEAR);

	if (nearsum == 48) return 0; // Near clip
	if (nearsum == 0) return 1; // No clip

	int split_tri = 1;

	switch (nearsum) {
	case 16:
		split_tri = 2;
		if (outcode1 & clip_tags::NEAR) {
			converted_tri2->p1 = vector_clip(0.0f, &tmptri.p3, &tmptri.p1);
			converted_tri2->p2 = vector_clip(0.0f, &tmptri.p2, &tmptri.p1);
			converted_tri2->p3.x = tmptri.p3.x;
			converted_tri2->p3.y = tmptri.p3.y;
			converted_tri2->p3.z = tmptri.p3.z;

			converted_tri1->p1.x = converted_tri2->p2.x;
			converted_tri1->p1.y = converted_tri2->p2.y;
			converted_tri1->p1.z = converted_tri2->p2.z;

			converted_tri1->p2.x = tmptri.p2.x;
			converted_tri1->p2.y = tmptri.p2.y;
			converted_tri1->p2.z = tmptri.p2.z;

			converted_tri1->p3.x = tmptri.p3.x;
			converted_tri1->p3.y = tmptri.p3.y;
			converted_tri1->p3.z = tmptri.p3.z;
		}
		else if (outcode2 & clip_tags::NEAR) {
			converted_tri2->p1 = vector_clip(0.0f, &tmptri.p1, &tmptri.p2);
			converted_tri2->p2 = vector_clip(0.0f, &tmptri.p3, &tmptri.p2);
			converted_tri2->p3.x = tmptri.p3.x;
			converted_tri2->p3.y = tmptri.p3.y;
			converted_tri2->p3.z = tmptri.p3.z;

			converted_tri1->p1.x = tmptri.p1.x;
			converted_tri1->p1.y = tmptri.p1.y;
			converted_tri1->p1.z = tmptri.p1.z;

			converted_tri1->p2.x = converted_tri2->p1.x;
			converted_tri1->p2.y = converted_tri2->p1.y;
			converted_tri1->p2.z = converted_tri2->p1.z;

			converted_tri1->p3.x = tmptri.p3.x;
			converted_tri1->p3.y = tmptri.p3.y;
			converted_tri1->p3.z = tmptri.p3.z;
		}
		else {
			converted_tri2->p2 = vector_clip(0.0f, &tmptri.p2, &tmptri.p3);
			converted_tri2->p3 = vector_clip(0.0f, &tmptri.p1, &tmptri.p3);
			converted_tri2->p1.x = tmptri.p1.x;
			converted_tri2->p1.y = tmptri.p1.y;
			converted_tri2->p1.z = tmptri.p1.z;

			converted_tri1->p1.x = tmptri.p1.x;
			converted_tri1->p1.y = tmptri.p1.y;
			converted_tri1->p1.z = tmptri.p1.z;

			converted_tri1->p2.x = tmptri.p2.x;
			converted_tri1->p2.y = tmptri.p2.y;
			converted_tri1->p2.z = tmptri.p2.z;

			converted_tri1->p3.x = converted_tri2->p2.x;
			converted_tri1->p3.y = converted_tri2->p2.y;
			converted_tri1->p3.z = converted_tri2->p2.z;

		}
		break;
	case 32:
		if (outcode1 & clip_tags::NEAR && outcode2 & clip_tags::NEAR) {
			converted_tri1->p1 = vector_clip(0.0f, &tmptri.p3, &tmptri.p1);
			converted_tri1->p2 = vector_clip(0.0f, &tmptri.p3, &tmptri.p2);

			converted_tri1->p3.x = tmptri.p3.x;
			converted_tri1->p3.y = tmptri.p3.y;
			converted_tri1->p3.z = tmptri.p3.z;
		}
		else if (outcode1 & clip_tags::NEAR && outcode3 & clip_tags::NEAR) {
			converted_tri1->p1 = vector_clip(0.0f, &tmptri.p2, &tmptri.p1);

			converted_tri1->p2.x = tmptri.p2.x;
			converted_tri1->p2.y = tmptri.p2.y;
			converted_tri1->p2.z = tmptri.p2.z;

			converted_tri1->p3 = vector_clip(0.0f, &tmptri.p2, &tmptri.p3);
		}
		else {
			converted_tri1->p1.x = tmptri.p1.x;
			converted_tri1->p1.y = tmptri.p1.y;
			converted_tri1->p1.z = tmptri.p1.z;

			converted_tri1->p2 = vector_clip(0.0f, &tmptri.p1, &tmptri.p2);
			converted_tri1->p3 = vector_clip(0.0f, &tmptri.p1, &tmptri.p3);
		}
		break;

	default:
		break;
	}
	return split_tri;
}