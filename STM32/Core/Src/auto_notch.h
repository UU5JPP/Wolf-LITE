#ifndef AUTO_NOTCH_h
#define AUTO_NOTCH_h

#include "stm32f4xx_hal.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "audio_processor.h"

#define AUTO_NOTCH_BLOCK_SIZE (AUDIO_BUFFER_HALF_SIZE / 3)	  // block size for processing
#define AUTO_NOTCH_TAPS 32				  // filter order
#define AUTO_NOTCH_REFERENCE_SIZE (AUTO_NOTCH_BLOCK_SIZE * 2) // size of the reference buffer
#define AUTO_NOTCH_STEP 0.0005f								  // LMS algorithm step
#define AUTO_NOTCH_DEBUG false

// Public methods
extern void InitAutoNotchReduction(void);										   // initialize the automatic notch filter
extern void processAutoNotchReduction(float32_t *buffer); // start automatic notch filter

#endif
