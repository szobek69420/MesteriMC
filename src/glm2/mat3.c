#include "mat3.h"
#include <stdio.h>


mat3 mat3_create(float szam)
{
    mat3 mat;
    for (int i = 0; i < 9; i++)
        mat.data[i] = 0.0f;
    mat3_set(&mat, 0, 0, szam);
    mat3_set(&mat, 1, 1, szam);
    mat3_set(&mat, 2, 2, szam);
    return mat;
}

mat3 mat3_create2(float* values)
{
    mat3 mat;

    for (int i = 0; i < 9; i++)
        mat.data[i] = values[i];

    return mat;
}


void mat3_set(mat3* mat, int row, int col, float value)
{
    mat->data[col * 3 + row] = value;
}


void mat3_print(mat3* mat)
{
    for (int i = 0; i < 3; i++) {
        printf("%.2f %.2f %.2f\n", mat->data[i], mat->data[3 + i], mat->data[6 + i]);
    }
}