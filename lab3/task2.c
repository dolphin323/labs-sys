#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/errno.h>
int BUFFER_SIZE = 512;
struct datum
{
  pid_t pid;
  struct tm current_timestamp;
  char buffer[256];
};

int main(void)
{
  int fd;
  char buffer[256];
  struct datum *dtm = NULL;
  time_t current_timestamp;
  fd = shm_open("/lab3", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
  if (fd == -1)
  {
    printf("Error: %s\n", strerror(errno));
    return -1;
  }
  if (ftruncate(fd, sizeof(struct datum)) != 0)
  {
    printf("Error: %s\n", strerror(errno));
    return -1;
  }
  dtm = mmap(NULL, sizeof(struct datum), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (dtm == MAP_FAILED)
  {
    printf("Error: %s\n", strerror(errno));
    return -1;
  }
  while (1)
  {
    printf("please, enter something:\n");
    fgets(buffer, 512, stdin);
    msync(dtm, sizeof(struct datum), MS_SYNC);
    printf("Data in datum:\n");
    printf("PID: %d\n", dtm->pid);
    printf("Current time: %s\n", asctime(&(dtm->current_timestamp)));
    printf("String from buffer: %s\n", dtm->buffer);
    dtm->pid = getpid();
    time(&current_timestamp);
    dtm->current_timestamp = (*localtime(&current_timestamp));

    strcpy(dtm->buffer, buffer);
  }
  munmap(dtm, sizeof(struct datum));
  shm_unlink("/lab3");
  return 0;
}