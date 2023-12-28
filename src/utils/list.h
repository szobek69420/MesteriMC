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

list lCreate();

void lClear(list* lista);

void lPush(list* lista, unsigned int index, void* data);

void lPushBack(list* lista, void* data);

void lRemove(list* lista, void* data);

void lRemoveAt(list* lista, unsigned int index);

void* lGet(list* lista, unsigned int index);

void lSet(list* lista, unsigned int index, void* data);

#endif