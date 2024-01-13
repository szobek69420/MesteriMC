#include "vector.h"
#include <stdio.h>

vector* vector_create(size_t capacity)
{

	vector* vec = malloc(sizeof(vector));
	if (vec == NULL)
	{
		printf("failed to allocate memory for vector\n");
		exit(1);
	}
	if (capacity > 0)
	{
		vec->data = (void**)malloc(capacity * sizeof(void*));
		if (vec->data == NULL)
		{
			printf("failed to allocate memory for vector data\n");
			exit(1);
		}
		vec->capacity = capacity;
		vec->size = 0;
	}
	else
	{
		vec->data = NULL;
		vec->capacity = 0;
		vec->size = 0;
	}
	return vec;
}
void vector_destroy(vector* vec)
{
	if (vec == NULL)
		return;
	free(vec->data);
	free(vec);
}

void* vector_get(vector* vec, size_t idx)
{
	if (vec == NULL || idx >= vec->size)
		return NULL;
	return vec->data[idx];
}
void vector_set(vector* vec, size_t idx, void* value)
{
	if (vec == NULL || idx >= vec->size)
		return;
	vec->data[idx] = value;
}
void vector_push_back(vector* vec, void* value)
{
	if (vec == NULL)
		return;
	if (vec->size == vec->capacity)
	{
		vec->capacity = vec->capacity == 0 ? 1 : vec->capacity * 2;
		vec->data = (void**)realloc(vec->data, sizeof(void*) * vec->capacity);
		if (vec->data == NULL)
		{
			printf("failed to allocate memory for vector data\n");
			exit(1);
		}
	}
	vec->data[vec->size++] = value;
}
void* vector_pop_back(vector* vec)
{
	if (vec == NULL || vec->size == 0)
		return NULL;
	vec->size--;
	return vec->data[vec->size];
}
void vector_insert(vector* vec, size_t idx, void* value)
{
	if (vec == NULL || idx > vec->size)
		return;
	if (vec->size == vec->capacity)
	{
		vec->capacity = vec->capacity == 0 ? 1 : vec->capacity * 2;
		vec->data = (void**)realloc(vec->data, sizeof(void*) * vec->capacity);
		if (vec->data == NULL)
		{
			printf("failed to allocate memory for vector data\n");
			exit(1);
		}
	}
	for (size_t i = vec->size; i > idx; i--)
		vec->data[i] = vec->data[i - 1];
	vec->data[idx] = value;
	vec->size++;
}
bool vector_contains(vector* vec, void* value)
{
	if (vec == NULL)
		return false;
	for (size_t i = 0; i < vec->size; i++)
		if (vec->data[i] == value)
			return true;
	return false;
}
int vector_index_of(vector* vec, void* value)
{
	if (vec == NULL)
		return -1;
	for (size_t i = 0; i < vec->size; i++)
		if (vec->data[i] == value)
			return i;
	return -1;
}
void vector_remove_at(vector* vec, size_t idx)
{
	if (vec == NULL || idx >= vec->size)
		return;
	for (size_t i = idx; i < vec->size - 1; i++)
		vec->data[i] = vec->data[i + 1];
	vec->size--;
}
void vector_remove(vector* vec, void* value)
{
	if (vec == NULL)
		return;
	for (size_t i = 0; i < vec->size; i++)
	{
		if (vec->data[i] == value)
		{
			vector_remove_at(vec, i);
			return;
		}
	}
}
void vector_reserve(vector* vec, size_t capacity)
{
	if (vec == NULL)
		return;
	if (capacity > vec->capacity)
	{
		vec->data = (void**)realloc(vec->data, sizeof(void*) * capacity);
		if (vec->data == NULL)
		{
			printf("failed to allocate memory for vector data\n");
			exit(1);
		}
		vec->capacity = capacity;
	}
}
size_t vector_size(vector* vec)
{
	if (vec == NULL)
		return 0;
	return vec->size;
}
void vector_clear(vector* vec)
{
	if (vec == NULL)
		return;
	free(vec->data);
	vec->data = NULL;
	vec->capacity = 0;
	vec->size = 0;
}
