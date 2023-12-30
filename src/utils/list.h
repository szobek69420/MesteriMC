#ifndef LIST_H
#define LIST_H

struct listElement {
	void* data;
	struct listElement* next;
};

typedef struct listElement listElement;

typedef struct {
	listElement* head;
	unsigned int size;
} list;

list list_create();

void list_clear(list* lista);

void list_push(list* lista, unsigned int index, void* data);

void list_push_back(list* lista, void* data);

void list_remove(list* lista, void* data);

void list_remove_at(list* lista, unsigned int index);

void* list_get(list* lista, unsigned int index);

void list_set(list* lista, unsigned int index, void* data);

#endif