#include "stm32f4xx_hal.h"
#include "main.h"
#include "rf_unit.h"
#include "lcd.h"
#include "trx_manager.h"
#include "settings.h"
#include "system_menu.h"
#include "functions.h"
#include "audio_filters.h"
#include "front_unit.h"

#define SENS_TABLE_COUNT 24
static float32_t  pttsw1_old = 0;
static float32_t  pttsw2_old = 0;

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
	//Get Data from ADC
	float32_t cpu_temperature = (float32_t)(HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_1)) * 3.3f / 4096.0f;
	float32_t cpu_vref = (float32_t)(HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_2));
	float32_t cpu_vbat = (float32_t)(HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_3)) * 3.3f / 4096.0f;
	
	float32_t power_in = (float32_t)(HAL_ADCEx_InjectedGetValue(&hadc2, ADC_INJECTED_RANK_1)) * 3.3f / 4096.0f;
	float32_t alc = (float32_t)(HAL_ADCEx_InjectedGetValue(&hadc2, ADC_INJECTED_RANK_2)) * 3.3f / 4096.0f;
	float32_t ptt_sw1 = (float32_t)(HAL_ADCEx_InjectedGetValue(&hadc2, ADC_INJECTED_RANK_3)) * 3.3f / 4096.0f;
	float32_t ptt_sw2 = (float32_t)(HAL_ADCEx_InjectedGetValue(&hadc2, ADC_INJECTED_RANK_4)) * 3.3f / 4096.0f;
	
	float32_t forward = (float32_t)(HAL_ADCEx_InjectedGetValue(&hadc3, ADC_INJECTED_RANK_1)) * 3.3f / 4096.0f;
	float32_t backward = (float32_t)(HAL_ADCEx_InjectedGetValue(&hadc3, ADC_INJECTED_RANK_2)) * 3.3f / 4096.0f;
		
	//CPU Sensors
	float32_t cpu_temperature_result = (cpu_temperature - 0.760f) / 0.0025f + 25.0f;
	TRX_CPU_temperature = TRX_CPU_temperature * 0.9f + cpu_temperature_result * 0.1f;
	
	uint16_t VREFINT_CAL = *VREFINT_CAL_ADDR;
	float32_t cpu_vref_result = 3.3f * (float32_t)VREFINT_CAL / (float32_t)cpu_vref;
	TRX_CPU_VRef = TRX_CPU_VRef * 0.9f + cpu_vref_result * 0.1f;
	
	TRX_CPU_VBat = TRX_CPU_VBat * 0.9f + cpu_vbat * 2.0f * 0.1f;

	//SW1_Voltage
		TRX_SW1_Voltage = ptt_sw1;
	//SW2_Voltage
		TRX_SW2_Voltage = ptt_sw2;

	//POWER
	power_in = power_in * CALIBRATE.volt_cal_rate; //do voltage calibration in future!!!
	if(fabsf(TRX_InVoltage - power_in) > 0.2f)
		TRX_InVoltage = power_in;
	
	//SWR
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

		TRX_VLT_forward = 0.99f * TRX_VLT_forward + 0.01f * forward;
		TRX_VLT_backward = 0.99f * TRX_VLT_backward + 0.01f * backward;
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
	
	//TANGENT
	//sendToDebug_float32(ptt_sw1, false);
	//sendToDebug_float32(ptt_sw2, false);
	//sendToDebug_newline();

	if(pttsw1_old > 3.2 && pttsw2_old > 3.2 && ptt_sw2 > 0.1 && ptt_sw2 < 0.3 && ptt_sw1 > 1.80 && ptt_sw1 <2.0)
	{
		FRONTPANEL_BUTTONHANDLER_BAND_N();//DN
		HAL_Delay(200);
	}
	if(pttsw1_old > 3.2 && pttsw2_old > 3.2 && ptt_sw2 > 0.1 && ptt_sw2 < 0.3 && ptt_sw1 > 1.1 && ptt_sw1 <1.3)
	{
		FRONTPANEL_BUTTONHANDLER_BAND_P();//UP
		HAL_Delay(200);
	}
	
	if(pttsw1_old > 3.2 && pttsw2_old > 3.2 && ptt_sw2 > 1.0 && ptt_sw2 < 1.3 && ptt_sw1 > 2.80 && ptt_sw1 <3.0)
	{
		FRONTPANEL_BUTTONHANDLER_BW_N();//P1
		HAL_Delay(200);
	}
	if(pttsw1_old > 3.2 && pttsw2_old > 3.2 && ptt_sw2 > 1.8 && ptt_sw2 < 2.0 && ptt_sw1 > 2.8 && ptt_sw1 <3.0)
	{
		FRONTPANEL_BUTTONHANDLER_BW_P();//P2
		HAL_Delay(200);
	}
		if(pttsw1_old > 3.2 && pttsw2_old > 3.2 && ptt_sw2 > 2.4 && ptt_sw2 < 2.6 && ptt_sw1 > 2.8 && ptt_sw1 <3.0)
	{
		FRONTPANEL_BUTTONHANDLER_MODE_N();//P3
		HAL_Delay(200);
	}
	if(pttsw1_old > 3.2 && pttsw2_old > 3.2 && ptt_sw2 > 2.8 && ptt_sw2 < 3.0 && ptt_sw1 > 2.8 && ptt_sw1 <3.0)
	{
		FRONTPANEL_BUTTONHANDLER_MODE_P();//P4
		HAL_Delay(200);
	}
	if(pttsw1_old > 3.2 && pttsw2_old > 3.2 && ptt_sw2 > 1.1 && ptt_sw2 < 1.3 && ptt_sw1 > 0 && ptt_sw1 <0.1)
	{
		FRONTPANEL_BUTTONHANDLER_PRE();//1
		HAL_Delay(200);
	}
	if(pttsw1_old > 3.2 && pttsw2_old > 3.2 && ptt_sw2 > 1.8 && ptt_sw2 < 2.0 && ptt_sw1 > 0 && ptt_sw1 <0.1)
	{
		FRONTPANEL_BUTTONHANDLER_ATT();//2
		HAL_Delay(200);
	}
	if(pttsw1_old > 3.2 && pttsw2_old > 3.2 && ptt_sw2 > 1.82 && ptt_sw2 < 2.02 && ptt_sw1 > 0.09 && ptt_sw1 <0.29)
	{
		FRONTPANEL_ENCODER2_checkRotate();
		HAL_Delay(200);
	}
	if(pttsw1_old > 3.2 && pttsw2_old > 3.2 && ptt_sw2 > 1.06 && ptt_sw2 < 1.26 && ptt_sw1 > 0.09 && ptt_sw1 <0.29)
	{
		FRONTPANEL_ENCODER2_checkRotate();
		HAL_Delay(200);
	}	
	if(pttsw1_old > 3.2 && pttsw2_old > 3.2 && ptt_sw2 > 2.4 && ptt_sw2 < 2.6 && ptt_sw1 > 0.0 && ptt_sw1 <0.1)
	{
		FRONTPANEL_BUTTONHANDLER_AGC();//3
		HAL_Delay(200);
	}	
	if(pttsw1_old > 3.2 && pttsw2_old > 3.2 && ptt_sw2 > 1.1 && ptt_sw2 < 1.3 && ptt_sw1 > 1.0 && ptt_sw1 <1.1)
	{
		FRONTPANEL_BUTTONHANDLER_FAST();//4
		HAL_Delay(200);
	}		
	if(pttsw1_old > 3.2 && pttsw2_old > 3.2 && ptt_sw2 > 1.8 && ptt_sw2 < 2.0 && ptt_sw1 > 1.0 && ptt_sw1 <1.1)
	{
		FRONTPANEL_BUTTONHANDLER_MUTE();//5
		HAL_Delay(200);
	}		
	if(pttsw1_old > 3.2 && pttsw2_old > 3.2 && ptt_sw2 > 2.4 && ptt_sw2 < 2.6 && ptt_sw1 > 1.0 && ptt_sw1 <1.1)
	{
		FRONTPANEL_BUTTONHANDLER_LOCK();//6
		HAL_Delay(200);
	}	
	if(pttsw1_old > 3.2 && pttsw2_old > 3.2 && ptt_sw2 > 1.1 && ptt_sw2 < 1.3 && ptt_sw1 > 1.8 && ptt_sw1 <2.0)
	{
		FRONTPANEL_BUTTONHANDLER_NOTCH();//7
		HAL_Delay(200);
	}	
	if(pttsw1_old > 3.2 && pttsw2_old > 3.2 && ptt_sw2 > 1.1 && ptt_sw2 < 1.3 && ptt_sw1 > 2.3 && ptt_sw1 <2.6)
	{
		FRONTPANEL_BUTTONHANDLER_TUNE();//*
		HAL_Delay(200);
	}
	if(pttsw1_old > 3.2 && pttsw2_old > 3.2 && ptt_sw2 > 2.3 && ptt_sw2 < 2.6 && ptt_sw1 > 2.3 && ptt_sw1 <2.6)
	{
		FRONTPANEL_BUTTONHANDLER_ZOOM_P();//#
		HAL_Delay(200);
	}	
	pttsw1_old = ptt_sw1;
	pttsw2_old = ptt_sw2;
}
