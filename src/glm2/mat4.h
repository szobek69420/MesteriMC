#ifndef MAT4_H
#define MAT4_H

#include "vec3.h"
#include "vec4.h"
#include "mat3.h"

typedef struct mat4 {
	float data[16];
} mat4; //column-major order

mat4 mat4_create(float szam);
mat4 mat4_create2(float* data);
mat4 mat4_createFromVec(vec4 col0, vec4 col1, vec4 col2, vec4 col3);
mat4 mat4_createFromMat(mat3);

float mat4_get(mat4* mat, int row, int col, float value);
void  mat4_set(mat4* mat, int row, int col, float value);

float mat4_determinant(mat4* mat);

mat4 mat4_sum(mat4 egy, mat4 katto);
mat4 mat4_multiply(mat4 egy, mat4 katto);

mat4 mat4_transpose(mat4 mat);
mat4 mat4_inverse(mat4 mat);


mat4 mat4_scale(mat4 mat, vec3 scale);
mat4 mat4_rotate(mat4 mat, vec3 axis, float angle);
mat4 mat4_translate(mat4 mat, vec3 pos);


mat4 mat4_lookAt(vec3 pos, vec3 direction, vec3 up);

mat4 mat4_perspective(float fov, float aspectXY, float near, float far);
mat4 mat4_ortho(float left, float right, float bottom, float top, float near, float far);

void mat4_print(mat4* mat);

#endif