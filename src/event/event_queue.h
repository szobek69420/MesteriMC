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

#endif