#include"math.h"

vector vect_by_value(vector a, float value) {
	vector c;
	c.x = a.x * value;
	c.y = a.y * value;
	c.q = a.q * value;
	return c;
}

vector vect_div_value(vector a, float value) {
	vector c;
	c.x = a.x / value;
	c.y = a.y / value;
	c.q = a.q / value;
	return c;
}

vector vect_sub(vector a, vector b) {
	vector c;
	c.x = a.x - b.x;
	c.y = a.y - b.y;
	c.q = a.q;
	return c;
}

vector vect_add(vector a, vector b) {
	vector c;
	c.x = a.x + b.x;
	c.y = a.y + b.y;
	c.q = a.q;
	return c;
}

float vect_dot_vect(vector a, vector b) {
	return(a.x * b.x + a.y * b.y + a.q * b.q);
}

point ray_pos_from_t(ray a, float t) {
	point q;
	return a.org;
}

int seg_seg_intersect(seg a, seg b, float* seg1_time, float* seg2_time, float* crossx, float* crossy) {
	// ray -> d*t + o	 d = direction, t = time, o = origin
	// seg -> a + s*(b - a), a = start, b = end, s = time
	// Useful: https://stackoverflow.com/questions/563198/how-do-you-detect-where-two-line-segments-intersect/565282#565282
	// The two division operations can be avoided for speed(division costs more than multiplication); 
	// if the lines intersect you need one division, if they do not intersect you need zero.One should 
	// first calculate the denominatorand stop early if it is zero(possibly adding code to detect colinearity.)
	// Next, instead of calculating s and t directly, test the relationship between the two numeratorsand the
	// denominator.Only if the lines are confirmed to intersect do you actually need to calculate the value of t(but not s)
		
	seg line;
	line.start.x = a.end.x - a.start.x;     
	line.start.y = a.end.y - a.start.y;
	line.start.q = 0;

	line.end.x = b.end.x - b.start.x;       
	line.end.y = b.end.y - b.start.y;
	line.end.q = 0;

	float den = (-line.end.x * line.start.y + line.start.x * line.end.y);
	if (den == 0) {
		return 0;
	}

	float s = (-line.start.y * (a.start.x - b.start.x) + line.start.x * (a.start.y - b.start.y)) / den;
	
	if (s >= 0 && s <= 1) {
		float t = (line.end.x * (a.start.y - b.start.y) - line.end.y * (a.start.x - b.start.x)) / den;
		if (t >= 0 && t <= 1) {
			if (seg2_time != nullptr)
				*seg2_time = s;
			if (seg1_time != nullptr)
				*seg1_time = t;
			// Collision detected
			if (crossx != nullptr)
				*crossx = a.start.x + (t * line.start.x);
			if (crossy != nullptr)
				*crossy = a.start.y + (t * line.start.y);
			return 1;
		}
	}

	return 0; // No collision
}

float flat_cross_product(vector2d a, vector2d b) {
	return(a.x * b.y - a.y * b.x); // Return z component only
}


float vect_dot_vect(vector2d a, vector2d b) {
	return(a.x * b.x + a.y * b.y);
}
