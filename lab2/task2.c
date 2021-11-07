#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <string.h>
#include <time.h>

void parentProcess(int * fd);
void childProcess();

int main(void) {
    int fd;
    ssize_t bytes_output;
    pid_t fork_id;

    fd = open("test.log", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
      printf("Error while opening test.log for writing (initial)\n");
      return -1;
    }
    write(fd, "STARTED\n", strlen("STARTED\n"));

    fork_id = fork();
    if (fork_id < 0) {
        write(fd, "Error occured while trying to fork()\n",
        strlen("Error occured while trying to fork()\n"));
        close(fd);
    }
    else if (fork_id == 0) {
        childProcess();
    }
    else {
        parentProcess(&fd);
    }

    return 0;
}

void parentProcess(int * fd) {
    write((* fd), "The child has been created, exiting\n",
    strlen("The child has been created, exiting\n"));
    exit(EXIT_SUCCESS);
}

void childProcess() {
    pid_t newSid;
    char buff[100];
    int buff_length;
    int pid;
    int i;
    int fd;

    newSid = setsid();
    if (newSid == -1) {
        printf("Error while calling setsid()\n");
    }
  
    if(chdir("/") == -1) {
        printf("Error while calling setsid()\n");
    }

    for(i = 0; i < 255; i++) {
        close(i);
    }

    open("/dev/null",O_RDWR);
    dup(0); 
    dup(0); 

    buff_length = sprintf(buff, "PID: %d, GID: %d, SID: %d\n", getpid(), getgid(), newSid);
    fd = open("/Users/liza/webUnivers/labs-sys/lab2/test.log", O_WRONLY | O_CREAT | O_APPEND, 0644);

    if (fd == -1) {
      printf("Error while opening test.log for writing (from daemon)\n");
      return;
    }
    
    while(1) {
        time_t cur = time(NULL);
        char * stime = ctime(&cur);
        buff_length = sprintf(buff, "PID: %d, Current time is: %s", getpid(), stime);
        write(fd, buff, buff_length);
        sleep(1);
    }
    close(fd);
}