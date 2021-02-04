#ifndef BANDS_H
#define BANDS_H

#include "stm32f4xx_hal.h"
#include <stdio.h>
#include <stdbool.h>

#define BANDS_COUNT 12 // number of bands in the collection

typedef struct // description of the region in the band
{
	const uint32_t startFreq;
	const uint32_t endFreq;
	const uint_fast8_t mode;
} REGION_MAP;

typedef struct // description of the band
{
	const char *name;
	const bool selectable;
	const uint32_t startFreq;
	const uint32_t endFreq;
	const REGION_MAP *regions;
	const uint_fast8_t regionsCount;
} BAND_MAP;

// Public variables
extern const BAND_MAP BANDS[BANDS_COUNT];

// Public methods
extern uint_fast8_t getModeFromFreq(uint32_t freq);			// mod from frequency
extern int8_t getBandFromFreq(uint32_t freq, bool nearest); // band number from frequency
#endif
