#pragma once

#include "ray.h"
#include "hitable_list.h"
#include <random>



class material {
public:

	static vec3 reflect(const vec3& v, const vec3& n)
	{
		return v - 2 * vec3::dot(v, n)*n;
	}

	static bool refract(const vec3& v, const vec3& n, float ni_over_nt, vec3& refracted)
	{
		vec3 uv = v;
		uv.make_unit_vector();
		float dt = vec3::dot(uv, n);
		float discriminant = 1.0 - ni_over_nt * ni_over_nt*(1 - dt * dt);
		if (discriminant > 0)
		{
			refracted = ni_over_nt * (uv - n * dt) - n * sqrt(discriminant);
			return true;
		}
		else false;
	}

	static vec3 random_in_unit_sphere()
	{
		vec3 p;
		do {
			p = 2.0*vec3(distribution(e), distribution(e), distribution(e)) - vec3(1, 1, 1);
		} while (p.squared_length() >= 1.0f);
		return p;
	}

	static float shlick(float cosine, float ref_idx)
	{
		float r0 = (1 - ref_idx) / (1 + ref_idx);
		r0 = r0 * r0;
		return r0 + (1 - r0)*pow((1 - cosine), 5);
	}

	virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const = 0;
};

class lambertian : public material {
public:
	lambertian(const vec3& a) : albedo(a) {}
	virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const
	{
		vec3 target = rec.p + rec.normal + material::random_in_unit_sphere();
		scattered = ray(rec.p, target - rec.p);
		attenuation = albedo;
		return true;
	}

	vec3 albedo;
};

class metal : public material {
public:
	metal(const vec3& a, float f) : albedo(a) { if (f < 1) fuzz = f; else fuzz = 1; }
	virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const
	{
		vec3 ray_dir = r_in.direction();
		ray_dir.make_unit_vector();
		vec3 reflected = reflect(ray_dir, rec.normal);
		scattered = ray(rec.p, reflected+ fuzz* random_in_unit_sphere());
		attenuation = albedo;
		return (vec3::dot(scattered.direction(), rec.normal )> 0);
	}

	float fuzz;
	vec3 albedo;
};

class dielectric : public material {
public:
	dielectric(float ri) : ref_idx(ri) {}
	virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const {
		vec3 outward_normal;
		vec3 reflected = reflect(r_in.direction(), rec.normal);
		float ni_over_nt;
		attenuation = vec3(1.0, 1.0, 1.0);
		vec3 refracted;
		float reflect_prob;
		float cosine;
		if (vec3::dot(r_in.direction(), rec.normal) > 0) {
			outward_normal = -rec.normal;
			ni_over_nt = ref_idx;
			//         cosine = ref_idx * dot(r_in.direction(), rec.normal) / r_in.direction().length();
			cosine = vec3::dot(r_in.direction(), rec.normal) / r_in.direction().length();
			cosine = sqrt(1 - ref_idx * ref_idx*(1 - cosine * cosine));
		}
		else {
			outward_normal = rec.normal;
			ni_over_nt = 1.0 / ref_idx;
			cosine = -vec3::dot(r_in.direction(), rec.normal) / r_in.direction().length();
		}
		if (refract(r_in.direction(), outward_normal, ni_over_nt, refracted))
			reflect_prob = shlick(cosine, ref_idx);
		else
			reflect_prob = 1.0;
		if (distribution(e) < reflect_prob)
			scattered = ray(rec.p, reflected);
		else
			scattered = ray(rec.p, refracted);
		return true;
	}

	float ref_idx;
};