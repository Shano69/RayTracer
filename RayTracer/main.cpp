#pragma once

#include <iostream>
#include <fstream>
#include <random>
#include <math.h>
#include "sphere.h"
#include "hitable_list.h"
#include "float.h"
#include "Camera.h"
#include "material.h"


using namespace std;

//// Seed with real random number if available
//random_device r;
//// Create random number generator
//default_random_engine e(r());
//// Create a distribution - we want doubles between 0.0 and 1.0
//uniform_real_distribution<double> distribution(0.0, 1.0);



vec3 color(const ray& r, hitable *world, int depth)
{
	hit_record rec;
	if (world->hit(r, 0.001, 2147483648.0f, rec)) 
	{
		ray scattered;
		vec3 attenuation;
		if (depth < 50 && rec.mat_ptr->scatter(r, rec, attenuation, scattered))
		{
			return attenuation * color(scattered, world, depth + 1);
		}
		else
		{
			return vec3(0, 0, 0);
		}
	}
	else
	{
		vec3 unit_direction = r.direction();
		unit_direction.make_unit_vector();
		float t = 0.5*(unit_direction.y() + 1.0f);
		return (1.0 - t) *vec3(1.0, 1.0, 1.0) + t * vec3(0.5, 0.7, 1.0);
	}
}


//ideally final scene
hitable *random_scene() {
	int n = 500;
	hitable** list = new hitable*[n + 1];
	list[0] = new sphere(vec3(0, -1000, 0), 1000, new lambertian(vec3(0.5, 0.5, 0.5)));
	int i = 1;
	for (int a = -11; a < 11; a++)
	{
		for (int b = -11; b < 11; b++)
		{
			float choose_mat = distribution(e);
			vec3 center(a + 0.9*distribution(e), 0.2, b + 0.9*distribution(e));
			if ((center - vec3(4, 0.2, 0)).length() > 0.9)
			{
				if (choose_mat < 0.8)
				{
					list[i++] = new sphere(center, 0.2, new lambertian(vec3(distribution(e)*distribution(e), distribution(e)*distribution(e), distribution(e)*distribution(e))));
				}
				else if (choose_mat < 0.95)
				{
					list[i++] = new sphere(center, 0.2,
						new metal(vec3(0.5*(1 + distribution(e)),
							0.5*(1 + distribution(e)),
							0.5*(1 + distribution(e))), 0.5*(1 + distribution(e))));
				}
				else
				{
					list[i++] = new sphere(center, 0.2, new dielectric(1.5));
				}
			}
		}
	}

	list[i++] = new sphere(vec3(0, 1, 0), 1.0, new dielectric(1.5));
	list[i++] = new sphere(vec3(-4, 1, 0), 1.0, new lambertian(vec3(0.4,0.2,0.1)));
	list[i++] = new sphere(vec3(4, 1, 0), 1.0, new metal(vec3(0.7, 0.6, 0.5), 0.0));

	return new hitable_list(list, i);
}

int main() {
	int nx = 1200;
	int ny = 800;
	int ns = 10;
	ofstream image;
	image.open("image.ppm");

	image << " P3\n " << nx << " " << ny << "\n255\n";
	hitable *list[5];
	float R = cos(M_PI / 4);
	list[0] = new sphere(vec3(0, 0, -1), 0.5, new lambertian(vec3(0.1, 0.2, 0.5)));
	list[1] = new sphere(vec3(0, -100.5, -1), 100, new lambertian(vec3(0.8, 0.8, 0.0)));
	list[2] = new sphere(vec3(1, 0, -1), 0.5, new metal(vec3(0.8, 0.6, 0.2), 0.0));
	list[3] = new sphere(vec3(-1, 0, -1), 0.5, new dielectric(1.5));
	list[4] = new sphere(vec3(-1, 0, -1), -0.45, new dielectric(1.5));
	hitable *world = new hitable_list(list, 5);
	world = random_scene();

	vec3 lookfrom(13, 2, 3);
	vec3 lookat(0, 0, 0);
	float dist_to_focus = 10.0;
	float aperture = 0.1;

	Camera cam(lookfrom, lookat, vec3(0, 1, 0), 20, float(nx) / float(ny), aperture, dist_to_focus);

	

	for (int j = ny - 1; j >= 0; j--)
	{
		for (int i = 0; i < nx; i++)
		{
			vec3 col(0, 0, 0);
			for (int s = 0; s < ns; s++)
			{
				float u = float(i + distribution(e)) / float(nx);
				float v = float(j + distribution(e)) / float(ny);

				ray r = cam.get_ray(u, v);
				vec3 p = r.point_at_parameter(2.0);
				col += color(r, world, 0);
			}
			
			col /= ns;
			col = vec3(sqrt(col[0]), sqrt(col[1]), sqrt(col[2]));
			int ir = int(255.99*col[0]);
			int ig = int(255.99*col[1]);
			int ib = int(255.99*col[2]);

			image << ir << " " << ig << " " << ib << endl;
		}
	}
	image.close();
	system("pause");

	return 0;

}
