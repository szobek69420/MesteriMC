#include "event_queue.h"

#include <string.h>

static event_queue queue;

void event_queue_init()
{
    queue.front = queue.size = 0;
    queue.back = EVENT_QUEUE_MAX_SIZE - 1;
    memset(queue.data, 0, sizeof(event) * EVENT_QUEUE_MAX_SIZE);
}

void event_queue_push(event e)
{
    if (event_queue_is_full())
        return;
    queue.back = (queue.back + 1) % EVENT_QUEUE_MAX_SIZE;
    queue.data[queue.back] = e;
    queue.size++;
}
event event_queue_poll()
{
    if (event_queue_is_empty())
        return (event){ 0 };
    event e = queue.data[queue.front];
    queue.front = (queue.front + 1) % EVENT_QUEUE_MAX_SIZE;
    queue.size--;
    return e;
}

int event_queue_is_empty()
{
    return queue.size == 0;
}
int event_queue_is_full()
{
    return queue.size == EVENT_QUEUE_MAX_SIZE;
}
int event_queue_get_size()
{
    return queue.size;
}

event event_queue_front()
{
    if (event_queue_is_empty())
        return (event){ 0 };
    return queue.data[queue.front];
}
event event_queue_back()
{
    if (event_queue_is_empty())
        return (event){ 0 };
    return queue.data[queue.back];
}

void event_queue_swap_back(event e)
{
    if (event_queue_is_empty())
        return;
    queue.data[queue.back] = e;
}