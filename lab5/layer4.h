#ifndef LAYER4
#define LAYER4

#include "error.h"
#include "const.h"

int layer4_transmit(const char* filename);
int layer4_receive(const char* filename);

#endif