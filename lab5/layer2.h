#ifndef LAYER2
#define LAYER2

#include "layer3.h"
#include "error.h"
#include "const.h"
#include <inttypes.h>

typedef struct L2_TB_BLOCK {
    uint8_t stx;
    uint8_t count[3];
    uint8_t ack;
    int8_t seq;
    uint8_t lframe;
    uint8_t resvd[10];
    L3_TB_BLOCK l3_block;
    uint16_t chksum;
    uint8_t etx;
} L2_TB_BLOCK;


int layer2_transmit(L3_TB_BLOCK* l3_tb, size_t buf_size, size_t last_block_size, bool last_package);
int layer2_receive(L3_TB_BLOCK segments[], size_t* last_data_size, bool* last_in_file);

#endif