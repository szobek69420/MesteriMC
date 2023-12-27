#ifndef MAT3_H
#define MAT3_H

typedef struct {
	float data[9];
} mat3; //column-major order

mat3 mat3_create(float szam);
mat3 mat3_create2(float* values);

void mat3_set(mat3* mat, int row, int col, float value);


void mat3_print(mat3* mat);

#endif