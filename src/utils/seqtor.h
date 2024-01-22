//sequential vector
#ifndef SEQTOR_H
#define SEQTOR_H

#include <stdlib.h>
#include <stdio.h>

// letrehoz egy TYPE tipusu vektort (nincs inicializalva, azt a seqtor_init vegzi)
#define seqtor_of(TYPE) struct { \
    TYPE *data; \
    size_t size; \
    size_t capacity; \
}

//a capacity ne legyen 0 mert az nemtom mit baszhat el
#define seqtor_init(VEC, CAPACITY) do { \
    (VEC).data = malloc((CAPACITY) * sizeof(*(VEC).data)); \
    if (!(VEC).data) { \
        fputs("malloc failed!\n", stderr); \
        abort(); \
    } \
    (VEC).size = 0; \
    (VEC).capacity = (CAPACITY); \
} while (0)

#define seqtor_size(VEC) (VEC).size

#define seqtor_capacity(VEC) (VEC).capacity

#define seqtor_isEmpty(VEC) ((VEC).size == 0)

#define seqtor_push_back(VEC, VAL) do { \
    if ((VEC).size + 1 > (VEC).capacity) { \
        size_t _n = (VEC).capacity * 2; \
        void *_p = realloc((VEC).data, _n * sizeof(*(VEC).data)); \
        if (!_p) { \
            fputs("realloc failed!\n", stderr); \
            abort(); \
        } \
        (VEC).data = _p; \
        (VEC).capacity = _n; \
    } \
    (VEC).data[seqtor_size(VEC)] = (VAL); \
    (VEC).size ++; \
} while (0)

#define seqtor_pop_back(VEC) do{ \
    if((VEC).size>0) (VEC).size--;\
    if((VEC).size<=(VEC).capacity/2&&(VEC).size!=0) { \
        (VEC).capacity/=2;\
        void *_p = realloc((VEC).data, (VEC).capacity * sizeof(*(VEC).data)); \
        if (!_p) { \
            fputs("realloc failed!\n", stderr); \
            abort(); \
        } \
        (VEC).data = _p; \
    }\
} while(0)

//nem ellenorzi, hogy helyes-e az index
#define seqtor_insert(VEC, INDEX, VALUE) do{ \
    if((VEC).size==(VEC).capacity) \
    { \
        (VEC).capacity*=2; \
        void *_p = realloc((VEC).data, (VEC).capacity * sizeof(*(VEC).data)); \
        if (!_p) { \
            fputs("realloc failed!\n", stderr); \
            abort(); \
        } \
        (VEC).data = _p; \
    } \
    for (size_t _i = (VEC).size; _i > (INDEX); _i--) \
        (VEC).data[_i] = (VEC).data[_i - 1]; \
    (VEC).data[(INDEX)] = (VALUE); \
    (VEC).size++; \
} while(0)

//nem ellenorzi, hogy helyes-e az index
#define seqtor_remove_at(VEC, INDEX) do{ \
    for (size_t _i = (INDEX); _i < (VEC).size - 1; _i++) \
        (VEC).data[_i] = (VEC).data[_i + 1]; \
    (VEC).size--; \
    if((VEC).size<=(VEC).capacity/2&&(VEC).size!=0) { \
        (VEC).capacity/=2;\
        void *_p = realloc((VEC).data, (VEC).capacity * sizeof(*(VEC).data)); \
        if (!_p) { \
            fputs("realloc failed!\n", stderr); \
            abort(); \
        } \
        (VEC).data = _p; \
    }\
} while(0)

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

#define seqtor_at(VEC, INDEX) (VEC).data[INDEX]

#define seqtor_front(VEC) (VEC).data[0]

#define seqtor_back(VEC) (VEC).data[seqtor_size(VEC) - 1]

#define seqtor_clear(VEC) do { \
    (VEC).size = 0; \
    (VEC).capacity = 1; \
    realloc((VEC).data, sizeof(*(VEC).data)); \
} while(0)

#endif 