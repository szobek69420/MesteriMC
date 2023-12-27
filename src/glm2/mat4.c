#include "mat4.h"

mat4 mat4_create(float szam)
{
    mat4 mat;
    for (int i = 0; i < 16; i++)
        mat.data[i] = 0.0f;
    mat4_set(&mat, 0, 0, szam);
    mat4_set(&mat, 1, 1, szam);
    mat4_set(&mat, 2, 2, szam);
    mat4_set(&mat, 3, 3, szam);
    return mat;
}
mat4 mat4_create2(float* data)
{
    mat4 mat;
    for (int i = 0; i < 16; i++)
        mat.data[i] = data[i];
    return mat;
}

float mat4_get(mat4* mat, int row, int col, float value)
{
    return mat->data[col * 4 + row];
}
void  mat4_set(mat4* mat, int row, int col, float value)
{
    mat->data[col * 4 + row] = value;
}


mat4 mat4_sum(mat4 egy, mat4 katto)
{
    mat4 mat;
    for (int i = 0; i < 16; i++)
        mat.data[i] = egy.data[i] + katto.data[i];
    return mat;
}
mat4 mat4_multiply(mat4 egy, mat4 katto)
{
    mat4 mat;
    for (int i = 0; i < 4; i++) //row
        for (int j = 0; j < 4; j++) //column
            for (int k = 0; k < 4; k++) //index
                mat.data[j * 4 + i] += egy.data[j * 4 + k] * katto.data[k * 4 + i];
}

mat4 mat4_transpose(mat4 mat)
{
    mat4 mat2;
    for (int i = 0; i < 4; i++) //row
        for (int j = 0; j < 4; j++) //column
            mat2.data[i * 4 + j] = mat.data[j * 4 + i];
    return mat2;
}
mat4 mat4_inverse(mat4 mat)
{
    ///TODO
}

mat4 mat4_scale(mat4 mat, vec3 scale)
{
    //TODO
}
mat4 mat4_rotate(mat4 mat, vec3 axis, float angle)
{
    //TODO
}
mat4 mat4_translate(mat4 mat, vec3 pos)
{
    //TODO
}

mat4 mat4_lookAt(vec3 pos, vec3 direction, vec3 up)
{
    //TODO
}

mat4 mat4_perspective(float fov, float aspectXY, float near, float far)
{
    //TODO
}
mat4 mat4_ortho(float left, float right, float bottom, float top, float near, float far)
{
    //TODO
}