#include "vec3.h"

#include <math.h>

vec3 vec3_create(float szam)
{
    return (vec3){ szam, szam, szam };
}
vec3 vec3_create2(float x, float y, float z)
{
    return (vec3){ x, y, z };
}

vec3 vec3_sum(vec3 egy, vec3 katto)
{
    return (vec3){ egy.x + katto.x, egy.y + katto.y, egy.z + katto.z }; 
}
vec3 vec3_subtract(vec3 egy, vec3 katto)
{
    return (vec3){ egy.x - katto.x, egy.y - katto.y, egy.z - katto.z };
}
float vec3_dot(vec3 egy, vec3 katto)
{
    return egy.x * katto.x + egy.y * katto.y + egy.z * katto.z;
}
vec3 vec3_cross(vec3 egy, vec3 katto)
{
    return vec3_create2(egy.y * katto.z - egy.z * katto.y, egy.z * katto.x - egy.x * katto.z, egy.x * katto.y - egy.y * katto.x);
}

float vec3_sqrMagnitude(vec3 vec)
{
    return vec.x * vec.x + vec.y * vec.y + vec.z * vec.z;
}

float vec3_magnitude(vec3 vec)
{
    return sqrtf(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
}

vec3 vec3_normalize(vec3 vec)
{
    float length = vec3_magnitude(vec);
    return (vec3){ vec.x / length, vec.y / length, vec.z / length };
}
vec3 vec3_scale(vec3 vec, float scalar)
{
    return (vec3){ vec.x * scalar, vec.y * scalar, vec.z * scalar };
}

void vec3_print(vec3* vec) 
{
    printf("(%.2f; %.2f; %.2f)\n", vec->x, vec->y, vec->z);
}
