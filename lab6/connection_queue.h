#pragma once

#include <pthread.h>
#include <semaphore.h>
#include <netinet/in.h>

#include "http_request.h"

typedef struct {
    uint16_t port;
    char root_path[1024];
    int num_threads;
    int queue_length;
    char log_path[1024];
} config_t;

typedef struct connection {
    struct connection *next;
    int client_fd;
    struct sockaddr_in *client_address;
    http_request_t *request;
} connection_t;

typedef struct {
    connection_t *start;
    connection_t *end;
    pthread_mutex_t lock;
    sem_t is_empty_queue;
    config_t *config_t;
    int fd_log;
} connection_queue_t;

connection_queue_t * connection_queue_init(void);
int connection_queue_push(connection_queue_t * self, connection_t * conn);
connection_t * connection_queue_pull(connection_queue_t * self);
void connection_queue_destroy(connection_queue_t * self);
