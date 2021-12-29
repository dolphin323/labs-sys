#pragma once

#include "connection_queue.h"

typedef void * (*handler_t)(void *);

typedef struct {
    pthread_t **thread_arr;
    size_t count_all;
    size_t count_free;
    pthread_mutex_t lock;
    connection_queue_t *connection_queue;
    handler_t func;
} thread_pool_t;

thread_pool_t * thread_pool_init(size_t num_threads,  connection_queue_t *conn_queue, handler_t func);
void thread_pool_destroy(thread_pool_t *self);
int thread_pool_start(thread_pool_t *self, handler_t func);
