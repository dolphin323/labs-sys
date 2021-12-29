#include <stdbool.h>
#include <stdlib.h>

#include "connection_queue.h"

connection_queue_t *connection_queue_init(void)
{
    connection_queue_t *self = malloc(sizeof(connection_queue_t));
    self->end = NULL;
    self->start = NULL;
    pthread_mutex_init(&self->lock, NULL);
    sem_init(&self->is_empty_queue, false, 0);
    return self;
}

int connection_queue_push(connection_queue_t *self, connection_t *conn)
{
    pthread_mutex_lock(&self->lock);
    if (self->start == NULL && self->end == NULL)
    {
        self->start = conn;
        self->end = conn;
    }
    else
    {
        self->end->next = conn;
        self->end = conn;
    }
    conn->next = NULL;
    sem_post(&self->is_empty_queue);
    pthread_mutex_unlock(&self->lock);
    return 0;
}

connection_t *connection_queue_pull(connection_queue_t *self)
{
    connection_t *res;
    sem_wait(&self->is_empty_queue);
    pthread_mutex_lock(&self->lock);
    res = self->start;
    self->start = self->start->next;
    if (self->start == NULL)
    {
        self->end = NULL;
    }
    pthread_mutex_unlock(&self->lock);
    return res;
}

void connection_queue_destroy(connection_queue_t *self)
{
    pthread_mutex_destroy(&self->lock);
    sem_destroy(&self->is_empty_queue);
    connection_t *cur = self->start;
    while (cur != NULL && cur->next != NULL)
    {
        free(cur);
        cur = cur->next;
    }
}
