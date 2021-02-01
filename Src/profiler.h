#ifndef PROFILER_h
#define PROFILER_h

#include "stm32f4xx_hal.h"
#include <stdbool.h>
#define PROFILES_COUNT 7 // number of profilers

typedef struct // profiler structure
{
	uint32_t startTime;
	uint32_t endTime;
	uint32_t diff;
	bool started;
	uint32_t samples;
} PROFILE_INFO;

// Public methods
extern void InitProfiler(void);						  // initialize the profiler
extern void StartProfiler(uint8_t pid);				  // start profiler
extern void EndProfiler(uint8_t pid, bool summarize); // terminate the profiler
extern void PrintProfilerResult(void);				  // output the profiler results
extern void StartProfilerUs(void);					  // run profiler in microseconds
extern void EndProfilerUs(bool summarize);			  // complete profiler in microseconds

#endif
