#pragma once

#include <time.h>
#include <sys/time.h>

#define MY_PATH_LEN 4096

typedef struct {
    char method[8];
    char * host;
    char * path;
    char * file;
    struct timeval request_time;
    struct timezone request_timezone;
} http_request_t;

http_request_t * parse_request(char *data);
void destroy_http_request(http_request_t *self);
