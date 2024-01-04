#ifndef MAT3_H
#define MAT3_H

#include "vec3.h"
#include "mat4.h"

struct mat4;

typedef struct {
	float data[9];
} mat3; //column-major order

mat3 mat3_create(float szam);
mat3 mat3_create2(float* values);
mat3 mat3_createFromVec(vec3 col0, vec3 col1, vec3 col2);
mat3 mat3_createFromMat(struct mat4);

void mat3_set(mat3* mat, int row, int col, float value);

mat3 mat3_transpose(mat3 mat);

mat3 mat3_sum(mat3 egy, mat3 ketto);
mat3 mat3_subtract(mat3 egy, mat3 ketto);
mat3 mat3_multiply(mat3 egy, mat3 ketto);

float mat3_determinant(mat3* mat);

mat3 mat3_inverse(mat3 mat);

void mat3_print(mat3* mat);

#endif