#pragma once

#include <stdlib.h>
#include <netinet/in.h>

#include "http_request.h"
#include "connection_queue.h"

ssize_t read_from_client(int client_socket_fd, char * buff, size_t buff_len);
void * handler(void * data);
