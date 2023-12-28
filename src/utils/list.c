#include <stdlib.h>
#include "list.h"

list lCreate()
{
	list lista;
	lista.head = NULL;
	lista.size = 0;

	return lista;
}

void lClear(list* lista)
{
	listElement* current = lista->head;
	listElement* temp;


	while (current != NULL)
	{
		temp = current;
		current = current->next;
		free(temp);
	}


	lista->head = NULL;
	lista->size = 0;
}

void lPush(list* lista, unsigned int index, void* data)
{
	int i;
	listElement* iterated;
	listElement* newElement;

	if (lista->size < index)
		index = lista->size;

	if (lista->size == 0)
	{
		newElement = (listElement*)malloc(sizeof(listElement));
		newElement->data = data;
		newElement->next = NULL;

		lista->head = newElement;
		lista->size = 1;
	}
	else if (index == 0)
	{
		newElement = (listElement*)malloc(sizeof(listElement));
		newElement->data = data;
		newElement->next = lista->head;

		lista->head = newElement;
		lista->size++;
	}
	else
	{
		iterated = lista->head;

		for (i = 1; i < index; i++)
			iterated = iterated->next;

		newElement = (listElement*)malloc(sizeof(listElement));
		newElement->data = data;
		newElement->next = iterated->next;

		iterated->next = newElement;
		lista->size++;
	}
}


void lPushBack(list* lista, void* data)
{
	lPush(lista, lista->size, data);
}


void lRemove(list* lista, void* data)
{
	listElement* previous = NULL;
	listElement* current = lista->head;

	while (current != NULL)
	{
		if (current->data == data)
		{
			if (previous == NULL)
				lista->head = current->next;
			else
				previous->next = current->next;

			free(current);
			lista->size--;
			break;
		}

		previous = current;
		current = current->next;
	}
}

void lRemoveAt(list* lista, unsigned int index)
{
	int i;
	listElement* previous;
	listElement* current;

	if (index >= lista->size)
		return;

	current = lista->head;
	previous = NULL;

	for (i = 0; i < index; i++)
	{
		previous = current;
		current = current->next;
	}

	if (previous == NULL)
		lista->head = current->next;
	else
		previous->next = current->next;
	free(current);
	lista->size--;
}

void* lGet(list* lista, unsigned int index)
{
	int i;
	listElement* current = lista->head;

	if (index >= lista->size)
		return NULL;

	for (i = 0; i < index; i++)
	{
		current = current->next;
	}

	return current->data;
}

void lSet(list* lista, unsigned int index, void* data)
{
	int i;
	listElement* current = lista->head;

	if (index >= lista->size)
		return;

	for (i = 0; i < index; i++)
		current = current->next;

	current->data = data;
}