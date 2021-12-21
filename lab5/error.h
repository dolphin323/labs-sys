#ifndef ERROR_H_
#define ERROR_H_

#include <stdlib.h>
#include <errno.h>
#include <string.h>

#define ERROR(msg, arg)                                                                         \
    if (arg < 0) {                                                                              \
        perror(msg);                                                                            \
        exit(errno);                                                                            \
    } 

#endif