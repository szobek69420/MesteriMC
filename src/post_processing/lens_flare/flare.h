#ifndef FLARE_H
#define FLARE_H

//sauce: ThinMatrix
//https://youtu.be/OiMRdkhvwqg?si=8gh4pnnRwoP8_tZe
//https://youtu.be/LMpw7foANNA?si=8mqjwuxBAFy_WTcg

#define FLARE_TEXTURE_COUNT 9
#define FLARE_QUERY_COUNT 20
#define FLARE_MAX_SAMPLES_PASSED_FULL_HD 7200 /*ez egyelore csak 1920x1080-nal ennyi, ezzel majd kell vmit kezdeni*/

#include "../../glm2/vec3.h"
#include "../../glm2/mat4.h"
#include "../../camera/camera.h"
#include "../../shader/shader.h"

struct flare;
typedef struct flare flare;

/*
miert nem eleg egy query?

mert az opengl par frame-el le lehet maradva (nekem 3 frame-mel vannak lemaradva a lekerdezesek), szoval ha frame-enkent kerdezunk le, akkor nem hagyjuk befejezodni az elozo lekerdezest
ez most egy sor akar lenni
*/

flare* flare_create(float spacing);
void flare_destroy(flare* sus);

void flare_render(flare* sus, mat4* projectionView, vec3 cumPos, vec3 sunDir, float aspectYX);
void flare_queryQueryResult(flare* sus);
void flare_query(flare* sus, mat4* projectionView, vec3 cumPos, vec3 sunDir, float aspectYX);

void flare_update(flare* sus);

#endif