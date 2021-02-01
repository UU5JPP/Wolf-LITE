#ifndef AGC_H
#define AGC_H

#include "stm32f4xx_hal.h"
#include <stdbool.h>
#include "audio_processor.h"

#define AGC_RINGBUFFER_TAPS_SIZE 3

//Public methods
extern void DoRxAGC(float32_t *agcbuffer, uint_fast16_t blockSize, uint_fast8_t mode); // start AGC on a data block
extern void ResetAGC(void);

#endif
