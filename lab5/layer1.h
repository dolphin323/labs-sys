#ifndef LAYER1
#define LAYER1

#include "layer2.h"
#include "error.h"
#include "const.h"

int layer1_transmit(L2_TB_BLOCK* l2_tbb);
int layer1_receive(L2_TB_BLOCK* pkg);

#endif