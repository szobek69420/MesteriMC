#ifndef EVENT_QUEUE_H
#define EVENT_QUEUE_H

#include "event.h"

#define EVENT_QUEUE_MAX_SIZE 50

typedef struct {
    int front, back, size;
    event data[EVENT_QUEUE_MAX_SIZE];
} event_queue;

void event_queue_init();

void event_queue_push(event e);
event event_queue_poll();

int event_queue_is_empty();
int event_queue_is_full();
int event_queue_get_size();

event event_queue_front();
event event_queue_back();

//a kepernyo meretenek megvaltoztatasanal kb pixelenkent lefut a callback, ami nagyon gyorsan feltolti a queue-t (es akkor elvesznek a legfrissebb meretek)
//szoval a window resize callback-ben amennyiben az utolso event is egy window resize, csak kicserelem az uj eventre
void event_queue_swap_back(event e);

#endif