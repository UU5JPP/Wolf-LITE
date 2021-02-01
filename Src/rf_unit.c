#include "stm32f4xx_hal.h"
#include "main.h"
#include "rf_unit.h"
#include "lcd.h"
#include "trx_manager.h"
#include "settings.h"
#include "system_menu.h"
#include "functions.h"
#include "audio_filters.h"

#define SENS_TABLE_COUNT 24
static const int16_t KTY81_120_sensTable[SENS_TABLE_COUNT][2] = { // table of sensor characteristics
	{-55, 490},
	{-50, 515},
	{-40, 567},
	{-30, 624},
	{-20, 684},
	{-10, 747},
	{0, 815},
	{10, 886},
	{20, 961},
	{25, 1000},
	{30, 1040},
	{40, 1122},
	{50, 1209},
	{60, 1299},
	{70, 1392},
	{80, 1490},
	{90, 1591},
	{100, 1696},
	{110, 1805},
	{120, 1915},
	{125, 1970},
	{130, 2023},
	{140, 2124},
	{150, 2211}};

void RF_UNIT_ProcessSensors(void)
{
	//SWR
	float32_t forward = (float32_t)(HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_2)) * 3.3f / 4096.0f;
	float32_t backward = (float32_t)(HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_1)) * 3.3f / 4096.0f;
	float32_t ptt_sw1 = (float32_t)(HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_3)) * 3.3f / 4096.0f;
	float32_t ptt_sw2 = (float32_t)(HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_4)) * 3.3f / 4096.0f;
	float32_t alc = (float32_t)(HAL_ADCEx_InjectedGetValue(&hadc2, ADC_INJECTED_RANK_2)) * 3.3f / 4096.0f;
	float32_t power_in = (float32_t)(HAL_ADCEx_InjectedGetValue(&hadc2, ADC_INJECTED_RANK_1)) * 3.3f / 4096.0f;
	
	power_in = power_in * 10.f; //do voltage calibration in future!!!
	if(fabsf(TRX_InVoltage - power_in) > 0.2f)
		TRX_InVoltage = power_in;
	
	static float32_t TRX_VLT_forward = 0.0f;
	static float32_t TRX_VLT_backward = 0.0f;
	forward = forward / (1510.0f / (0.1f + 1510.0f)); // adjust the voltage based on the voltage divider (0.1 ohm and 510 ohm)
	if (forward < 0.01f)							  // do not measure less than 10mV
	{
		TRX_VLT_forward = 0.0f;
		TRX_VLT_backward = 0.0f;
		TRX_SWR = 1.0f;
	}
	else
	{
		forward += 0.21f;							  // drop on diode
		forward = forward * CALIBRATE.swr_trans_rate; // Transformation ratio of the SWR meter

		backward = backward / (1510.0f / (0.1f + 1510.0f)); // adjust the voltage based on the voltage divider (0.1 ohm and 510 ohm)
		if (backward >= 0.01f)								// do not measure less than 10mV
		{
			backward += 0.21f;								// drop on diode
			backward = backward * CALIBRATE.swr_trans_rate; // Transformation ratio of the SWR meter
		}
		else
			backward = 0.001f;

		TRX_VLT_forward = TRX_VLT_forward + (forward - TRX_VLT_forward) / 2;
		TRX_VLT_backward = TRX_VLT_backward + (backward - TRX_VLT_backward) / 2;
		TRX_SWR = (TRX_VLT_forward + TRX_VLT_backward) / (TRX_VLT_forward - TRX_VLT_backward);

		if (TRX_VLT_backward > TRX_VLT_forward)
			TRX_SWR = 10.0f;
		if (TRX_SWR > 10.0f)
			TRX_SWR = 10.0f;

		TRX_PWR_Forward = (TRX_VLT_forward * TRX_VLT_forward) / 50.0f;
		if (TRX_PWR_Forward < 0.0f)
			TRX_PWR_Forward = 0.0f;
		TRX_PWR_Backward = (TRX_VLT_backward * TRX_VLT_backward) / 50.0f;
		if (TRX_PWR_Backward < 0.0f)
			TRX_PWR_Backward = 0.0f;
	}
}
