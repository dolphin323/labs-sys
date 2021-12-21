#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./layer2.h"
#include "./layer3.h"

int layer3_transmit(uint8_t buf[], size_t buf_size, bool last_package) {
    puts("layer3_transmit");
    L3_TB_BLOCK* l3_tb = (L3_TB_BLOCK*)calloc(L3_TB_SIZE, sizeof(L2_TB_BLOCK));
    int num_blocks = buf_size / L3_MAX_SEG_SIZE;

    for (int i = 0; i < num_blocks; ++i) {
        memcpy(&l3_tb[i].data, &buf[i * L3_MAX_SEG_SIZE], L3_MAX_SEG_SIZE);
    }

    size_t last_block_size = buf_size - num_blocks * L3_MAX_SEG_SIZE;
    if (last_block_size != 0) {
        memcpy(&l3_tb[num_blocks].data, &buf[num_blocks*L3_MAX_SEG_SIZE], last_block_size);
        num_blocks++;
    }

    int res = layer2_transmit(l3_tb, num_blocks, last_block_size, last_package);
    free(l3_tb);
    return res;
}


int layer3_receive(uint8_t buf[], bool* last_package) {
    puts("layer3_receive");
    L3_TB_BLOCK* buffer = calloc(L3_TB_SIZE, sizeof(L2_TB_BLOCK));
    if (buffer == NULL) {
        return -1;
    }

    size_t last_block_size;
    size_t len = layer2_receive(buffer, &last_block_size, last_package);

    for (size_t i = 0; i < len-1; i++) {
        memcpy(&buf[i * L3_MAX_SEG_SIZE], &buffer[i].data, L3_MAX_SEG_SIZE);
    }
    memcpy(&buf[(len - 1) * L3_MAX_SEG_SIZE], &buffer[len - 1].data, last_block_size);

    free(buffer);
    return L3_MAX_SEG_SIZE * (len - 1) + last_block_size;
}