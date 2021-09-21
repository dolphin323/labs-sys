#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

ssize_t processFile(int buffCount, int asciiShift, int fin, int fout);

int main(int argc, char *argv[])
{
  if (argc < 3)
  {
    fprintf(stderr, "Missing files\n");
    exit(1);
  }
  if (argc != 3)
  {
    fprintf(stderr, "Need only two files\n");
    exit(1);
  }
  char *filenameForRead = argv[1];
  char *filenameForWrite = argv[2];

  int fin = open(filenameForRead, O_RDONLY);

  if (fin == -1)
  {
    fprintf(stderr, "Error opening file '%s'\n", filenameForRead);
    return EXIT_FAILURE;
  }

  int fout = open(filenameForWrite, O_CREAT | O_WRONLY, 0644);

  if (fout == -1)
  {
    fprintf(stderr, "Error opening file '%s'\n", filenameForWrite);
    close(fin);
    return EXIT_FAILURE;
  }

  int buffCount = 512;
  int asciiShift = -32;

  ssize_t changedBytes = processFile(buffCount, asciiShift, fin, fout);

  printf("Number of changed bytes: %li \n", changedBytes);
  close(fout);
  close(fin);
}

ssize_t processFile(int buffCount, int asciiShift, int fin, int fout)
{
  char buff[++buffCount];
  ssize_t bytesRead = 0;
  ssize_t changedBytes = 0;

  do
  {
    bytesRead = read(fin, buff, buffCount);
    for (ssize_t i = 0; i <= buffCount; i++)
    {
      if (buff[i] >= 'a' && buff[i] <= 'z')
      {
        buff[i] += asciiShift;
        changedBytes++;
      }
    }
    write(fout, buff, bytesRead);
  } while (bytesRead == buffCount);

  return changedBytes;
}