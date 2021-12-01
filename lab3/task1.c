#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

void signal_handler(int signo, siginfo_t * si, void * ucontext);

int FP;

int main(void) {
  char buff[100];
  int counter = 0;
 
  static struct sigaction oa;
  static struct sigaction na;

  int bytes_written = sprintf(buff, "PID: %d\nStarted working...\n", (int)getpid());
  FP = open("logs.log", O_CREAT | O_RDWR | O_TRUNC, 0644);
  write(FP, buff, bytes_written);
  close(FP);
  
  sigaction(SIGHUP, NULL, &oa);

  na.sa_sigaction = signal_handler;
  na.sa_flags = SA_SIGINFO;
  sigaction(SIGHUP, &na, NULL);

  while(1) {
    printf("Infinite loop (%d)\n", counter);
    FP = open("logs.log", O_CREAT | O_RDWR | O_APPEND, 0644);
    write(FP, "Waiting...\n", sizeof("Waiting...\n") - 1);
    close(FP);
    sleep(3);
    counter++;
  }

  return 0;
}

void signal_handler(int signo, siginfo_t *si, void * ucontext) {
  char buffer[256];
  int size = sprintf(buffer, "\nSignal number: %i\nErrno association: %i\nSignal code: %i\nSending process: %u\nSender's ruid: %u\nExit value: %i\nFaulting instruction: %p\nSignal value: %i\nBand event for SIGPOLL: %li\n", 
    si->si_signo, 
    si->si_errno,
    si->si_code,
    si->si_pid,
    si->si_uid,
    si->si_status,
    si->si_addr,
    si->si_value.sival_int,
    si->si_band
  );
  FP = open("logs.log", O_CREAT | O_RDWR | O_APPEND, 0644);
  
  write(FP, buffer, size);
  close(FP);
}