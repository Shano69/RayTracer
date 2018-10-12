#pragma once

#include "ray.h"

class material;

struct hit_record {
	//which time it hit
	float t;
	//the point it hit
	vec3 p;
	//the normal of the surface of hit
	vec3 normal;
	//material of the  object hit
	material *mat_ptr;
};
class hitable
{

public:
	virtual bool hit(const ray& r, float t_min, float t_max, hit_record& rec) const = 0;
	
};

