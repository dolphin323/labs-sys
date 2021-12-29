#include <stdbool.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>

#include "handle.h"
#include "http_request.h"

#define SEND_BUFF_SIZE 1024 * 1024 * 10
#define MY_MAX 32767

ssize_t get_content_length(char *data, size_t len);
bool ends_with_two_new_lines(char *data, size_t len);
ssize_t make_http_response(char *buf, size_t len, int status, char *headers, char *body, size_t len_body);

static bool check_if_file_exists(char *path)
{
    struct stat stat_buf;
    if (stat(path, &stat_buf) < 0)
    {
        if (errno == ENOENT)
        {
            return false;
        }
        else
        {
            exit(errno);
        }
    }
    return S_ISREG(stat_buf.st_mode);
    ;
}

ssize_t read_from_client(int client_socket_fd, char *buff, size_t buff_len)
{
    size_t cur_pos = 0;
    ssize_t full_length = MY_MAX;
    do
    {
        ssize_t numRead = recv(client_socket_fd, buff + cur_pos, buff_len - cur_pos, 0);
        if (numRead <= 0)
        {
            return -1;
        }
        cur_pos += numRead;
        if (full_length == MY_MAX)
        {
            full_length = get_content_length(buff, cur_pos);
        }
        if (full_length == MY_MAX && ends_with_two_new_lines(buff, cur_pos))
        {
            break;
        }
    } while (cur_pos < full_length);
    return cur_pos;
}

int write_to_client(int client_socket_fd, char *buff, size_t buff_len)
{
    while (buff_len > 0)
    {
        ssize_t numSent = send(client_socket_fd, buff, buff_len, 0);
        if (numSent < 0)
        {
            return -1;
        }
        buff += numSent;
        buff_len -= numSent;
    }
    return 0;
}

ssize_t get_content_length(char *data, size_t len)
{
    data[len] = 0;
    char *pos_content = strstr(data, "Content-Length: ");
    ssize_t startKey = pos_content == NULL ? -1 : pos_content - data;
    ssize_t endValue = -1;
    if (pos_content != NULL)
    {
        char *pos_end = strstr(pos_content + 16, "\r\n");
        endValue = pos_end == NULL ? -1 : pos_end - data;
    }
    if (startKey != -1 && endValue != -1)
    {
        return strtoul(pos_content, NULL, 10);
    }
    return MY_MAX;
}

bool ends_with_two_new_lines(char *data, size_t len)
{
    data[len] = 0;
    char *pos_content = strstr(data, "\r\n\r\n");
    if (pos_content != NULL)
    {
        return pos_content + 4 - len == data;
    }
    return false;
}

ssize_t make_http_headers(char *buf, size_t len, const char *f_type, size_t body_length)
{
    ssize_t cur_pos = 0;
    time_t now = time(0);
    char time_buf[1000];
    strftime(time_buf, 1000, "%a, %d %b %Y %H:%M:%S %Z", gmtime(&now));
    char *mime_type = "text/plain";
    if (f_type != NULL)
    {
        if (strcmp(f_type, "htm") == 0)
        {
            mime_type = "text/html";
        }
        else if (strcmp(f_type, "html") == 0)
        {
            mime_type = "text/html";
        }
        else if (strcmp(f_type, "txt") == 0)
        {
            mime_type = "text/txt";
        }
        else if (strcmp(f_type, "jpeg") == 0)
        {
            mime_type = "image/jpeg";
        }
        else if (strcmp(f_type, "jpg") == 0)
        {
            mime_type = "image/jpeg";
        }
        else if (strcmp(f_type, "mpeg") == 0)
        {
            mime_type = "video/mpeg";
        }
        else if (strcmp(f_type, "png") == 0)
        {
            mime_type = "image/png";
        }
        else
        {
            mime_type = "text/plain";
        }
    }
    int num = snprintf(buf, len, "Content-Length: %lu\r\nConnection: close\r\n"
                                 "Content-Type: %s\r\nDate: %s",
                       body_length, mime_type, time_buf);
    if (num < 0)
    {
        return -1;
    }
    return cur_pos + num;
}

ssize_t get_file_size(int fd)
{
    off_t data_file_size_bytes = lseek(fd, 0, SEEK_END);
    if (data_file_size_bytes < 0)
    {
        return -1;
    }
    if (lseek(fd, 0, SEEK_SET))
    {
        return -1;
    }
    return data_file_size_bytes;
}

ssize_t read_from_file(char *path, void *buff, size_t len)
{
    int fd = open(path, O_RDONLY, 0644);
    if (fd < 0)
    {
        return -1;
    }
    int read_bytes = 0;
    ssize_t real_len = get_file_size(fd);
    ssize_t cur_read = 0;
    while (read_bytes != real_len && (cur_read = read(fd, buff + read_bytes, len - read_bytes)) > 0)
    {
        read_bytes += cur_read;
    }
    return read_bytes;
}

ssize_t make_http_response(char *buf, size_t len, int status, char *headers, char *body, size_t len_body)
{
    ssize_t cur_pos = 0;
    const char *version = "HTTP/1.1 ";
    memcpy(buf + cur_pos, version, strlen(version));
    cur_pos += strlen(version);
    int num = snprintf(buf + cur_pos, len - cur_pos, "%d ", status);
    if (num < 0)
    {
        return -1;
    }
    cur_pos += num;
    char *r_phrase;
    switch (status)
    {
    case 200:
        r_phrase = "OK";
        break;
    case 403:
        r_phrase = "Forbidden";
        break;
    case 503:
        r_phrase = "Service Unavailable";
        break;
    default:
        r_phrase = "Not Found";
        break;
    }
    memcpy(buf + cur_pos, r_phrase, strlen(r_phrase));
    cur_pos += strlen(r_phrase);
    memcpy(buf + cur_pos, "\r\n", 2);
    cur_pos += 2;
    memcpy(buf + cur_pos, headers, strlen(headers));
    cur_pos += strlen(headers);
    memcpy(buf + cur_pos, "\r\n\r\n", 4);
    cur_pos += 4;
    memcpy(buf + cur_pos, body, len_body);
    cur_pos += len_body;
    return cur_pos;
}

void *handle(void *data)
{
    connection_queue_t *conn_queue = data;
    while (true)
    {
        connection_t *conn = connection_queue_pull(conn_queue);
        dprintf(conn_queue->fd_log, "Get request\n");
        char *buff_all = malloc(SEND_BUFF_SIZE + 1);
        size_t buff_all_len = 0;
        char *buff_header = malloc(SEND_BUFF_SIZE + 1);
        size_t buff_header_len = 0;
        char *body = malloc(SEND_BUFF_SIZE + 1);
        if (strcmp(conn->request->method, "GET") == 0)
        {
            if (strcmp(conn->request->path, "/") == 0 && strlen(conn->request->file) == 0)
            {
                char path[MY_PATH_LEN + 1];
                int num = snprintf(path, MY_PATH_LEN, "%s/%s", conn_queue->config_t->root_path, "index.html");
                if (num < 0)
                {
                    dprintf(conn_queue->fd_log, "Couldn't retrieve path: %s\n", strerror(errno));
                    exit(EXIT_FAILURE);
                }
                path[num] = 0;
                int len = read_from_file(path, body, SEND_BUFF_SIZE);
                if (len < 0)
                {
                    dprintf(conn_queue->fd_log, "Couldn't read from file: %s\n", strerror(errno));
                    exit(EXIT_FAILURE);
                }
                body[len] = 0;
                buff_header_len = make_http_headers(buff_header, SEND_BUFF_SIZE, "html", len);
                buff_header[buff_header_len] = 0;
                buff_all_len = make_http_response(buff_all, SEND_BUFF_SIZE, 200, buff_header, (char *)body, len);
            }
            else
            {
                char path[MY_PATH_LEN + 1];
                int num = snprintf(path, MY_PATH_LEN, "%s/%s/%s", conn_queue->config_t->root_path,
                                   conn->request->path, conn->request->file);
                if (num < 0)
                {
                    dprintf(conn_queue->fd_log, "Couldn't retrieve path: %s\n", strerror(errno));
                    exit(EXIT_FAILURE);
                }
                path[num] = 0;
                if (check_if_file_exists(path))
                {
                    int len = read_from_file(path, body, SEND_BUFF_SIZE);
                    if (len < 0)
                    {
                        dprintf(conn_queue->fd_log, "Couldn't read from file: %s\n", strerror(errno));
                        exit(EXIT_FAILURE);
                    }
                    body[len] = 0;

                    buff_header_len = make_http_headers(buff_header, SEND_BUFF_SIZE, strrchr(path, '.') + 1, len);
                    buff_header[buff_header_len] = 0;
                    buff_all_len = make_http_response(buff_all, SEND_BUFF_SIZE, 200, buff_header, (char *)body, len);
                }
                else
                {
                    char *body_nf_method = "Not found";
                    buff_header_len = make_http_headers(buff_header, SEND_BUFF_SIZE, NULL, strlen(body_nf_method));
                    buff_header[buff_header_len] = 0;
                    buff_all_len = make_http_response(buff_all, SEND_BUFF_SIZE, 404, buff_header, body_nf_method, strlen(body_nf_method));
                }
            }
        }
        else
        {
            char *body_err_method = "Method not allowed";
            buff_header_len = make_http_headers(buff_header, SEND_BUFF_SIZE, NULL, strlen(body_err_method));
            buff_header[buff_header_len] = 0;
            buff_all_len = make_http_response(buff_all, SEND_BUFF_SIZE, 503, buff_header, body_err_method, strlen(body_err_method));
        }
        if (write_to_client(conn->client_fd, buff_all, buff_all_len) < 0)
        {
            dprintf(conn_queue->fd_log, "Couldn't send to client: %s\n", strerror(errno));
        }
        dprintf(conn_queue->fd_log, "Send response\n");
        close(conn->client_fd);
        free(body);
        free(buff_all);
        free(buff_header);
    }
}
