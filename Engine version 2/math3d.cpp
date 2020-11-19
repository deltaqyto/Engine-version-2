#include "math3d.h"
#include "iostream"
#include <math.h>

float vec3d_dot(vec3d a, vec3d b) {
	return(a.x * b.x + a.y * b.y + a.z * b.z);
}

vec3d vec3d_by_const(vec3d a, float val) {
	vec3d out;
	out.x = a.x * val;
	out.y = a.y * val;
	out.z = a.z * val;
	out.q = a.q;
	return out;
}

int normalize(vec3d* a) { // OPtimize
	float l = pow(pow(a->x, 2) + pow(a->y, 2) + pow(a->z, 2) + pow(a->q, 2), 0.5);
	if (l == 0) return -1;
	a->x = a->x / l;
	a->y = a->y / l;
	a->z = a->z / l;
	return 0;
}

vec3d vector_clip(float dist, vec3d* lineStart, vec3d* lineEnd)
{
	vec3d out;
	float t = (dist - lineStart->z) / (lineEnd->z - lineStart->z);
	out.x = lineStart->x + t * (lineEnd->x - lineStart->x);
	out.y = lineStart->y + t * (lineEnd->y - lineStart->y);
	out.z = dist;
	return out;
}

ray3d seg_to_ray(vec3d a, vec3d b) {
	ray3d out;
	out.org = a;
	out.dir.x = b.x - a.x;
	out.dir.y = b.y - a.y;
	out.dir.z = b.z - a.z;
	out.dir.z = 1;
	return out;
}

int fast_seg_plane() {
	return 0;
}

int ray_plane_intersect(ray3d ray, plane plane, float* time, float* collisionx, float* collisiony, float* collisionz) {

	// Plane clipper found on 358
	// https://stackoverflow.com/questions/23975555/how-to-do-ray-plane-intersection
	float d = vec3d_dot(plane.org, vec3d_by_const(plane.norm, -1));
	float denom = vec3d_dot(ray.dir, plane.norm);
	if (denom == 0) {
		return 0;
	}

	float t = -(d + ray.org.z * plane.norm.z + ray.org.y * plane.norm.y + ray.org.x * plane.norm.x) / denom;

	if (time != nullptr)
		*time = t;
	if (collisionx != nullptr)
		*collisionx = ray.org.x + t * ray.dir.x;
	if (collisiony != nullptr)
		*collisiony = ray.org.y + t * ray.dir.y;
	if (collisionz != nullptr)
		*collisionz = ray.org.z + t * ray.dir.z;
	return 1;
}

int get_inverse(matx3d* cam_matx) {
	matx3d m = *cam_matx;
	float det;
	int i;

	cam_matx->a.x = m.b.y * m.c.z * m.d.q - m.b.y * m.c.q * m.d.z - m.c.y * m.b.z * m.d.q + m.c.y * m.b.q * m.d.z + m.d.y * m.b.z * m.c.q - m.d.y * m.b.q * m.c.z;

	cam_matx->b.x = -m.b.x * m.c.z * m.d.q + m.b.x * m.c.q * m.d.z + m.c.x * m.b.z * m.d.q - m.c.x * m.b.q * m.d.z - m.d.x * m.b.z * m.c.q + m.d.x * m.b.q * m.c.z;

	cam_matx->c.x = m.b.x * m.c.y * m.d.q - m.b.x * m.c.q * m.d.y - m.c.x * m.b.y * m.d.q + m.c.x * m.b.q * m.d.y + m.d.x * m.b.y * m.c.q - m.d.x * m.b.q * m.c.y;

	cam_matx->d.x = -m.b.x * m.c.y * m.d.z + m.b.x * m.c.z * m.d.y + m.c.x * m.b.y * m.d.z - m.c.x * m.b.z * m.d.y - m.d.x * m.b.y * m.c.z + m.d.x * m.b.z * m.c.y;

	cam_matx->a.y = -m.a.y * m.c.z * m.d.q + m.a.y * m.c.q * m.d.z + m.c.y * m.a.z * m.d.q - m.c.y * m.a.q * m.d.z - m.d.y * m.a.z * m.c.q + m.d.y * m.a.q * m.c.z;

	cam_matx->b.y = m.a.x * m.c.z * m.d.q - m.a.x * m.c.q * m.d.z - m.c.x * m.a.z * m.d.q + m.c.x * m.a.q * m.d.z + m.d.x * m.a.z * m.c.q - m.d.x * m.a.q * m.c.z;

	cam_matx->c.y = -m.a.x * m.c.y * m.d.q + m.a.x * m.c.q * m.d.y + m.c.x * m.a.y * m.d.q - m.c.x * m.a.q * m.d.y - m.d.x * m.a.y * m.c.q + m.d.x * m.a.q * m.c.y;

	cam_matx->d.y = m.a.x * m.c.y * m.d.z - m.a.x * m.c.z * m.d.y - m.c.x * m.a.y * m.d.z + m.c.x * m.a.z * m.d.y + m.d.x * m.a.y * m.c.z - m.d.x * m.a.z * m.c.y;

	cam_matx->a.z = m.a.y * m.b.z * m.d.q - m.a.y * m.b.q * m.d.z - m.b.y * m.a.z * m.d.q + m.b.y * m.a.q * m.d.z + m.d.y * m.a.z * m.b.q - m.d.y * m.a.q * m.b.z;

	cam_matx->b.z = -m.a.x * m.b.z * m.d.q + m.a.x * m.b.q * m.d.z + m.b.x * m.a.z * m.d.q - m.b.x * m.a.q * m.d.z - m.d.x * m.a.z * m.b.q + m.d.x * m.a.q * m.b.z;

	cam_matx->c.z = m.a.x * m.b.y * m.d.q - m.a.x * m.b.q * m.d.y - m.b.x * m.a.y * m.d.q + m.b.x * m.a.q * m.d.y + m.d.x * m.a.y * m.b.q - m.d.x * m.a.q * m.b.y;

	cam_matx->d.z = -m.a.x * m.b.y * m.d.z + m.a.x * m.b.z * m.d.y + m.b.x * m.a.y * m.d.z - m.b.x * m.a.z * m.d.y - m.d.x * m.a.y * m.b.z + m.d.x * m.a.z * m.b.y;

	cam_matx->a.q = -m.a.y * m.b.z * m.c.q + m.a.y * m.b.q * m.c.z + m.b.y * m.a.z * m.c.q - m.b.y * m.a.q * m.c.z - m.c.y * m.a.z * m.b.q + m.c.y * m.a.q * m.b.z;

	cam_matx->b.q = m.a.x * m.b.z * m.c.q - m.a.x * m.b.q * m.c.z - m.b.x * m.a.z * m.c.q + m.b.x * m.a.q * m.c.z + m.c.x * m.a.z * m.b.q - m.c.x * m.a.q * m.b.z;

	cam_matx->c.q = -m.a.x * m.b.y * m.c.q + m.a.x * m.b.q * m.c.y + m.b.x * m.a.y * m.c.q - m.b.x * m.a.q * m.c.y - m.c.x * m.a.y * m.b.q + m.c.x * m.a.q * m.b.y;

	cam_matx->d.q = m.a.x * m.b.y * m.c.z - m.a.x * m.b.z * m.c.y - m.b.x * m.a.y * m.c.z + m.b.x * m.a.z * m.c.y + m.c.x * m.a.y * m.b.z - m.c.x * m.a.z * m.b.y;

	det = m.a.x * cam_matx->a.x + m.a.y * cam_matx->b.x + m.a.z * cam_matx->c.x + m.a.q * cam_matx->d.x;

	if (det == 0)
		return 1; //Div by zero

	det = 1.0 / det;

	cam_matx->a.x *= det;
	cam_matx->a.y *= det;
	cam_matx->a.z *= det;
	cam_matx->a.q *= det;

	cam_matx->b.x *= det;
	cam_matx->b.y *= det;
	cam_matx->b.z *= det;
	cam_matx->b.q *= det;

	cam_matx->c.x *= det;
	cam_matx->c.y *= det;
	cam_matx->c.z *= det;
	cam_matx->c.q *= det;

	cam_matx->d.x *= det;
	cam_matx->d.y *= det;
	cam_matx->d.z *= det;
	cam_matx->d.q *= det;

	return 0;
}

vec3d matrix_by_vect(matx3d a, vec3d b) {
	vec3d out;
	out.x = a.a.x * b.x + a.a.y * b.y + a.a.z * b.z + a.a.q * b.q;
	out.y = a.b.x * b.x + a.b.y * b.y + a.b.z * b.z + a.b.q * b.q;
	out.z = a.c.x * b.x + a.c.y * b.y + a.c.z * b.z + a.c.q * b.q;
	out.q = a.d.x * b.x + a.d.y * b.y + a.d.z * b.z + a.d.q * b.q;
	return out;
}

vec3d to_vec(float comp_a, float comp_b, float comp_c, float comp_d) {
	vec3d out;
	out.x = comp_a;
	out.y = comp_b;
	out.z = comp_c;
	out.q = comp_d;
	return out;
}

int set_rotation(matx3d* input, vec3d* normal) {
	vec3d top = { 0, 0, 1, 1 };
	vec3d forward = { normal->x, normal->y, normal->z, 1 };
	vec3d right = vec3d_norm_cross(top, forward);
	vec3d up = vec3d_norm_cross(forward, right);

	input->a.x = right.x;
	input->a.y = up.x;
	input->a.z = forward.x;

	input->b.x = right.y;
	input->b.y = up.y;
	input->b.z = forward.y;

	input->c.x = right.z;
	input->c.y = up.z;
	input->c.z = forward.z;

	/*input->a.x = forward.x;
	input->a.y = right.x;
	input->a.z = up.x;

	input->b.x = forward.y;
	input->b.y = right.y;
	input->b.z = up.y;

	input->c.x = forward.z;
	input->c.y = right.z;
	input->c.z = up.z; */

	input->d.x = 0;
	input->d.y = 0;
	input->d.z = 0;
	return 0;
}

int set_cam_rotation(vec3d* angle, float pitch, float yaw) {
	angle->x = pitch;
	angle->y = yaw;
	angle->z = 0;
	return 0;
}

int delta_rotate(matx3d* input, vec3d angle, float anglex, float angley, float anglez) {
	angle.x += anglex;
	angle.y += angley;
	angle.z += anglez;

	float x_rot = (angle.x) * 0.0174f;
	float y_rot = (angle.y) * 0.0174f;
	float z_rot = (angle.z) * 0.0174f;// 3.14/180 -> radian conversion

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

	matx3d final_angle = x * y * z;

	input->a.x = final_angle.a.x;
	input->a.y = final_angle.a.y;
	input->a.z = final_angle.a.z;

	input->b.x = final_angle.b.x;
	input->b.y = final_angle.b.y;
	input->b.z = final_angle.b.z;

	input->c.x = final_angle.c.x;
	input->c.y = final_angle.c.y;
	input->c.z = final_angle.c.z;

	input->d.x = final_angle.d.x;
	input->d.y = final_angle.d.y;
	input->d.z = final_angle.d.z;
	return 0;
}

matx3d operator*(matx3d a, matx3d b) {
	matx3d out; // Written as so to speed up execution, for loops are too slow

	out.a.x = a.a.x * b.a.x + a.a.y * b.b.x + a.a.z * b.c.x + a.a.q * b.d.x;
	out.a.y = a.a.x * b.a.y + a.a.y * b.b.y + a.a.z * b.c.y + a.a.q * b.d.y;
	out.a.z = a.a.x * b.a.z + a.a.y * b.b.z + a.a.z * b.c.z + a.a.q * b.d.z;
	out.a.q = a.a.x * b.a.q + a.a.y * b.b.q + a.a.z * b.c.q + a.a.q * b.d.q;

	out.b.x = a.b.x * b.a.x + a.b.y * b.b.x + a.b.z * b.c.x + a.b.q * b.d.x;
	out.b.y = a.b.x * b.a.y + a.b.y * b.b.y + a.b.z * b.c.y + a.b.q * b.d.y;
	out.b.z = a.b.x * b.a.z + a.b.y * b.b.z + a.b.z * b.c.z + a.b.q * b.d.z;
	out.b.q = a.b.x * b.a.q + a.b.y * b.b.q + a.b.z * b.c.q + a.b.q * b.d.q;

	out.c.x = a.c.x * b.a.x + a.c.y * b.b.x + a.c.z * b.c.x + a.c.q * b.d.x;
	out.c.y = a.c.x * b.a.y + a.c.y * b.b.y + a.c.z * b.c.y + a.c.q * b.d.y;
	out.c.z = a.c.x * b.a.z + a.c.y * b.b.z + a.c.z * b.c.z + a.c.q * b.d.z;
	out.c.q = a.c.x * b.a.q + a.c.y * b.b.q + a.c.z * b.c.q + a.c.q * b.d.q;

	out.d.x = a.d.x * b.a.x + a.d.y * b.b.x + a.d.z * b.c.x + a.d.q * b.d.x;
	out.d.y = a.d.x * b.a.y + a.d.y * b.b.y + a.d.z * b.c.y + a.d.q * b.d.y;
	out.d.z = a.d.x * b.a.z + a.d.y * b.b.z + a.d.z * b.c.z + a.d.q * b.d.z;
	out.d.q = a.d.x * b.a.q + a.d.y * b.b.q + a.d.z * b.c.q + a.d.q * b.d.q;

	return out;
}

vec3d operator*(matx3d a, vec3d b) {
	vec3d out;
	out.x = a.a.x * b.x + a.a.y * b.y + a.a.z * b.z + a.a.q * b.q;
	out.y = a.b.x * b.x + a.b.y * b.y + a.b.z * b.z + a.b.q * b.q;
	out.z = a.c.x * b.x + a.c.y * b.y + a.c.z * b.z + a.c.q * b.q;
	out.q = a.d.x * b.x + a.d.y * b.y + a.d.z * b.z + a.d.q * b.q;
	return out;
}

vec3d vec3d_norm_cross(vec3d a, vec3d b){
	vec3d out;
	out.x = (a.y*b.z - a.z*b.y);
	out.y = (a.z*b.x - a.x*b.z);
	out.z = (a.x*b.y - a.y*b.x);
	float len = pow(pow(out.x, 2) + pow(out.y, 2) + pow(out.z, 2), 0.5);
	if (len == 0) return out;
	out.x /= len;
	out.y /= len;
	out.z /= len;
	return out;
}
