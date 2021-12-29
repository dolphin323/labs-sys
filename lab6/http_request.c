#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "http_request.h"

#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

http_request_t *parse_request(char *data)
{
    http_request_t *res = malloc(sizeof(http_request_t));

    gettimeofday(&res->request_time, &res->request_timezone);

    char *method_end_ptr = strstr(data, " ");
    if (method_end_ptr == NULL)
    {
        free(res);
        return NULL;
    }

    memcpy(res->method, data, min(method_end_ptr - data, 8));
    res->method[min(method_end_ptr - data, 8)] = 0;
    char *uri_end_ptr = strstr(method_end_ptr + 1, " ");
    if (uri_end_ptr == NULL)
    {
        free(res);
        return NULL;
    }

    char tmp[MY_PATH_LEN + 1];
    memcpy(tmp, method_end_ptr + 1, uri_end_ptr - method_end_ptr - 1);
    tmp[uri_end_ptr - method_end_ptr - 1] = 0;
    res->path = strdup(tmp);

    char *start_file_pos = strrchr(res->path, '/');
    if (start_file_pos == NULL)
    {
        res->file = 0;
    }
    else
    {
        char tmp_file[MY_PATH_LEN + 1];
        memcpy(tmp_file, start_file_pos + 1, res->path + strlen(res->path) - start_file_pos - 1);
        tmp_file[res->path + strlen(res->path) - start_file_pos - 1] = 0;
        res->file = strdup(tmp_file);
        *(start_file_pos + 1) = 0;
    }

    char *host_start_ptr = strstr(method_end_ptr, "Host: ");
    res->host = NULL;
    if (host_start_ptr != NULL)
    {
        char *host_end_ptr = strstr(host_start_ptr, "\r\n");
        if (host_end_ptr != NULL)
        {
            char tmp_host[MY_PATH_LEN + 1];
            memcpy(tmp_host, host_start_ptr + 6, host_end_ptr - host_start_ptr - 6);
            tmp_host[host_end_ptr - host_start_ptr - 6] = 0;
            res->host = strdup(tmp_host);
        }
    }

    return res;
}

void destroy_http_request(http_request_t *self)
{
    free(self->host);
    free(self->path);
    free(self->file);
    free(self);
}
