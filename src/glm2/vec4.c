#include "vec4.h"
#include "mat4.h"

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

vec4 vec4_multiplyWithMatrix(mat4 mat, vec4 vec)
{
	vec4 result;
	result.x = mat.data[0] * vec.x + mat.data[1] * vec.y + mat.data[2] * vec.z + mat.data[3] * vec.w;
	result.y = mat.data[4] * vec.x + mat.data[5] * vec.y + mat.data[6] * vec.z + mat.data[7] * vec.w;
	result.z = mat.data[8] * vec.x + mat.data[9] * vec.y + mat.data[10] * vec.z + mat.data[11] * vec.w;
	result.w = mat.data[12] * vec.x + mat.data[13] * vec.y + mat.data[14] * vec.z + mat.data[15] * vec.w;

	return result;
}

void vec4_print(vec4* vec)
{
	printf("(%.2f; %.2f; %.2f; %.2f)\n", vec->x, vec->y, vec->z, vec->w);
}