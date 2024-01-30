//sequential vector
#ifndef SEQTOR_H
#define SEQTOR_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// letrehoz egy TYPE tipusu vektort (nincs inicializalva, azt a seqtor_init vegzi)
#define seqtor_of(TYPE) struct { \
    TYPE *data; \
    size_t size; \
    size_t capacity; \
}

//initializes the vector. without this, the vector won't be initialized
//a capacity ne legyen 0 mert az nemtom mit baszhat el
#define seqtor_init(VEC, CAPACITY) do { \
    (VEC).data = malloc((CAPACITY) * sizeof(*(VEC).data)); \
    if (!(VEC).data) \
    { \
        fputs("seqtor_init failed!\n", stderr); \
        abort(); \
    } \
    (VEC).size = 0; \
    (VEC).capacity = (CAPACITY); \
} while (0)

#define seqtor_size(VEC) (VEC).size

#define seqtor_capacity(VEC) (VEC).capacity

#define seqtor_isEmpty(VEC) ((VEC).size == 0)

//adds VAL to the end of the vector
#define seqtor_push_back(VEC, VAL) do { \
    if ((VEC).size + 1 > (VEC).capacity) \
    { \
        size_t _n = (VEC).capacity * 2; \
        void *_p = realloc((VEC).data, _n * sizeof(*(VEC).data)); \
        if (!_p) \
        { \
            fputs("seqtor_push_back failed!\n", stderr); \
            abort(); \
        } \
        (VEC).data = _p; \
        (VEC).capacity = _n; \
    } \
    (VEC).data[seqtor_size(VEC)] = (VAL); \
    (VEC).size ++; \
} while (0)

//removes the last element of the vector
//doesn't return anything
#define seqtor_pop_back(VEC) do{ \
    if((VEC).size>0) (VEC).size--;\
    if((VEC).size<=(VEC).capacity/2&&(VEC).size!=0) \
    { \
        (VEC).capacity/=2;\
        void *_p = realloc((VEC).data, (VEC).capacity * sizeof(*(VEC).data)); \
        if (!_p) \
        { \
            fputs("seqtor_pop_back failed!\n", stderr); \
            abort(); \
        } \
        (VEC).data = _p; \
    }\
} while(0)

//inserts VALUE into the vector. the index of the inserted element will be INDEX
//nem ellenorzi, hogy helyes-e az index
#define seqtor_insert(VEC, INDEX, VALUE) do{ \
    if((VEC).size==(VEC).capacity) \
    { \
        (VEC).capacity*=2; \
        void *_p = realloc((VEC).data, (VEC).capacity * sizeof(*(VEC).data)); \
        if (!_p) \
        { \
            fputs("sextor_insert failed!\n", stderr); \
            abort(); \
        } \
        (VEC).data = _p; \
    } \
    for (size_t _i = (VEC).size; _i > (INDEX); _i--) \
        (VEC).data[_i] = (VEC).data[_i - 1]; \
    (VEC).data[(INDEX)] = (VALUE); \
    (VEC).size++; \
} while(0)

//removes the element with the index of INDEX from the vector
//nem ellenorzi, hogy helyes-e az index
#define seqtor_remove_at(VEC, INDEX) do{ \
    for (size_t _i = (INDEX); _i < (VEC).size - 1; _i++) \
        (VEC).data[_i] = (VEC).data[_i + 1]; \
    (VEC).size--; \
    if((VEC).size<=(VEC).capacity/2&&(VEC).size!=0) \
    { \
        (VEC).capacity/=2;\
        void *_p = realloc((VEC).data, (VEC).capacity * sizeof(*(VEC).data)); \
        if (!_p) \
        { \
            fputs("seqtor_remove_at failed!\n", stderr); \
            abort(); \
        } \
        (VEC).data = _p; \
    }\
} while(0)

//removes the first element with value from the vector
#define seqtor_remove(VEC, VALUE) do{\
    for(size_t _i2=0;_i2<(VEC).size;_i2++) \
    { \
        if((VALUE)==(VEC).data[_i2]) \
        { \
            seqtor_remove_at((VEC), _i2); \
            break; \
        } \
    } \
} while(0)

//copies content of SRC into DST
//deletes the content of DST if necessary
#define seqtor_copy(DST, SRC) do { \
    if ((DST).capacity != 0) \
        free((DST).data); \
    (DST).size = (SRC).size; \
    (DST).capacity = (SRC).capacity; \
    (DST).data = malloc((DST).capacity * sizeof(*(DST).data)); \
    if ((DST).data == NULL) \
    { \
        fputs("seqtor_copy failed!\n", stderr); \
        abort(); \
    } \
    memcpy((DST).data, (SRC).data, (DST).capacity * sizeof(*(DST).data)); \
} while(0)


#define seqtor_at(VEC, INDEX) (VEC).data[INDEX]

#define seqtor_front(VEC) (VEC).data[0]

#define seqtor_back(VEC) (VEC).data[seqtor_size(VEC) - 1]

//kiuriti a vektort, es az elso elem kivetelevel mindent felszabadit (capacity=1)
#define seqtor_clear(VEC) do { \
    (VEC).size = 0; \
    (VEC).capacity = 1; \
    void* __p=realloc((VEC).data, sizeof(*(VEC).data)); \
    if(__p==NULL) \
    { \
        fputs("seqtor_clear failed!\n", stderr); \
            abort(); \
    } \
} while(0)

//ha nem hasznalod tobbe a vektort, akkor hivd meg ezt (csak akkor)
//mig a seqtor_clear 1 kapacitasura uriti a vektort, ez mindentol megszabadul
#define seqtor_destroy(VEC) do{ \
    free((VEC).data);\
    (VEC).data=NULL;\
    (VEC).size = 0; \
    (VEC).capacity = 0; \
}while(0)

#endif 