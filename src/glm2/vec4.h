#ifndef VEC4_H
#define VEC4_H

struct mat4;

typedef struct {
	float x;
	float y;
	float z;
    float w;
} vec4;

vec4 vec4_create(float);
vec4 vec4_create2(float, float, float,float);

vec4 vec4_sum(vec4, vec4);
vec4 vec4_subtract(vec4, vec4);
float vec4_dot(vec4, vec4);

float vec4_sqrMagnitude(vec4);
float vec4_magnitude(vec4);
vec4 vec4_normalize(vec4);
vec4 vec4_scale(vec4, float);

vec4 vec4_multiplyWithMatrix(struct mat4, vec4);

void vec4_print(vec4*);

#endif