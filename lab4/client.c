#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/errno.h>

int main(void) {
    struct sockaddr_in addr;
    int sockfd;
    char send_buff[1024];
    int send_bytes;
    char recv_buff[1024];
    int recv_bytes;


    addr.sin_family = PF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = htons(3227);

    sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("Error: %s\n", strerror(errno));
        return -1;
    }

    if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        printf("Error: %s\n", strerror(errno));
        return -1;
    }

    do {
        printf("Send: ");
        fgets(send_buff, 1024, stdin);
        send_bytes = strlen(send_buff);
        if (send_bytes == 0) {
            continue;
        }

        if (send_buff[send_bytes - 1] == '\n') {
            send_buff[send_bytes - 1] = '\0';
        }

        if (send(sockfd, send_buff, send_bytes, 0) == -1) {
            printf("Error: %s\n", strerror(errno));
        }

        recv_bytes = recv(sockfd, recv_buff, sizeof(recv_buff), 0);
        printf("Respone: %s", recv_buff);
    } while(strcmp(send_buff, "close") != 0);

    close(sockfd);
    printf("Finished\n");
    return 0;
}