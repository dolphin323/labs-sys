#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <unistd.h> 
#include <time.h> 
#include <sys/mman.h> 
#include <sys/stat.h> 
#include <fcntl.h> 

struct datum_s {
	pid_t pid;
	struct tm cur_time;
	char buff[256];
};

typedef struct datum_s datum_t;

int main(void) {
  int fd;
  char buff[256];
  time_t rawtime;

  fd = shm_open("/lab3", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
  
  if (fd == -1) {
      printf("Error while trying to shm_open!");
      return -1;
  }
  
  if (ftruncate(fd, sizeof(struct datum_s)) == -1) {
      perror("ftruncate");
      printf("Error while trying to ftruncate fd from shp_open!");
      return -1;
  }
  
  datum_t * datum = mmap(NULL, sizeof(struct datum_s), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (datum == MAP_FAILED) {
      printf("Error while trying to mmap() fd!");
      return -1;
  }

  while(1) {
    printf("Please, enter some string:\n");
    fgets(buff, 256, stdin);

    msync(datum, sizeof(struct datum_s), MS_SYNC);
    printf("Data in datum:\n");
    printf("PID: %d\n", datum->pid);
    printf("Time %s", asctime(&(datum->cur_time)));
    printf("String %s\n", datum->buff);
  
    datum->pid = getpid();
    time(&rawtime);
    datum->cur_time = (* localtime(&rawtime));
    strcpy(datum->buff, buff);
  }
  
  munmap(datum, sizeof(struct datum_s));
  shm_unlink("/lab3");
  return 0;
}