#ifndef VEC3_H
#define VEC3_H
typedef struct {
	float x;
	float y;
	float z;
} vec3;

vec3 vec3_create(float);
vec3 vec3_create2(float, float, float);

vec3 vec3_sum(vec3, vec3);
vec3 vec3_subtract(vec3, vec3);
float vec3_dot(vec3, vec3);
vec3 vec3_cross(vec3, vec3);

float vec3_sqrMagnitude(vec3);
float vec3_magnitude(vec3);
vec3 vec3_normalize(vec3);
vec3 vec3_scale(vec3, float);

void vec3_print(vec3*);

#endif