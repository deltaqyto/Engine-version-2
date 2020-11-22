#include "graphics.h"

int to_point(SDL_Point* out, vec3d point) {
	out->x = (int)floor(point.x);
	out->y = (int)floor(point.y);
	return 0;
}

int draw_tri(std::vector<float>& color_buffer, std::vector<float>& buffer_out, tri3d triangle, int half_x_width, int half_y_width, int depth_test, light light, texture* texture) { // depthmap is a linear array. Buffer out is pointing to the first value
	tri3d scaled_tri = triangle;
	scaled_tri.p1.x = scaled_tri.p1.x * half_x_width + half_x_width;
	scaled_tri.p2.x = scaled_tri.p2.x * half_x_width + half_x_width;
	scaled_tri.p3.x = scaled_tri.p3.x * half_x_width + half_x_width;

	scaled_tri.p1.y = scaled_tri.p1.y * half_y_width + half_y_width;
	scaled_tri.p2.y = scaled_tri.p2.y * half_y_width + half_y_width;
	scaled_tri.p3.y = scaled_tri.p3.y * half_y_width + half_y_width;

	int x_offset = (int)std::min(std::min(floor(scaled_tri.p1.x), floor(scaled_tri.p2.x)), floor(scaled_tri.p3.x));
	int y_offset = (int)std::min(std::min(floor(scaled_tri.p1.y), floor(scaled_tri.p2.y)), floor(scaled_tri.p3.y));
	int x_width = (int)std::max(std::max(ceil(scaled_tri.p1.x), ceil(scaled_tri.p2.x)), ceil(scaled_tri.p3.x)) - x_offset;
	int y_width = (int)std::max(std::max(ceil(scaled_tri.p1.y), ceil(scaled_tri.p2.y)), ceil(scaled_tri.p3.y)) - y_offset;
	vector2d ac;
	ac.x = scaled_tri.p3.x - scaled_tri.p1.x;
	ac.y = scaled_tri.p3.y - scaled_tri.p1.y;
	vector2d ab;
	ab.x = scaled_tri.p2.x - scaled_tri.p1.x;
	ab.y = scaled_tri.p2.y - scaled_tri.p1.y;
	float cc = vect_dot_vect(ac, ac);
	float cb = vect_dot_vect(ac, ab);
	float cp;
	float bb = vect_dot_vect(ab, ab);
	float bp;
	float invDenom = 1 / (cc * bb - pow(cb, 2));
	float u;
	float v;
	int initx = x_width;
	int inity = y_width;
	int trimx = x_offset;
	int trimy = y_offset;

	if (x_offset + x_width > half_x_width * 2) {
		x_width = half_x_width * 2 - x_offset;
	}
	if (y_offset + y_width > half_y_width * 2) {
		y_width = half_y_width * 2 - y_offset;
	}
	if (x_width < 0) {
		x_width = 0;
	}
	if (y_width < 0) {
		y_width = 0;
	}
	if (x_offset < 0) {
		// half_x_width = (int)(half_x_width * 2 + x) / 2;
		x_offset = 0;
	}
	if (y_offset < 0) {
		// half_y_width = (int)(half_y_width * 2 + y) / 2;
		y_offset = 0;
	}
	if (x_width > 640) {
		x_width = 640;
	}
	if (y_width > 480) {
		y_width = 480;
	}

	trimx = x_offset - trimx;
	trimy = y_offset - trimy;

	// Debug print
	//printf("%g\n", x);
	//printf("%g\n", camera_pos.z);
	//printf("-----------------\n\n");
	// For each pixel draw color * check value

	float x_dif = x_offset - scaled_tri.p1.x;
	float y_dif = y_offset - scaled_tri.p1.y;
	int full_x_width = half_x_width * 2;
	int full_y_width = half_y_width * 2;
	float twoarea = (ab.x * ac.y - ab.y * ac.x);
	float w = 0;
	int tex_x;
	int tex_y;
	int texture_x;
	int texture_y;
	float texture_weights_x[3];
	float texture_weights_y[3];

	if (texture != nullptr) {
		tex_x = texture->width;
		tex_y = texture->height;

		texture_weights_x[0] = (triangle.tx.x / triangle.p1.q) * tex_x;
		texture_weights_x[1] = (triangle.tx.y / triangle.p2.q) * tex_x;
		texture_weights_x[2] = (triangle.tx.q / triangle.p3.q) * tex_x;

		texture_weights_y[0] = (triangle.ty.x / triangle.p1.q) * tex_y;
		texture_weights_y[1] = (triangle.ty.y / triangle.p2.q) * tex_y;
		texture_weights_y[2] = (triangle.ty.q / triangle.p3.q) * tex_y;

		texture_x = 0;
		texture_y = 0;
	}
	float barycentric_depth_weights[3] = { scaled_tri.p1.z, scaled_tri.p2.z, scaled_tri.p3.z };
	float depth_value = 0;

	bool skip;
	bool drawing;
	color color = triangle.cp1;

	for (size_t current_y = 0; current_y < y_width; current_y++) {
		skip = false;
		drawing = false;
		for (size_t current_x = 0; current_x < x_width; current_x++) {
			if (skip) {
				break;
			}
			vector2d ap;
			ap.x = current_x + x_dif;
			ap.y = current_y + y_dif;
			cp = vect_dot_vect(ac, ap);
			bp = vect_dot_vect(ab, ap);
			u = (bb * cp - cb * bp) * invDenom;
			v = (cc * bp - cb * cp) * invDenom;
			// b1 = u, b2 = v
			w = abs(1 - u - v);
			depth_value = (w * barycentric_depth_weights[0] + v * barycentric_depth_weights[1] + u * barycentric_depth_weights[2]);
			if (u >= 0 && v >= 0 && u + v < 1) {
				drawing = true;	   //https://www.scratchapixel.com/lessons/3d-basic-rendering/rasterization-practical-implementation/perspective-correct-interpolation-vertex-attributes
				if (buffer_out[(y_offset + current_y) * full_x_width + (x_offset + current_x)] * depth_test < (0.0625 + depth_value)) {
					if (texture != nullptr) {
						texture_x = std::min(int((w * texture_weights_x[0] + v * texture_weights_x[1] + u * texture_weights_x[2]) / depth_value), tex_x - 1);
						texture_y = std::min(int((w * texture_weights_y[0] + v * texture_weights_y[1] + u * texture_weights_y[2]) / depth_value), tex_y - 1);
						color = texture->get_pixel(texture_x, texture_y);
					}
					color_buffer[3 * ((y_offset + current_y) * full_x_width + (x_offset + current_x))] = color.r;
					color_buffer[3 * ((y_offset + current_y) * full_x_width + (x_offset + current_x)) + 1] = color.g;
					color_buffer[3 * ((y_offset + current_y) * full_x_width + (x_offset + current_x)) + 2] = color.b;
					buffer_out[(y_offset + current_y) * full_x_width + (x_offset + current_x)] = depth_value;

				}
			}
			else {
				skip = true * drawing;
			}

		}
	}
	return 0;
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

int full_convert_obj(std::vector<float>& color_buffer, object_info object, camera camera, std::vector<float>& depth_buffer, int half_screen_x, int half_screen_y, light light) {
	if (!object.tags.show_object) return 0; // Object is hidden
	
											// Check validity
	if (!object.get_zipped()) return 1; // Unzipped mesh error

	// Make matrices
	matx3d cam_world = camera.matrix;
	matx3d obj_world;

	camera_matrix(&cam_world, camera.camera_pos);
	object_matrix(&obj_world, object.model_org, object.model_rot);
	get_inverse(&cam_world);
	matx3d cam_obj = (cam_world) * obj_world;

	tri3d converted_tris[4];
	short int num_tris = 1;
	short int num_tris2 = 1;
	vec3d depth;
	vec3d converted;
	color color;
	float htan_angle = tan(camera.frustrum.horfov / 2);
	float vtan_angle = tan(camera.frustrum.verfov / 2);
	texture* bound_texture = nullptr;

	// foreach
	for (size_t i = object.model_mesh.size() - 1; i != -1; i--)
	{
		// Transform
		converted_tris[0].p1 = cam_obj * object.model_mesh[i].p1;
		converted_tris[0].p2 = cam_obj * object.model_mesh[i].p2;
		converted_tris[0].p3 = cam_obj * object.model_mesh[i].p3;
		converted_tris[0].tx = object.model_mesh[i].tx;
		converted_tris[0].ty = object.model_mesh[i].ty;

		get_normal_from_tri(&converted_tris[0]); // Perform back face culling
		if (object.tags.bfc && vec3d_dot(converted_tris->normal, 
			{ converted_tris[0].p1.x - camera.camera_vect.x,
			  converted_tris[0].p1.y - camera.camera_vect.y,
			  converted_tris[0].p1.z - camera.camera_vect.z, 1 }) >= 0) {
			continue;
		}



		// Clip and store tris so that the tris are packed together
		if (!object.tags.noclipping) {
			num_tris = clip_far(&converted_tris[0], &converted_tris[1], camera.frustrum.far);
			if (num_tris == 2) {
				num_tris2 = clip_near(&converted_tris[0], &converted_tris[2], camera.frustrum.near);
				if (num_tris2 == 2) {
					num_tris += clip_near(&converted_tris[1], &converted_tris[3], camera.frustrum.near);
				}
				else {
					num_tris += clip_near(&converted_tris[1], &converted_tris[2], camera.frustrum.near);
				}
			}
			else if (num_tris == 1) {
				num_tris = clip_near(&converted_tris[0], &converted_tris[1], camera.frustrum.near);
			}
		}
		if (!object.has_texture || !object.tags.use_texture) {
			// Set color with basic flat shading
			float brightness = 1;
			if (!object.tags.fullbright) {
				brightness = std::min(light.ambient + std::max(vec3d_dot(converted_tris[0].normal, light.direction), 0.0f), 1.0f);
			}
			if (object.tags.debug_color) {
				color = { int(255 * float(i) / object.tris.size()), int(255 - 255 * float(i) / object.tris.size()),int(float(i) / (2 * object.tris.size())), 255 };
			}
			else {
				color = object.color;
			}
			
			converted_tris[0].cp1 = { int(color.r * brightness), int(color.g * brightness),int(color.b * brightness), 255 };
		} else {
			bound_texture = object.textures[object.current_texture];
		}
		// Draw to screen
		switch (num_tris) { //Support for 2 cutting planes, giving 4 tris, abusing switch case as a goto function
		case 4:
			//SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);

			// Persp transform
			converted = converted_tris[3].p1;
			converted.x = converted.x / (htan_angle * converted.z);
			converted.y = converted.y / (vtan_angle * converted.z);
			converted.z /= camera.frustrum.far;
			converted.q = converted.z;
			converted_tris[3].p1 = converted;

			converted = converted_tris[3].p2;
			converted.x = converted.x / (htan_angle * converted.z);
			converted.y = converted.y / (vtan_angle * converted.z);
			converted.z /= camera.frustrum.far;
			converted.q = converted.z;
			converted_tris[3].p2 = converted;

			converted = converted_tris[3].p3;
			converted.x = converted.x / (htan_angle * converted.z); // add in resolution term
			converted.y = converted.y / (vtan_angle * converted.z);
			converted.z /= camera.frustrum.far;
			converted.q = converted.z;
			converted_tris[3].p3 = converted;

			converted_tris[3].p1.z = 1 / converted_tris[3].p1.z;
			converted_tris[3].p2.z = 1 / converted_tris[3].p2.z;
			converted_tris[3].p3.z = 1 / converted_tris[3].p3.z;

			draw_tri(color_buffer, depth_buffer, converted_tris[3], half_screen_x, half_screen_y, (int)object.tags.depth_test, light, bound_texture);
		case 3:
			//SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255);

			// Persp transform
			converted = converted_tris[2].p1;
			converted.x = converted.x / (htan_angle * converted.z);
			converted.y = converted.y / (vtan_angle * converted.z);
			converted.z /= camera.frustrum.far;
			converted.q = converted.z;
			converted_tris[2].p1 = converted;

			converted = converted_tris[2].p2;
			converted.x = converted.x / (htan_angle * converted.z);
			converted.y = converted.y / (vtan_angle * converted.z);
			converted.z /= camera.frustrum.far;
			converted.q = converted.z;
			converted_tris[2].p2 = converted;

			converted = converted_tris[2].p3;
			converted.x = converted.x / (htan_angle * converted.z); // add in resolution term
			converted.y = converted.y / (vtan_angle * converted.z);
			converted.z /= camera.frustrum.far;
			converted.q = converted.z;
			converted_tris[2].p3 = converted;

			converted_tris[2].p1.z = 1 / converted_tris[2].p1.z;
			converted_tris[2].p2.z = 1 / converted_tris[2].p2.z;
			converted_tris[2].p3.z = 1 / converted_tris[2].p3.z;

			draw_tri(color_buffer, depth_buffer, converted_tris[2], half_screen_x, half_screen_y, (int)object.tags.depth_test, light, bound_texture);
		case 2:
			//SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

			// Persp transform
			converted = converted_tris[1].p1;
			converted.x = converted.x / (htan_angle * converted.z);
			converted.y = converted.y / (vtan_angle * converted.z);
			converted.z /= camera.frustrum.far;
			converted.q = converted.z;
			converted_tris[1].p1 = converted;

			converted = converted_tris[1].p2;
			converted.x = converted.x / (htan_angle * converted.z);
			converted.y = converted.y / (vtan_angle * converted.z);
			converted.z /= camera.frustrum.far;
			converted.q = converted.z;
			converted_tris[1].p2 = converted;

			converted = converted_tris[1].p3;
			converted.x = converted.x / (htan_angle * converted.z); // add in resolution term
			converted.y = converted.y / (vtan_angle * converted.z);
			converted.z /= camera.frustrum.far;
			converted.q = converted.z;
			converted_tris[1].p3 = converted;

			converted_tris[1].p1.z = 1 / converted_tris[1].p1.z;
			converted_tris[1].p2.z = 1 / converted_tris[1].p2.z;
			converted_tris[1].p3.z = 1 / converted_tris[1].p3.z;

			draw_tri(color_buffer, depth_buffer, converted_tris[1], half_screen_x, half_screen_y, (int)object.tags.depth_test, light, bound_texture);
		case 1:
			//SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);

			// Persp transform
			converted = converted_tris[0].p1;
			converted.x = converted.x / (htan_angle * converted.z);
			converted.y = converted.y / (vtan_angle * converted.z);
			converted.z /= camera.frustrum.far;
			converted.q = converted.z;
			converted_tris[0].p1 = converted;

			converted = converted_tris[0].p2;
			converted.x = converted.x / (htan_angle * converted.z);
			converted.y = converted.y / (vtan_angle * converted.z);
			converted.z /= camera.frustrum.far;
			converted.q = converted.z;
			converted_tris[0].p2 = converted;

			converted = converted_tris[0].p3;
			converted.x = converted.x / (htan_angle * converted.z); // add in resolution term
			converted.y = converted.y / (vtan_angle * converted.z);
			converted.z /= camera.frustrum.far;
			converted.q = converted.z;
			converted_tris[0].p3 = converted;

			converted_tris[0].p1.z = 1 / converted_tris[0].p1.z;
			converted_tris[0].p2.z = 1 / converted_tris[0].p2.z;
			converted_tris[0].p3.z = 1 / converted_tris[0].p3.z;
 
			draw_tri(color_buffer, depth_buffer, converted_tris[0], half_screen_x, half_screen_y, (int)object.tags.depth_test, light, bound_texture);
		default:																										 
			break;
		}
	}
	return 0;
}

clip_tags::OutCode get_near(vec3d input, float limit) {
	if (input.z < limit)          
		return clip_tags::NEAR;
	return clip_tags::INSIDE;
}

clip_tags::OutCode get_far(vec3d input, float limit) {
	if (input.z > limit)
		return clip_tags::FAR;
	return clip_tags::INSIDE;
}

int clip_far(tri3d* converted_tri1, tri3d* converted_tri2, float far_depth) {  
	// Return num of active tris
	// Converted tri 1 holds input
	tri3d tmptri;
	float tof;

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
	clip_tags::OutCode outcode1 = get_far(tmptri.p1, far_depth);
	clip_tags::OutCode outcode2 = get_far(tmptri.p2, far_depth);
	clip_tags::OutCode outcode3 = get_far(tmptri.p3, far_depth);

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
			converted_tri2->p1 = vector_clip(far_depth, &tmptri.p3, &tmptri.p1, &tof);
			converted_tri2->tx.x = converted_tri1->tx.q + (converted_tri1->tx.x - converted_tri1->tx.q) * tof;
			converted_tri2->ty.x = converted_tri1->ty.q + (converted_tri1->ty.x - converted_tri1->ty.q) * tof;
			converted_tri2->p2 = vector_clip(far_depth, &tmptri.p2, &tmptri.p1, &tof);
			converted_tri2->tx.y = converted_tri1->tx.y + (converted_tri1->tx.x - converted_tri1->tx.y) * tof;
			converted_tri2->ty.y = converted_tri1->ty.y + (converted_tri1->ty.x - converted_tri1->ty.y) * tof;
			converted_tri2->p3.x = tmptri.p3.x;
			converted_tri2->p3.y = tmptri.p3.y;
			converted_tri2->p3.z = tmptri.p3.z;
			converted_tri2->tx.q = converted_tri1->tx.q;
			converted_tri2->ty.q = converted_tri1->ty.q;

			converted_tri1->p1.x = converted_tri2->p2.x;
			converted_tri1->p1.y = converted_tri2->p2.y;
			converted_tri1->p1.z = converted_tri2->p2.z;
			converted_tri1->tx.x = converted_tri2->tx.y;
			converted_tri1->ty.x = converted_tri2->ty.y;
		}
		else if (outcode2 & clip_tags::FAR) {
			converted_tri2->p1 = vector_clip(far_depth, &tmptri.p1, &tmptri.p2, &tof);
			converted_tri2->tx.x = converted_tri1->tx.x + (converted_tri1->tx.y - converted_tri1->tx.x) * tof;
			converted_tri2->ty.x = converted_tri1->ty.x + (converted_tri1->ty.y - converted_tri1->ty.x) * tof;
			converted_tri2->p2 = vector_clip(far_depth, &tmptri.p3, &tmptri.p2, &tof);
			converted_tri2->tx.y = converted_tri1->tx.q + (converted_tri1->tx.y - converted_tri1->tx.q) * tof;
			converted_tri2->ty.y = converted_tri1->ty.q + (converted_tri1->ty.y - converted_tri1->ty.q) * tof;
			converted_tri2->p3.x = tmptri.p3.x;
			converted_tri2->p3.y = tmptri.p3.y;
			converted_tri2->p3.z = tmptri.p3.z;
			converted_tri2->tx.q = converted_tri1->tx.q;
			converted_tri2->ty.q = converted_tri1->ty.q;

			converted_tri1->p2.x = converted_tri2->p1.x;
			converted_tri1->p2.y = converted_tri2->p1.y;
			converted_tri1->p2.z = converted_tri2->p1.z;
			converted_tri1->tx.y = converted_tri2->tx.x;
			converted_tri1->ty.y = converted_tri2->ty.x;
		}
		else {
			converted_tri2->p2 = vector_clip(far_depth, &tmptri.p2, &tmptri.p3, &tof);
			converted_tri2->tx.y = converted_tri1->tx.y + (converted_tri1->tx.q - converted_tri1->tx.y) * tof;
			converted_tri2->ty.y = converted_tri1->ty.y + (converted_tri1->ty.q - converted_tri1->ty.y) * tof;
			converted_tri2->p3 = vector_clip(far_depth, &tmptri.p1, &tmptri.p3, &tof);
			converted_tri2->tx.q = converted_tri1->tx.x + (converted_tri1->tx.q - converted_tri1->tx.x) * tof;
			converted_tri2->ty.q = converted_tri1->ty.x + (converted_tri1->ty.q - converted_tri1->ty.x) * tof;
			converted_tri2->p1.x = tmptri.p1.x;
			converted_tri2->p1.y = tmptri.p1.y;
			converted_tri2->p1.z = tmptri.p1.z;
			converted_tri2->tx.x = converted_tri1->tx.x;
			converted_tri2->ty.x = converted_tri1->ty.x;

			converted_tri1->p3.x = converted_tri2->p2.x;
			converted_tri1->p3.y = converted_tri2->p2.y;
			converted_tri1->p3.z = converted_tri2->p2.z;
			converted_tri1->tx.q = converted_tri2->tx.y;
			converted_tri1->ty.q = converted_tri2->ty.y;
		}
		break;
	case 64:
		if (outcode1 & clip_tags::FAR && outcode2 & clip_tags::FAR) {
			converted_tri1->p1 = vector_clip(far_depth, &tmptri.p3, &tmptri.p1, &tof);
			converted_tri1->tx.x = converted_tri1->tx.q + (converted_tri1->tx.x - converted_tri1->tx.q) * tof;
			converted_tri1->ty.x = converted_tri1->ty.q + (converted_tri1->ty.x - converted_tri1->ty.q) * tof;
			converted_tri1->p2 = vector_clip(far_depth, &tmptri.p3, &tmptri.p2, &tof);
			converted_tri1->tx.y = converted_tri1->tx.q + (converted_tri1->tx.y - converted_tri1->tx.q) * tof;
			converted_tri1->ty.y = converted_tri1->ty.q + (converted_tri1->ty.y - converted_tri1->ty.q) * tof;
		}
		else if (outcode1 & clip_tags::FAR && outcode3 & clip_tags::FAR) {
			converted_tri1->p1 = vector_clip(far_depth, &tmptri.p2, &tmptri.p1, &tof);
			converted_tri1->tx.x = converted_tri1->tx.y + (converted_tri1->tx.x - converted_tri1->tx.y) * tof;
			converted_tri1->ty.x = converted_tri1->ty.y + (converted_tri1->ty.x - converted_tri1->ty.y) * tof;
			converted_tri1->p3 = vector_clip(far_depth, &tmptri.p2, &tmptri.p3, &tof);
			converted_tri1->tx.q = converted_tri1->tx.y + (converted_tri1->tx.q - converted_tri1->tx.y) * tof;
			converted_tri1->ty.q = converted_tri1->ty.y + (converted_tri1->ty.q - converted_tri1->ty.y) * tof;
		}
		else {
			converted_tri1->p2 = vector_clip(far_depth, &tmptri.p1, &tmptri.p2, &tof);
			converted_tri1->tx.y = converted_tri1->tx.x + (converted_tri1->tx.y - converted_tri1->tx.x) * tof;
			converted_tri1->ty.y = converted_tri1->ty.x + (converted_tri1->ty.y - converted_tri1->ty.x) * tof;
			converted_tri1->p3 = vector_clip(far_depth, &tmptri.p1, &tmptri.p3, &tof);
			converted_tri1->tx.q = converted_tri1->tx.x + (converted_tri1->tx.q - converted_tri1->tx.x) * tof;
			converted_tri1->ty.q = converted_tri1->ty.x + (converted_tri1->ty.q - converted_tri1->ty.x) * tof;
		}
		break;

	default:
		break;
	}

	return split_tri;
}

int clip_near(tri3d* converted_tri1, tri3d* converted_tri2, float near_depth) { 
	// Return num of active tris
	// Converted tri 1 holds input
	tri3d tmptri;
	float tof;

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
	clip_tags::OutCode outcode1 = get_near(tmptri.p1, near_depth);
	clip_tags::OutCode outcode2 = get_near(tmptri.p2, near_depth);
	clip_tags::OutCode outcode3 = get_near(tmptri.p3, near_depth);

	int nearsum = (outcode1 & clip_tags::NEAR) + (outcode2 & clip_tags::NEAR) + (outcode3 & clip_tags::NEAR);

	if (nearsum == 48) return 0; // Near clip
	if (nearsum == 0) return 1; // No clip

	int split_tri = 1;

	switch (nearsum) {
	case 16:
		split_tri = 2;
		if (outcode1 & clip_tags::NEAR) {
			converted_tri2->p1 = vector_clip(near_depth, &tmptri.p3, &tmptri.p1, &tof);
			converted_tri2->tx.x = converted_tri1->tx.q + (converted_tri1->tx.x - converted_tri1->tx.q) * tof;
			converted_tri2->ty.x = converted_tri1->ty.q + (converted_tri1->ty.x - converted_tri1->ty.q) * tof;
			converted_tri2->p2 = vector_clip(near_depth, &tmptri.p2, &tmptri.p1, &tof);
			converted_tri2->tx.y = converted_tri1->tx.y + (converted_tri1->tx.x - converted_tri1->tx.y) * tof;
			converted_tri2->ty.y = converted_tri1->ty.y + (converted_tri1->ty.x - converted_tri1->ty.y) * tof;
			
			converted_tri2->p3.x = tmptri.p3.x;
			converted_tri2->p3.y = tmptri.p3.y;
			converted_tri2->p3.z = tmptri.p3.z;
			converted_tri2->tx.q = converted_tri1->tx.q;
			converted_tri2->ty.q = converted_tri1->ty.q; 

			converted_tri1->p1.x = converted_tri2->p2.x;
			converted_tri1->p1.y = converted_tri2->p2.y;
			converted_tri1->p1.z = converted_tri2->p2.z;
			converted_tri1->tx.x = converted_tri2->tx.y;
			converted_tri1->ty.x = converted_tri2->ty.y;
		} 
		else if (outcode2 & clip_tags::NEAR) {
			converted_tri2->p1 = vector_clip(near_depth, &tmptri.p1, &tmptri.p2, &tof);
			converted_tri2->tx.x = converted_tri1->tx.x + (converted_tri1->tx.y - converted_tri1->tx.x) * tof;
			converted_tri2->ty.x = converted_tri1->ty.x + (converted_tri1->ty.y - converted_tri1->ty.x) * tof;
			converted_tri2->p2 = vector_clip(near_depth, &tmptri.p3, &tmptri.p2, &tof);
			converted_tri2->tx.y = converted_tri1->tx.q + (converted_tri1->tx.y - converted_tri1->tx.q) * tof;
			converted_tri2->ty.y = converted_tri1->ty.q + (converted_tri1->ty.y - converted_tri1->ty.q) * tof;
			
			converted_tri2->p3.x = tmptri.p3.x;
			converted_tri2->p3.y = tmptri.p3.y;
			converted_tri2->p3.z = tmptri.p3.z;
			converted_tri2->tx.q = converted_tri1->tx.q;
			converted_tri2->ty.q = converted_tri1->ty.q;

			converted_tri1->p2.x = converted_tri2->p1.x;
			converted_tri1->p2.y = converted_tri2->p1.y;
			converted_tri1->p2.z = converted_tri2->p1.z;
			converted_tri1->tx.y = converted_tri2->tx.x;
			converted_tri1->ty.y = converted_tri2->ty.x;
		} 
		else {
			converted_tri2->p2 = vector_clip(near_depth, &tmptri.p2, &tmptri.p3, &tof);
			converted_tri2->tx.y = converted_tri1->tx.y + (converted_tri1->tx.q - converted_tri1->tx.y) * tof;
			converted_tri2->ty.y = converted_tri1->ty.y + (converted_tri1->ty.q - converted_tri1->ty.y) * tof;
			converted_tri2->p3 = vector_clip(near_depth, &tmptri.p1, &tmptri.p3, &tof);
			converted_tri2->tx.q = converted_tri1->tx.x + (converted_tri1->tx.q - converted_tri1->tx.x) * tof;
			converted_tri2->ty.q = converted_tri1->ty.x + (converted_tri1->ty.q - converted_tri1->ty.x) * tof;

			converted_tri2->p1.x = tmptri.p1.x;
			converted_tri2->p1.y = tmptri.p1.y;
			converted_tri2->p1.z = tmptri.p1.z;
			converted_tri2->tx.x = converted_tri1->tx.x;
			converted_tri2->ty.x = converted_tri1->ty.x;

			converted_tri1->p3.x = converted_tri2->p2.x;
			converted_tri1->p3.y = converted_tri2->p2.y;
			converted_tri1->p3.z = converted_tri2->p2.z;
			converted_tri1->tx.q = converted_tri2->tx.y;
			converted_tri1->ty.q = converted_tri2->ty.y;
		}
		break;
	case 32:
		if (outcode1 & clip_tags::NEAR && outcode2 & clip_tags::NEAR) {
			converted_tri1->p1 = vector_clip(near_depth, &tmptri.p3, &tmptri.p1, &tof);
			converted_tri1->tx.x = converted_tri1->tx.q + (converted_tri1->tx.x - converted_tri1->tx.q) * tof;
			converted_tri1->ty.x = converted_tri1->ty.q + (converted_tri1->ty.x - converted_tri1->ty.q) * tof;
			converted_tri1->p2 = vector_clip(near_depth, &tmptri.p3, &tmptri.p2, &tof);
			converted_tri1->tx.y = converted_tri1->tx.q + (converted_tri1->tx.y - converted_tri1->tx.q) * tof;
			converted_tri1->ty.y = converted_tri1->ty.q + (converted_tri1->ty.y - converted_tri1->ty.q) * tof;
		} 
		else if (outcode1 & clip_tags::NEAR && outcode3 & clip_tags::NEAR) {
			converted_tri1->p1 = vector_clip(near_depth, &tmptri.p2, &tmptri.p1, &tof);
			converted_tri1->tx.x = converted_tri1->tx.y + (converted_tri1->tx.x - converted_tri1->tx.y) * tof;
			converted_tri1->ty.x = converted_tri1->ty.y + (converted_tri1->ty.x - converted_tri1->ty.y) * tof;
			converted_tri1->p3 = vector_clip(near_depth, &tmptri.p2, &tmptri.p3, &tof);
			converted_tri1->tx.q = converted_tri1->tx.y + (converted_tri1->tx.q - converted_tri1->tx.y) * tof;
			converted_tri1->ty.q = converted_tri1->ty.y + (converted_tri1->ty.q - converted_tri1->ty.y) * tof;
		} 
		else {
			converted_tri1->p2 = vector_clip(near_depth, &tmptri.p1, &tmptri.p2, &tof);
			converted_tri1->tx.y = converted_tri1->tx.x + (converted_tri1->tx.y - converted_tri1->tx.x) * tof;
			converted_tri1->ty.y = converted_tri1->ty.x + (converted_tri1->ty.y - converted_tri1->ty.x) * tof;
			converted_tri1->p3 = vector_clip(near_depth, &tmptri.p1, &tmptri.p3, &tof);
			converted_tri1->tx.q = converted_tri1->tx.x + (converted_tri1->tx.q - converted_tri1->tx.x) * tof;
			converted_tri1->ty.q = converted_tri1->ty.x + (converted_tri1->ty.q - converted_tri1->ty.x) * tof;
		}
		break;

	default:
		break;
	}

	return split_tri;
}

int setup_render(camera* camera, SDL_Renderer* renderer) {
	get_normal_from_cam(camera);
	set_rotation(&camera->matrix, &camera->camera_vect);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
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

int draw_buffer(SDL_Renderer* renderer, std::vector<float>& buffer, int half_screen_x, int half_screen_y, int mode) {
	int screen_y = 2 * half_screen_y;
	int screen_x = 2 * half_screen_x;
	switch (mode) {
	case 0:
		int depth_map_value;
		for (size_t i = 0; i < screen_y; i++) {
			for (size_t q = 0; q < screen_x; q++) {
				depth_map_value = buffer.at(screen_x * i + q) * 100;
				SDL_SetRenderDrawColor(renderer, depth_map_value, depth_map_value, depth_map_value, 255);
				SDL_RenderDrawPoint(renderer, (int)q, (int)i);
			}
		}
	case 1:
		for (size_t y_pos = 0; y_pos < screen_y; y_pos++) {
			for (size_t x_pos = 0; x_pos < screen_x; x_pos++) {
				SDL_SetRenderDrawColor(renderer, buffer.at((screen_x * y_pos + x_pos) * 3), buffer.at((screen_x * y_pos + x_pos) * 3 + 1), buffer.at((screen_x * y_pos + x_pos) * 3 + 2), 255);
				SDL_RenderDrawPoint(renderer, (int)x_pos, (int)y_pos);
			}
		}
	}
	return 0;
}

int draw_texture(SDL_Renderer* renderer, texture texture) {
	color color;
	for (size_t i = 0; i < texture.height; i++) {
		for (size_t q = 0; q < texture.width; q++) {
			color = texture.get_pixel(q, i);
			SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
			SDL_RenderDrawPoint(renderer, (int)q, (int)i);
		}
	}
	return 0;
}

int clear_buffer(std::vector<float>& buffer) {  
	for (size_t i = buffer.size() - 1; i != 0; i--) {
		buffer[i] = 0.0f;
	}
	return 0;
}

int get_normal_from_cam(camera* camera) {
	vec3d angle = { camera->angles.x * 3.1415 / 180, camera->angles.y * 3.1415 / 180, camera->angles.z * 3.1415 / 180 };
	camera->camera_vect = { cos(angle.x)*cos(angle.y), cos(angle.x)*sin(angle.y), sin(angle.x), 1};
	float len = pow(pow(camera->camera_vect.x, 2) + pow(camera->camera_vect.y, 2) + pow(camera->camera_vect.z, 2), 0.5);
	if (len == 0) return 0;
	camera->camera_vect.x /= len;
	camera->camera_vect.y /= len;
	camera->camera_vect.z /= len;

	return 0;
}