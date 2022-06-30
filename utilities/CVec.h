#ifndef CVECTOR_H_
#define CVECTOR_H_

#include <malloc.h>

#define CVECTOR_INIT_CAPACITY 4

typedef struct{
    int* data;
    int size;
    int capacity;
    size_t element_size;
} cvector;


void cvector_init(cvector* __v)
{
    __v->capacity = CVECTOR_INIT_CAPACITY;
    __v->size = 0;
    __v->data = (int*) malloc(CVECTOR_INIT_CAPACITY * sizeof(int));
}

int cvector_size(cvector* __v)
{
    return __v->size;
}

void cvector_resize(cvector* __v, int __newCap)
{
    __v->capacity = __newCap;
    __v->data = (int*)realloc(__v->data, __newCap * sizeof(int));
}


void cvector_push(cvector* __v, int __data)
{
    if (__v->size >= __v->capacity)
        cvector_resize(__v, __v->capacity + __v->capacity / 2);

    __v->data[__v->size] = __data;

    __v->size++;
}

int cvector_get(cvector* __v, int __index)
{
    if (__index < 0 || __index > __v->size - 1 || __v->size <= 0)
        return 0;

    return __v->data[__index];
}

#endif /* CVECTOR_H_ */
