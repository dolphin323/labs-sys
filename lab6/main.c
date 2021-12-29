#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdbool.h>
#include <signal.h>

#include "handle.h"
#include "connection_queue.h"
#include "thread_pool.h"

#define BACKLOG 5

#define CLIENT_REQ_BUFF_SIZE 2048

#define ERROR(str, arg) \
    if (arg < 0)        \
    {                   \
        perror(str);    \
        exit(errno);    \
    }

#define ERROR_OBJ(str, obj)   \
    if (obj == NULL)          \
    {                         \
        fprintf(stderr, str); \
        exit(errno);          \
    }

connection_t *handle_request(int client_socket_fd, struct sockaddr_in *client_address);

connection_queue_t *conn_queue;
thread_pool_t *thread_pool;

bool shutdown_server = false;

void shutdown_handler(int signo)
{
    shutdown_server = true;
}

void *handle(void *data);

pid_t daemonize()
{
    pid_t child = fork();

    if (child != 0)
    {
        printf("Child PID: %i\n", child);
        exit(EXIT_SUCCESS);
    }

    setsid();

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    open("/dev/null", O_RDONLY);
    open("/dev/null", O_WRONLY);
    dup(STDOUT_FILENO);
    return child;
}

connection_t *connection_new()
{
    connection_t *result = malloc(sizeof(connection_t));
    result->request = NULL;
    result->client_address = NULL;
    result->client_fd = -1;
    result->next = NULL;
    return result;
}

void connection_destroy(connection_t *self)
{
    free(self);
}

config_t get_config_t_from_args(int argc, char *argv[])
{
    int opt;
    config_t config_t = {
        .port = 8080,
        .root_path = ".",
        .num_threads = 4,
        .queue_length = 15,
        .log_path = "log.txt",
    };

    while ((opt = getopt(argc, argv, "p:d:t:q:l:")) != -1)
    {
        switch (opt)
        {
        case 'p':
        {
            config_t.port = strtoul(optarg, NULL, 10);
            if (config_t.port < 0 || config_t.port > 65535)
            {
                fprintf(stderr, "Invalid port, should be > 0 and < 65535\n");
                exit(EXIT_FAILURE);
            }
        }
        break;
        case 'd':
        {
            if (strcmp(optarg, "") == 0)
            {
                fprintf(stderr, "Root server path couldn't be empty\n");
                exit(EXIT_FAILURE);
            }
            memcpy(config_t.root_path, optarg, strlen(optarg));
            config_t.root_path[strlen(optarg)] = 0;
        }
        break;
        case 't':
        {
            unsigned long num_threads = strtoul(optarg, NULL, 10);
            if (num_threads < 0 || num_threads > 16)
            {
                fprintf(stderr, "Invalid number of threads, should be > 0 and < 16\n");
                exit(EXIT_FAILURE);
            }
            config_t.num_threads = (int)num_threads;
        }
        break;
        case 'q':
        {
            unsigned long queue_length = strtoul(optarg, NULL, 10);
            if (queue_length < 0 || queue_length > 128)
            {
                fprintf(stderr, "Invalid length of queue, should be > 0 and < 128\n");
                exit(EXIT_FAILURE);
            }
            config_t.queue_length = (int)queue_length;
        }
        break;
        case 'l':
        {
            if (strcmp(optarg, "") == 0)
            {
                fprintf(stderr, "Log file path couldn't be empty\n");
                exit(EXIT_FAILURE);
            }
            memcpy(config_t.log_path, optarg, strlen(optarg));
            config_t.log_path[strlen(optarg)] = 0;
        }
        break;
        default:
        {
            fprintf(stderr, "Usage: %s [-p port] [-d root path] [-t num threads] "
                            "[-q queue length] [-l log file]\n",
                    argv[0]);
            exit(EXIT_FAILURE);
        }
        }
    }

    return config_t;
}

int main(int argc, char *argv[])
{
    config_t config_t = get_config_t_from_args(argc, argv);

    daemonize();

    pid_t pid = getpid();
    int fd_log;

    char cwd[MY_PATH_LEN];
    strncat(cwd, config_t.log_path, strlen(config_t.log_path));
    fd_log = open(cwd, O_WRONLY | O_TRUNC | O_CREAT, 0644);
    ERROR("open: ", fd_log);

    dprintf(fd_log, "Server strarted with PID %d\n", pid);

    conn_queue = connection_queue_init();

    conn_queue->fd_log = fd_log;
    conn_queue->config_t = &config_t;

    thread_pool = thread_pool_init(config_t.num_threads, conn_queue, handle);

    struct sockaddr_in adrr = {
        .sin_family = PF_INET,
        .sin_addr.s_addr = htonl(INADDR_ANY),
        .sin_port = htons(config_t.port),
    };

    int server_socket_fd = socket(PF_INET, SOCK_STREAM, 0);
    ERROR("socket: ", server_socket_fd);
    dprintf(fd_log, "Socket created\n");

    ERROR("bind: ", bind(server_socket_fd, (struct sockaddr *)&adrr, sizeof(struct sockaddr_in)));
    dprintf(fd_log, "Bind at port: %d\n", config_t.port);

    ERROR("listen: ", listen(server_socket_fd, BACKLOG));
    dprintf(fd_log, "Listening for requests...\n");

    signal(SIGPIPE, shutdown_handler);
    signal(SIGTERM, shutdown_handler);

    while (true)
    {
        int client_socket_fd = accept(server_socket_fd, NULL, NULL);
        ERROR("accept: ", client_socket_fd);
        dprintf(fd_log, "Connected client\n");

        connection_t *cur_conn = handle_request(client_socket_fd, &adrr);
        ERROR_OBJ("handle_request", cur_conn);

        ERROR("connection_queue_push: ", connection_queue_push(conn_queue, cur_conn));
    }
}

connection_t *handle_request(int client_socket_fd, struct sockaddr_in *client_address)
{
    size_t len = CLIENT_REQ_BUFF_SIZE;
    char buff[len + 1];
    len = read_from_client(client_socket_fd, buff, len);
    if (len < 0)
    {
        return NULL;
    }
    buff[len] = 0;
    http_request_t *request = parse_request(buff);
    if (request == NULL)
    {
        return NULL;
    }
    connection_t *con = connection_new();
    if (con == NULL)
    {
        return NULL;
    }
    con->request = request;
    con->client_fd = client_socket_fd;
    con->client_address = client_address;
    return con;
}