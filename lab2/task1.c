#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

void print_ids(void);
void parentProcess(void);
void childProcess(void);


int main(void) {
  pid_t pid, gid, sid;
  pid = getpid(); 
  gid = getgid(); 
  sid = getsid(pid);

  if (sid == -1) {
    printf("Error while trying to get sid of process");
  }
  printf("BEFORE FORK:\nPID: %d\nGID: %d\nSID: %d\n", pid, gid, sid);
  printf("______________\n");
  pid_t fork_id = fork();
  printf("AFTER FORK:\n");
  switch (fork_id){
    case -1:
      printf("Error while trying to fork()"); 
      return 1; 
    case 0:
      childProcess();
      printf("\nCHILD PROCESS ENDED, exiting\n");
    default:
      parentProcess();
      printf("\nPARENT PROCESS ENDED, exiting\n");
  }
  return 0;
}

void print_ids(void) {
  int i;
  printf("______________\n");
  for (i = 0; i < 4; i++) {
    pid_t pid, gid, sid;
    pid = getpid();
    gid = getgid();
    sid = getsid(pid);
    printf("ITERATION: %d:\nPID: %d\nGID: %d\nSID: %d\n", i, pid, gid, sid);
  printf("______________\n");}
 
}

void parentProcess(void) {
  int waitResultInfo;
  print_ids();
  wait(&waitResultInfo);
}

void childProcess(void) {
  print_ids();
}
