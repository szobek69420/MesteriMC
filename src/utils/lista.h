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

#define lista_init(LIST) do{\
	(LIST).head=NULL;\
	(LIST).size = 0;\
} while(0)

#define lista_clear(LIST) do{\
	(LIST).op1=(LIST).head;\
	while((LIST).op1!=NULL)\
	{\
		(LIST).op2=(LIST).op1;\
		(LIST).op1=(LIST).op1->next;\
		free((LIST).op2);\
	}\
	(LIST).head = NULL;\
	(LIST).size = 0;\
} while(0)

//nem ellenorzi, hogy ervenyes-e az index
#define lista_push(LIST, INDEX, VALUE) do{\
	if ((LIST).size == 0)\
	{\
		(LIST).op2 = malloc(sizeof(*(LIST).head));\
		(LIST).op2->data = (VALUE);\
		(LIST).op2->next = NULL;\
		(LIST).head = (LIST).op2;\
		(LIST).size = 1;\
	}\
	else if ((INDEX) == 0)\
	{\
		(LIST).op2 = malloc(sizeof(*(LIST).head));\
		(LIST).op2->data = (VALUE);\
		(LIST).op2->next = (LIST).head;\
		(LIST).head = (LIST).op2;\
		(LIST).size++;\
	}\
	else\
	{\
		(LIST).op1 = (LIST).head;\
		for (int __i = 1; __i < (INDEX); __i++)\
			(LIST).op1 = (LIST).op1->next;\
		(LIST).op2 = malloc(sizeof(*(LIST).head)); \
		(LIST).op2->data = (VALUE);\
		(LIST).op2->next = (LIST).op1->next;\
		(LIST).op1->next = (LIST).op2;\
		(LIST).size++;\
	}\
} while(0)

#define lista_push_back(LIST, VALUE) lista_push((LIST), (LIST).size, (VALUE))

#define lista_remove(LIST, VALUE) do{\
	(LIST).op1=NULL;\
	(LIST).op2 = (LIST).head;\
	while((LIST).op2!=NULL)\
	{\
		if ((LIST).op2->data == (VALUE))\
		{\
			if((LIST).op1==NULL)\
				(LIST).head = (LIST).op2->next;\
			else\
				(LIST).op1->next = (LIST).op2->next;\
			free((LIST).op2);\
			(LIST).size--;\
			break;\
		}\
		(LIST).op1=(LIST).op2;\
		(LIST).op2=(LIST).op2->next;\
	}\
} while(0)

//nem ellenorzi, hogy ervenyes-e az index
#define lista_remove_at(LIST, INDEX) do{\
	(LIST).op2 = (LIST).head;\
	(LIST).op1 = NULL;\
	for (int ___i = 0; ___i < (INDEX); ___i++)\
	{\
		(LIST).op1 = (LIST).op2;\
		(LIST).op2 = (LIST).op2->next;\
	}\
	if ((INDEX) == 0 && (LIST).size == 1)\
		(LIST).head = NULL;\
	else if ((LIST).op1 == NULL)\
		(LIST).head = (LIST).op2->next;\
	else\
		(LIST).op1->next = (LIST).op2->next;\
	free((LIST).op2);\
	(LIST).size--;\
} while(0)

//a buffer egy TYPE pointer, amibe beleirja az INDEX-edik elem tartalmat (next nelkul)
//nem ellenorzi, hogy ervenyes-e az index
#define lista_at(LIST, INDEX, BUFFER_P) do{\
	(LIST).op1 = (LIST).head;\
	for(int ___i=0;___i<(INDEX);___i++)\
		(LIST).op1 = (LIST).op1->next;\
	*(BUFFER_P)=(LIST).op1->data;\
} while(0)

//nem ellenorzi, hogy ervenyes-e az index
#define lista_set(LIST, INDEX, VALUE) do{\
	(LIST).op1=(LIST).head;\
	for (int ___i = 0; ___i < (INDEX); ___i++)\
		(LIST).op1 = (LIST).op1->next;\
	(LIST).op1->data=(VALUE);\
} while(0)

#endif