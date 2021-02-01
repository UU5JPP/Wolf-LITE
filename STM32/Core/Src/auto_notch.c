#include "auto_notch.h"
#include "trx_manager.h"

// Private variables
static arm_lms_norm_instance_f32 lms2_Norm_instance;
static float32_t lms2_stateF32[AUTO_NOTCH_TAPS + AUTO_NOTCH_BLOCK_SIZE - 1];
static float32_t lms2_normCoeff_f32[AUTO_NOTCH_TAPS];
static float32_t lms2_reference[AUTO_NOTCH_REFERENCE_SIZE];
static float32_t lms2_errsig2[AUTO_NOTCH_BLOCK_SIZE];
static uint_fast16_t reference_index_old;
static uint_fast16_t reference_index_new;

// initialize the automatic notch filter
void InitAutoNotchReduction(void)
{
	memset(&lms2_stateF32, 0x00, sizeof(lms2_stateF32));
	memset(&lms2_normCoeff_f32, 0x00, sizeof(lms2_normCoeff_f32));
	memset(&lms2_reference, 0x00, sizeof(lms2_reference));
	memset(&lms2_errsig2, 0x00, sizeof(lms2_errsig2));
	reference_index_old = 0;
	reference_index_new = 0;
	arm_lms_norm_init_f32(&lms2_Norm_instance, AUTO_NOTCH_TAPS, lms2_normCoeff_f32, lms2_stateF32, AUTO_NOTCH_STEP, AUTO_NOTCH_BLOCK_SIZE);
}

// start automatic notch filter
void processAutoNotchReduction(float32_t *buffer)
{
	//overflow protect
	static uint32_t temporary_stop = 0;
	if(temporary_stop > 0)
	{
		temporary_stop--;
		return;
	}
	memcpy(&lms2_reference[reference_index_new], buffer, sizeof(float32_t) * AUTO_NOTCH_BLOCK_SIZE);												// save the data to the reference buffer
	arm_lms_norm_f32(&lms2_Norm_instance, buffer, &lms2_reference[reference_index_old], lms2_errsig2, buffer, AUTO_NOTCH_BLOCK_SIZE); // start LMS filter
	
	//overflow protect
	float32_t minValOut = 0;
	float32_t maxValOut = 0;
	uint32_t index = 0;
	arm_min_f32(buffer, AUTO_NOTCH_BLOCK_SIZE, &minValOut, &index);
	arm_max_no_idx_f32(buffer, AUTO_NOTCH_BLOCK_SIZE, &maxValOut);
	if(isnanf(minValOut) || isinff(minValOut) || isnanf(maxValOut) || isinff(maxValOut))
	{
		if(AUTO_NOTCH_DEBUG)
		{
			sendToDebug_str("auto notch err ");
			sendToDebug_float32(minValOut,true);
			sendToDebug_str(" ");
			sendToDebug_float32(maxValOut,false);
		}
		InitAutoNotchReduction();
		memset(buffer, 0x00, sizeof(float32_t) * AUTO_NOTCH_BLOCK_SIZE);
		temporary_stop = 500;
	}
	arm_max_no_idx_f32(lms2_Norm_instance.pCoeffs, AUTO_NOTCH_TAPS, &maxValOut);
	if(maxValOut > 1.0f)
	{
		if(AUTO_NOTCH_DEBUG)
		{
			sendToDebug_strln("auto notch reset");
			sendToDebug_float32(maxValOut,false);
		}
		InitAutoNotchReduction();
		temporary_stop = 500;
	}
	
	reference_index_old += AUTO_NOTCH_BLOCK_SIZE;																												  // move along the reference buffer
	if (reference_index_old >= AUTO_NOTCH_REFERENCE_SIZE)
		reference_index_old = 0;
	reference_index_new = reference_index_old + AUTO_NOTCH_BLOCK_SIZE;
	if (reference_index_new >= AUTO_NOTCH_REFERENCE_SIZE)
		reference_index_new = 0;
}
