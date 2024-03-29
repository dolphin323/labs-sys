#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
  size_t BUFF_SIZE = 1024;
  if (argc < 2)
  {
    fprintf(stderr, "No label\n");
    exit(1);
  }
  if (argc > 2)
  {
    fprintf(stderr, "Need only one label\n");
    exit(1);
  }
  char *label = argv[1];
  char buff[BUFF_SIZE];
  int result, nread;
  fd_set inputs, testfds;
  struct timeval timeout;
  FD_ZERO(&inputs);
  FD_SET(0, &inputs);

  while (1)
  {
    testfds = inputs;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    result = select(FD_SETSIZE, &testfds, (fd_set *)NULL, (fd_set *)NULL, &timeout);
    switch (result)
    {
    case 0:
      fprintf(stderr, "No data within five seconds.\n");
      break;
    case -1:
      fprintf(stderr, "Error at select\n");
      exit(EXIT_FAILURE);
    default:
      nread = read(STDIN_FILENO, buff, BUFF_SIZE);
      if (buff[nread - 1] == '\n')
      {
        buff[nread - 1] = '\0';
      }
      fprintf(stderr, "%s: '%s'\n", label, buff);
      break;
    }
  }
  return 0;
}
