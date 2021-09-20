#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
  size_t BUFF_SIZE = 1024;
  if (argc < 2)
  {
    printf("No label");
    exit(1);
  }
  char *label = argv[1];
  fd_set rfds;
  struct timeval tv;
  int retval, len;
  char buff[BUFF_SIZE];

  tv.tv_sec = 5;
  tv.tv_usec = 0;

  while (1)
  {
    FD_ZERO(&rfds);
    FD_SET(0, &rfds);
    retval = select(1, &rfds, NULL, NULL, &tv);
    if (retval == -1)
    {
      printf("Error at select");
      exit(EXIT_FAILURE);
    }
    else if (retval)
    {
      ssize_t readBytes = read(STDIN_FILENO, buff, BUFF_SIZE);
      len = readBytes - 1;
      if (buff[len] == '\n')
      {
        buff[len] = '\0';
      }
      printf("%s: '%s'\n", label, buff);
    }
    else
    {
      printf("No data within five seconds.\n");
    }
  }
  return 0;
}