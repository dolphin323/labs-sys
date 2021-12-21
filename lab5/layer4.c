#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "./layer3.h"
#include "./layer4.h"

int layer4_transmit(const char* filename) {
    puts("layer4_transmit");
    FILE* fin = fopen(filename, "r");
    if (fin == NULL) {
        perror("fopen: ");
    }
    size_t buf_len = L3_TB_SIZE * L3_MAX_SEG_SIZE;
    uint8_t buf[buf_len];
    size_t size;
    size_t sent = 0;
    while ((size = fread(buf, 1, buf_len, fin)) != 0) {
        bool is_eof = true;
        char c = fgetc(fin);
        if (feof(fin)) {
            is_eof = true;
        } else {
            ungetc(c, fin);
            is_eof = false;
        }
        layer3_transmit(buf, size, is_eof);
        sent += 1;
    }
    return 0;
}

int layer4_receive(const char* filename) {
   
    puts("layer4_receive");
    FILE* out = fopen(filename, "w");

    size_t buf_len = L3_TB_SIZE * L3_MAX_SEG_SIZE;
    uint8_t buf[buf_len];
    bool last_package = false;
    while (!last_package) {
        size_t len = layer3_receive(buf, &last_package);
        size_t wrote_bytes = fwrite(buf, 1, len, out);
        if (wrote_bytes == 0) {
            return -1;
        }
    }

    return 0;
}