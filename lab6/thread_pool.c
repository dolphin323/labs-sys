#include <stdlib.h>
#include <pthread.h>

#include "thread_pool.h"

thread_pool_t *thread_pool_init(size_t num_threads, connection_queue_t *conn_queue, handler_t func)
{
    thread_pool_t *self = malloc(sizeof(thread_pool_t));
    self->count_all = num_threads;
    self->count_free = num_threads;
    self->thread_arr = malloc(num_threads * sizeof(pthread_t *));
    self->connection_queue = conn_queue;
    pthread_mutex_init(&self->lock, NULL);

    self->func = func;
    for (size_t i = 0; i < self->count_all; i++)
    {
        self->thread_arr[i] = malloc(sizeof(pthread_t));
        pthread_create(self->thread_arr[i], NULL, self->func, self->connection_queue);
    }
    return self;
}

void thread_pool_destroy(thread_pool_t *self)
{
    for (size_t i = 0; i < self->count_all; i++)
    {
        pthread_join(*self->thread_arr[i], NULL);
    }
    pthread_mutex_destroy(&self->lock);
    free(self->thread_arr);
}