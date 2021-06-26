#ifndef AGC_H
#define AGC_H

#include "stm32f4xx_hal.h"
#include <stdbool.h>
#include "audio_processor.h"

#define AGC_RINGBUFFER_TAPS_SIZE 3

//Public methods
extern void DoRxAGC(float32_t *agcbuffer, uint_fast16_t blockSize, uint_fast8_t mode); // start RX AGC on a data block
extern void DoTxAGC(float32_t *agcbuffer_i, uint_fast16_t blockSize, float32_t target, uint_fast8_t mode);                         // start TX AGC on a data block
extern void ResetAGC(void);

#endif
