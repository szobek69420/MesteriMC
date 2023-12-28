#include "vec4.h"

#include <math.h>
#include <stdio.h>

vec4 vec4_create(float szam)
{
	vec4 vec;
	vec.x = szam;
	vec.y = szam;
	vec.z = szam;
	vec.w = szam;
	return vec;
}
vec4 vec4_create2(float x, float y, float z, float w)
{
	vec4 vec;
	vec.x = x;
	vec.y = y;
	vec.z = z;
	vec.w = w;
	return vec;
}

vec4 vec4_sum(vec4 egy, vec4 katto)
{
	egy.x += katto.x;
	egy.y += katto.y;
	egy.z += katto.z;
	egy.w += katto.w;
	return egy;
}

vec4 vec4_subtract(vec4 egy, vec4 katto)
{
	egy.x -= katto.x;
	egy.y -= katto.y;
	egy.z -= katto.z;
	egy.w -= katto.w;
	return egy;
}

float vec4_dot(vec4 egy, vec4 katto)
{
	float szam = egy.x * katto.x+egy.y*katto.y+egy.z*katto.z+egy.w*katto.w;
	return szam;
}

float vec4_sqrMagnitude(vec4 vec)
{
	return vec.x * vec.x + vec.y * vec.y + vec.z * vec.z + vec.w * vec.w;
}

float vec4_magnitude(vec4 vec)
{
	float szam = vec.x * vec.x + vec.y * vec.y + vec.z * vec.z + vec.w * vec.w;
	szam = sqrtf(szam);
	return szam;
}

vec4 vec4_normalize(vec4 vec)
{
	float szam = vec.x * vec.x + vec.y * vec.y + vec.z * vec.z + vec.w * vec.w;
	if (szam == 0)
		return vec;

	szam = 1/sqrtf(szam);
	vec.x *= szam;
	vec.y *= szam;
	vec.z *= szam;
	vec.w *= szam;

	return vec;
}

vec4 vec4_scale(vec4 vec, float szam)
{
	vec.x *= szam;
	vec.y *= szam;
	vec.z *= szam;
	vec.w *= szam;

	return vec;
}

void vec4_print(vec4* vec)
{
	printf("(%.2f; %.2f; %.2f; %.2f)\n", vec->x, vec->y, vec->z, vec->w);
}