#include "mat4.h"
#include "mat3.h"

#include <stdio.h>

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


float mat4_determinant(mat4* mat)
{
    float det = 0;
    float* v = mat->data;//to keep things short
    mat3 submat;

    submat = mat3_create2((float[]){
        v[5],v[6],v[7],
        v[9],v[10],v[11],
        v[13],v[14],v[15]
        });
    det += v[0] * mat3_determinant(&submat);
    
    submat = mat3_create2((float[]) {
        v[1], v[2], v[3],
        v[9], v[10], v[11],
        v[13], v[14], v[15]
    });
    det -= v[4] * mat3_determinant(&submat);

    submat = mat3_create2((float[]) {
        v[1], v[2], v[3],
        v[5], v[6], v[7],
        v[13], v[14], v[15]
    });
    det += v[8] * mat3_determinant(&submat);

    submat = mat3_create2((float[]) {
        v[1], v[2], v[3],
        v[5], v[6], v[7],
        v[9], v[10], v[11]
    });
    det -= v[12] * mat3_determinant(&submat);

    return det;
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
    mat4 mat=mat4_create(0);
    
    for (int i = 0; i < 4; i++) //row
        for (int j = 0; j < 4; j++) //column
            for (int k = 0; k < 4; k++) //index
                mat.data[j * 4 + i] += egy.data[k * 4 + i] * katto.data[j * 4 + k];

    return mat;
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
    mat4 newMat;
    mat3 subMat;
    float* d = mat.data;//to keep things simple
    int sorIndex[3];
    int oszlopIndex[3];
    int temp;

    float szam = mat4_determinant(&mat);
    if (szam == 0)
        return mat;//mindegy mi megy vissza, mert úgy sincs inverze a mátrixnak
    szam = 1 / szam;

    //calculating adjugate matrix
    for (int i = 0; i < 4; i++)//column
    {
        for (int j = 0; j < 4; j++)//row
        {
            //setting up submatrix
            temp = 0;
            for (int k = 0; k < 4; k++) 
            {
                if (k == j)
                    continue;
                sorIndex[temp] = k;
                temp++;
            }
            temp = 0;
            for (int k = 0; k < 4; k++)
            {
                if (k == i)
                    continue;
                oszlopIndex[temp] = k;
                temp++;
            }
            
            subMat = mat3_create2((float[]){
                d[4 * oszlopIndex[0] + sorIndex[0]],d[4 * oszlopIndex[0] + sorIndex[1]],d[4 * oszlopIndex[0] + sorIndex[2]],
                d[4 * oszlopIndex[1] + sorIndex[0]],d[4 * oszlopIndex[1] + sorIndex[1]],d[4 * oszlopIndex[1] + sorIndex[2]],
                d[4 * oszlopIndex[2] + sorIndex[0]],d[4 * oszlopIndex[2] + sorIndex[1]],d[4 * oszlopIndex[2] + sorIndex[2]]
                });


            //set value
            if ((i + j) % 2 == 0)
                newMat.data[4 * i + j] = mat3_determinant(&subMat);
            else
                newMat.data[4 * i + j] = -1*mat3_determinant(&subMat);
        }
    }

    newMat = mat4_transpose(newMat);

    //finishing it
    for (int i = 0; i < 16; i++)
        newMat.data[i] *= szam;

    return newMat;
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

void mat4_print(mat4* mat)
{
    for (int i = 0; i < 4; i++) {
        printf("%.2f %.2f %.2f %.2f\n", mat->data[i], mat->data[4 + i], mat->data[8 + i], mat->data[12 + i]);
    }
}