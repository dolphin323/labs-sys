#ifndef LAYER3
#define LAYER3

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "error.h"
#include "const.h"

typedef struct L3_TB_BLOCK {
    uint8_t header[40];
    uint8_t data[60];
} L3_TB_BLOCK;

int layer3_transmit(uint8_t buf[], size_t buf_size, bool last_package);
int layer3_receive(uint8_t buf[], bool* last_package);


#endif