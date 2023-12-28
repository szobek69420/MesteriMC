#include "mat3.h"
#include "vec3.h"
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

mat3 mat3_createFromVec(vec3 col0, vec3 col1, vec3 col2)
{
    mat3 vissza;

    vissza.data[0] = col0.x;
    vissza.data[1] = col0.y;
    vissza.data[2] = col0.z;

    vissza.data[3] = col1.x;
    vissza.data[4] = col1.y;
    vissza.data[5] = col1.z;

    vissza.data[6] = col2.x;
    vissza.data[7] = col2.y;
    vissza.data[8] = col2.z;

    return vissza;
}


mat3 mat3_createFromMat(mat4 mat)
{
    mat3 vissza;

    vissza.data[0] = mat.data[0];
    vissza.data[1] = mat.data[1];
    vissza.data[2] = mat.data[2];

    vissza.data[3] = mat.data[4];
    vissza.data[4] = mat.data[5];
    vissza.data[5] = mat.data[6];
    
    vissza.data[6] = mat.data[8];
    vissza.data[7] = mat.data[9];
    vissza.data[8] = mat.data[10];

    return vissza;
}


void mat3_set(mat3* mat, int row, int col, float value)
{
    mat->data[col * 3 + row] = value;
}


mat3 mat3_transpose(mat3 mat)
{
    float temp;
    
    temp = mat.data[1];
    mat.data[1] = mat.data[3];
    mat.data[3] = temp;

    temp = mat.data[2];
    mat.data[2] = mat.data[6];
    mat.data[6] = temp;

    temp = mat.data[5];
    mat.data[5] = mat.data[7];
    mat.data[7] = temp;

    return mat;
}

mat3 mat3_sum(mat3 egy, mat3 ketto)
{
    for (int i = 0; i < 9; i++)
        egy.data[i] += ketto.data[i];

    return egy;
}


mat3 mat3_subtract(mat3 egy, mat3 ketto)
{
    for (int i = 0; i < 9; i++)
        egy.data[i] -= ketto.data[i];

    return egy;
}

mat3 mat3_multiply(mat3 egy, mat3 ketto)
{
    mat3 mat = mat3_create(0);

    for (int i = 0; i < 3; i++) //row
        for (int j = 0; j < 3; j++) //column
            for (int k = 0; k < 3; k++) //index
                mat.data[j * 3 + i] += egy.data[k * 3 + i] * ketto.data[j * 3 + k];

    return mat;
}


float mat3_determinant(mat3* mat)
{
    float det = 0;

    det += mat->data[0] * (mat->data[4] * mat->data[8] - mat->data[5] * mat->data[7]);
    det -= mat->data[3] * (mat->data[1] * mat->data[8] - mat->data[2] * mat->data[7]);
    det += mat->data[6] * (mat->data[1] * mat->data[5] - mat->data[2] * mat->data[4]);

    return det;
}

mat3 mat3_inverse(mat3 mat)
{
    float szam = mat3_determinant(&mat);
    if (szam == 0)//nincs inverze
        return mat;
    szam = 1 / szam;

    float* d = mat.data;

    mat3 adjugate = mat3_transpose(mat3_create2((float[]) {
        d[4]*d[8]-d[7]*d[5],    -d[3]*d[8]+d[5]*d[6],   d[3]*d[7]-d[4]*d[6],
        -d[1]*d[8]+d[7]*d[2],   d[0]*d[8]-d[6]*d[2],    -d[0]*d[7]+d[6]*d[1],
        d[1]*d[5]-d[4]*d[2],    -d[0]*d[5]+d[3]*d[2],   d[0]*d[4]-d[3]*d[1]
    }));

    for (int i = 0; i < 9; i++)
        adjugate.data[i] *= szam;

    return adjugate;
}

void mat3_print(mat3* mat)
{
    for (int i = 0; i < 3; i++) {
        printf("%.2f %.2f %.2f\n", mat->data[i], mat->data[3 + i], mat->data[6 + i]);
    }
}