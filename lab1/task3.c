#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>

int main(int argc, char *argv[])
{
  size_t BUFF_SIZE = 1024;
  if (argc < 2)
  {
    printf("No label");
    exit(1);
  }
  char *label = argv[1];
  struct pollfd fds[1];
  int result, nread;
  char buff[BUFF_SIZE];

  while (1)
  {
    fds[0].fd = STDIN_FILENO;
    fds[0].events = POLLIN;
    result = poll(fds, 1, 5000);
    switch (result)
    {
    case 0:
      printf("No data within five seconds.\n");
      break;
    case -1:
      printf("Error at select");
      exit(EXIT_FAILURE);
    default:
      nread = read(STDIN_FILENO, buff, BUFF_SIZE);
      if (buff[nread - 1] == '\n')
      {
        buff[nread - 1] = '\0';
      }
      printf("%s: '%s'\n", label, buff);
      break;
    }
  }
  return 0;
}