#ifndef CRC16
#define CRC16

#include <stdint.h>

uint16_t gen_crc16(const uint8_t *data, uint16_t size);

#endif