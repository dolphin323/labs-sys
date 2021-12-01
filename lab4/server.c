#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/errno.h>

int main(void) {
    int fd;
    pid_t fork_id;

    fd = open("logs.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
      printf("Error: %s\n", strerror(errno));
      return -1;
    }
    write(fd, "Start\n", strlen("Start\n"));

    fork_id = fork();
    if (fork_id < 0) {
        write(fd, "Fork error\n", strlen("Fork error\n"));
        close(fd);
    }
    else if (fork_id == 0) {
        struct sockaddr_in sa_in;
        int sockfd;
        int client_sockfd;
        struct sockaddr_in client_sa_in;
        socklen_t client_sa_in_size;

        if (setsid() == -1) {
            printf("Error: %s\n", strerror(errno));
        }

        sa_in.sin_family = PF_INET;
        sa_in.sin_addr.s_addr = htonl(INADDR_ANY);
        sa_in.sin_port = htons(3227);

        sockfd = socket(PF_INET, SOCK_STREAM, 0);
        if (sockfd == -1) {
            write(fd, "Socket error\n", strlen("Socket error\n"));
            close(fd);
            exit(1);
        }
        if (bind(sockfd, (struct sockaddr *)&sa_in, sizeof(sa_in)) == -1) {
            write(fd, "Bind error\n", strlen("Bind error\n"));
            close(fd);
            exit(1);
        }

        char string[1024];
        sprintf(string, "(PID: %d) Listening\n", getpid());
        write(fd, string, strlen(string));

        listen(sockfd, 5);
        client_sa_in_size = sizeof(client_sa_in);

        while(1) {
            client_sockfd = accept(sockfd, (struct sockaddr *)&client_sa_in, &client_sa_in_size);
            if (client_sockfd < 0) {
                write(fd, "Connection error\n", strlen("Connection error\n"));
            }
            else {
                fork_id = fork();
                if (fork_id < 0) {
                    write(fd, "Fork error\n", strlen("Fork error\n"));
                    close(fd);
                }
                else if (fork_id == 0) {
                    close(sockfd);

                    char recv_buff[1024];
                    int recv_bytes;
                    char send_buff[1024];
                    int send_bytes;
                    time_t rawtime;
                    struct tm cur_time;

                    if (setsid() == -1) {
                        printf("Error: %s\n", strerror(errno));
                    }

                    while (1) {
                        recv_bytes = recv(client_sockfd, recv_buff, sizeof(recv_buff), 0);

                        time(&rawtime);
                        cur_time = (* localtime(&rawtime));
                        send_bytes = sprintf(send_buff, "(%d) %s%s\n\n", getpid(), asctime(&cur_time), recv_buff);
                        send_buff[send_bytes - 1] = '\0';
                        write(fd, send_buff, strlen(send_buff));
                        if (send(client_sockfd, send_buff, send_bytes, 0) == -1) {
                            write(fd, "Send error\n", strlen("Send error\n"));
                        }

                        if (strcmp(recv_buff, "close") == 0) break;
                        recv_buff[0] = '\0';
                        send_buff[0] = '\0';
                    }

                    write(fd, "Close\n", strlen("Close\n"));
                    close(client_sockfd);
                    close(fd);
                    exit(0);
                }
                else {
                    close(client_sockfd);
                } 
            }
        }
        close(fd);
    }
    else {
        write(fd, "Close parent\n", strlen("Close parent\n"));
        close(fd);
        exit(0);
    }

    close(fd);
    printf("Finished\n");
    return 0;
}