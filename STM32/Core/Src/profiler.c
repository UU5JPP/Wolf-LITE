#include "profiler.h"
#include "functions.h"

//Public variables
static PROFILE_INFO profiles[PROFILES_COUNT] = {0}; // collection of profilers

// initialize the profiler
void InitProfiler()
{
	for (uint8_t i = 0; i < PROFILES_COUNT; i++)
	{
		profiles[i].startTime = 0;
		profiles[i].endTime = 0;
		profiles[i].diff = 0;
		profiles[i].samples = 0;
		profiles[i].started = false;
	}
}

// start profiler
void StartProfiler(uint8_t pid)
{
	if (pid >= PROFILES_COUNT)
		return;
	if (profiles[pid].started)
		return;
	profiles[pid].started = true;
	profiles[pid].startTime = HAL_GetTick();
}

// run profiler in microseconds
void StartProfilerUs()
{
	if (profiles[PROFILES_COUNT - 1].started)
		return;
	profiles[PROFILES_COUNT - 1].started = true;
	profiles[PROFILES_COUNT - 1].startTime = DWT->CYCCNT;
}

// terminate the profiler
void EndProfiler(uint8_t pid, bool summarize)
{
	if (pid >= PROFILES_COUNT)
		return;
	if (!profiles[pid].started)
		return;
	profiles[pid].endTime = HAL_GetTick();
	if (summarize)
		profiles[pid].diff += profiles[pid].endTime - profiles[pid].startTime;
	else
		profiles[pid].diff = profiles[pid].endTime - profiles[pid].startTime;
	profiles[pid].samples++;
	profiles[pid].started = false;
}

// complete profiler in microseconds
void EndProfilerUs(bool summarize)
{
	if (!profiles[PROFILES_COUNT - 1].started)
		return;
	profiles[PROFILES_COUNT - 1].endTime = DWT->CYCCNT;
	uint32_t diff = (profiles[PROFILES_COUNT - 1].endTime - profiles[PROFILES_COUNT - 1].startTime);
	if (summarize)
		profiles[PROFILES_COUNT - 1].diff += diff;
	else
		profiles[PROFILES_COUNT - 1].diff = diff;
	profiles[PROFILES_COUNT - 1].samples++;
	profiles[PROFILES_COUNT - 1].started = false;
}

// output the profiler results
void PrintProfilerResult()
{
	bool printed = false;
	for (uint8_t i = 0; i < PROFILES_COUNT; i++)
		if (profiles[i].samples > 0)
		{
			sendToDebug_str("Profile #");
			sendToDebug_uint8(i, true);
			sendToDebug_str(" Samples: ");
			sendToDebug_uint32(profiles[i].samples, true);
			if (i == PROFILES_COUNT - 1)
			{
				sendToDebug_str(" Time, us: ");
				sendToDebug_uint32(profiles[i].diff / (SystemCoreClock / 1000000), false);
			}
			else
			{
				sendToDebug_str(" Time, ms: ");
				sendToDebug_uint32(profiles[i].diff, false);
			}
			profiles[i].diff = 0;
			profiles[i].samples = 0;
			printed = true;
		}
	if (printed)
		sendToDebug_str("\r\n");
}
