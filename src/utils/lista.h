#ifndef LISTA_H
#define LISTA_H

#include <stdlib.h>
#include <stdio.h>

#define lista_element_of(TYPE) struct xXx_RobuxHaver_xXx_##TYPE{\
	TYPE data;\
	struct xXx_RobuxHaver_xXx_##TYPE * next;\
}

//az op1 es op2 nem tarolnak informaciot, csak a kesobbi muveletekhez kellenek
#define lista_of(TYPE) struct { \
	lista_element_of(TYPE) * head, *op1, *op2;\
    size_t size; \
}

#define lista_init(LIST_P) do{\
	(LIST_P)->head=NULL;\
	(LIST_P)->size = 0;\
} while(0)

#define lista_clear(LIST_P) do{\
	(LIST_P)->op1=(LIST_P)->head;\
	while((LIST_P)->op1!=NULL)\
	{\
		(LIST_P)->op2=(LIST_P)->op1;\
		(LIST_P)->op1=(LIST_P)->op1->next;\
		free((LIST_P)->op2);\
	}\
	(LIST_P)->head = NULL;\
	(LIST_P)->size = 0;\
} while(0)

//nem ellenorzi, hogy ervenyes-e az index
#define lista_push(LIST_P, INDEX, VALUE) do{\
	if ((LIST_P)->size == 0)\
	{\
		(LIST_P)->op2 = malloc(sizeof(*(LIST_P)->head));\
		(LIST_P)->op2->data = (VALUE);\
		(LIST_P)->op2->next = NULL;\
		(LIST_P)->head = (LIST_P)->op2;\
		(LIST_P)->size = 1;\
	}\
	else if ((INDEX) == 0)\
	{\
		(LIST_P)->op2 = malloc(sizeof(*(LIST_P)->head));\
		(LIST_P)->op2->data = (VALUE);\
		(LIST_P)->op2->next = (LIST_P)->head;\
		(LIST_P)->head = (LIST_P)->op2;\
		(LIST_P)->size++;\
	}\
	else\
	{\
		(LIST_P)->op1 = (LIST_P)->head;\
		for (int __i = 1; __i < (INDEX); __i++)\
			(LIST_P)->op1 = (LIST_P)->op1->next;\
		(LIST_P)->op2 = malloc(sizeof(*(LIST_P)->head)); \
		(LIST_P)->op2->data = (VALUE);\
		(LIST_P)->op2->next = (LIST_P)->op1->next;\
		(LIST_P)->op1->next = (LIST_P)->op2;\
		(LIST_P)->size++;\
	}\
} while(0)

#define lista_push_back(LIST_P, VALUE) lista_push((LIST_P), (LIST_P)->size, (VALUE))

#define lista_remove(LIST_P, VALUE) do{\
	(LIST_P)->op1=NULL;\
	(LIST_P)->op2 = (LIST_P)->head;\
	while((LIST_P)->op2!=NULL)\
	{\
		if ((LIST_P)->op2->data == (VALUE))\
		{\
			if((LIST_P)->op1==NULL)\
				(LIST_P)->head = (LIST_P)->op2->next;\
			else\
				(LIST_P)->op1->next = (LIST_P)->op2->next;\
			free((LIST_P)->op2);\
			(LIST_P)->size--;\
			break;\
		}\
		(LIST_P)->op1=(LIST_P)->op2;\
		(LIST_P)->op2=(LIST_P)->op2->next;\
	}\
} while(0)

//nem ellenorzi, hogy ervenyes-e az index
#define lista_remove_at(LIST_P, INDEX) do{\
	(LIST_P)->op2 = (LIST_P)->head;\
	(LIST_P)->op1 = NULL;\
	for (int ___i = 0; ___i < (INDEX); ___i++)\
	{\
		(LIST_P)->op1 = (LIST_P)->op2;\
		(LIST_P)->op2 = (LIST_P)->op2->next;\
	}\
	if ((INDEX) == 0 && (LIST_P)->size == 1)\
		(LIST_P)->head = NULL;\
	else if ((LIST_P)->op1 == NULL)\
		(LIST_P)->head = (LIST_P)->op2->next;\
	else\
		(LIST_P)->op1->next = (LIST_P)->op2->next;\
	free((LIST_P)->op2);\
	(LIST_P)->size--;\
} while(0)

//a buffer egy TYPE pointer, amibe beleirja az INDEX-edik elem tartalmat (next nelkul)
//nem ellenorzi, hogy ervenyes-e az index
#define lista_at(LIST_P, INDEX, BUFFER_P) do{\
	(LIST_P)->op1 = (LIST_P)->head;\
	for(int ___i=0;___i<(INDEX);___i++)\
		(LIST_P)->op1 = (LIST_P)->op1->next;\
	*(BUFFER_P)=(LIST_P)->op1->data;\
} while(0)

//nem ellenorzi, hogy ervenyes-e az index
#define lista_set(LIST_P, INDEX, VALUE) do{\
	(LIST_P)->op1=(LIST_P)->head;\
	for (int ___i = 0; ___i < (INDEX); ___i++)\
		(LIST_P)->op1 = (LIST_P)->op1->next;\
	(LIST_P)->op1->data=(VALUE)\
} while(0)

#endif