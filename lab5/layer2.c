#include <stdint.h>
#include <string.h>
#include <sys/types.h>

#include "./layer1.h"
#include "./layer2.h"
#include "./gen_crc16.h"

int layer2_transmit(L3_TB_BLOCK l3_tb[], size_t buf_size, size_t last_block_size, bool last_package) {
    puts("layer2_transmit");
    
    L2_TB_BLOCK* l2_tb = (L2_TB_BLOCK*)l3_tb;
    for (ssize_t i = buf_size - 1; i >= 0; i--) {
        L2_TB_BLOCK* pkg = &l2_tb[i];
        memmove(&pkg->l3_block, &l3_tb[i], sizeof(L3_TB_BLOCK));

        pkg->stx = 0x02;
        pkg->count[0] = 0;
        pkg->count[1] = 0;
        pkg->count[2] = i == buf_size - 1 ? last_block_size : L3_MAX_SEG_SIZE;
        pkg->ack = 0;
        pkg->seq = i;
        if (i == buf_size - 1) {
            if (last_package) {
                pkg->lframe = 0x0F;
            } else {
                pkg->lframe = 0x01;
            }
        } else {
            pkg->lframe = 0;
        }
        pkg->chksum = 0;
        pkg->etx = 0x03;

        pkg->chksum = gen_crc16((uint8_t*)pkg, sizeof(L2_TB_BLOCK));
    } 

    for (int i = 0; i < buf_size; i++) {
        int errors = 0;
        while (layer1_transmit(&l2_tb[i]) < 0) {
            errors += 1;
            if (errors >= TTR) {
                return -1;
            }
        }
    }

    return 0;
}

int layer2_receive(L3_TB_BLOCK l3_tb[], size_t* last_block_size, bool* last_package) {
    puts("layer2_receive");
    L2_TB_BLOCK* l2_tb = (L2_TB_BLOCK*)l3_tb;
    for (size_t i = 0; i < L3_TB_SIZE; ++i) {
        l2_tb[i].seq = -1;
    }

    *last_block_size = sizeof(L3_TB_BLOCK);
    *last_package = false;

    size_t received;
    size_t waiting_for = L3_TB_SIZE;
    for (received = 0; received < waiting_for;) {
        L2_TB_BLOCK pkg = {0};
        if (layer1_receive(&pkg) < 0) {
            continue;
        }
        int8_t seq = pkg.seq;
        if (l2_tb[seq].seq == -1) {
            memcpy(&l2_tb[seq], &pkg, sizeof(L2_TB_BLOCK));
            received += 1;
        }
        if (pkg.lframe == 0x01 || pkg.lframe == 0x0F) {
            waiting_for = seq;
            *last_block_size = pkg.count[2];
            *last_package = pkg.lframe == 0x0F;
        }
    }

    for (size_t i = 0; i < received; i++) {
        memmove(&l3_tb[i], &l2_tb[i].l3_block, sizeof(L3_TB_BLOCK));
    }

    return received;
}