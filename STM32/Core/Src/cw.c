#include "stm32f4xx_hal.h"
#include "main.h"
#include "trx_manager.h"
#include "functions.h"
#include "lcd.h"
#include "fpga.h"
#include "settings.h"
#include "wm8731.h"
#include "fpga.h"
#include "bands.h"
#include "agc.h"
#include "audio_filters.h"
#include "usbd_audio_if.h"
#include "front_unit.h"
#include "rf_unit.h"
#include "system_menu.h"
#include "swr_analyzer.h"

volatile bool CW_key_serial = false;
volatile bool CW_old_key_serial = false;
volatile bool CW_key_dot_hard = false;
volatile bool CW_key_dash_hard = false;
volatile uint_fast16_t CW_Key_Timeout_est = 0;
volatile uint_fast8_t KEYER_symbol_status = 0;	  // status (signal or period) of the automatic key symbol

static uint32_t KEYER_symbol_start_time = 0;	  // start time of the automatic key character

static float32_t current_cw_power = 0.0f;				// current amplitude (for rise/fall)
static bool iambic_first_button_pressed = false; //start symbol | false - dot, true - dash
static bool iambic_last_symbol = false;					//last Iambic symbol | false - dot, true - dash
static bool iambic_sequence_started = false;
	
void CW_key_change(void)
{
	if (TRX_Tune)
		return;
	if (CurrentVFO()->Mode != TRX_MODE_CW_L && CurrentVFO()->Mode != TRX_MODE_CW_U)
		return;
	
	bool TRX_new_key_dot_hard = !HAL_GPIO_ReadPin(KEY_IN_DOT_GPIO_Port, KEY_IN_DOT_Pin);
	//if(TRX.CW_Invert)
		//TRX_new_key_dot_hard = !HAL_GPIO_ReadPin(KEY_IN_DASH_GPIO_Port, KEY_IN_DASH_Pin);
	
	if (CW_key_dot_hard != TRX_new_key_dot_hard)
	{
		CW_key_dot_hard = TRX_new_key_dot_hard;
		if (CW_key_dot_hard == true && (KEYER_symbol_status == 0 || !TRX.CW_KEYER))
		{
			CW_Key_Timeout_est = TRX.CW_Key_timeout;
			LCD_UpdateQuery.StatusInfoGUIRedraw = true;
			FPGA_NeedSendParams = true;
			TRX_Restart_Mode();
		}
	}
	
	bool TRX_new_key_dash_hard = !HAL_GPIO_ReadPin(KEY_IN_DASH_GPIO_Port, KEY_IN_DASH_Pin);
	//if(TRX.CW_Invert)
		//TRX_new_key_dash_hard = !HAL_GPIO_ReadPin(KEY_IN_DOT_GPIO_Port, KEY_IN_DOT_Pin);
	
	if (CW_key_dash_hard != TRX_new_key_dash_hard)
	{
		CW_key_dash_hard = TRX_new_key_dash_hard;
		if (CW_key_dash_hard == true && (KEYER_symbol_status == 0 || !TRX.CW_KEYER))
		{
			CW_Key_Timeout_est = TRX.CW_Key_timeout;
			LCD_UpdateQuery.StatusInfoGUIRedraw = true;
			FPGA_NeedSendParams = true;
			TRX_Restart_Mode();
		}
	}
	
	if (CW_key_serial != CW_old_key_serial)
	{
		CW_old_key_serial = CW_key_serial;
		if (CW_key_serial == true)
			CW_Key_Timeout_est = TRX.CW_Key_timeout;
		LCD_UpdateQuery.StatusInfoGUIRedraw = true;
		FPGA_NeedSendParams = true;
		TRX_Restart_Mode();
	}
}

static float32_t CW_generateRiseSignal(float32_t power)
{
	if (current_cw_power < power)
		current_cw_power += power * 0.007f;
	if (current_cw_power > power)
		current_cw_power = power;
	return current_cw_power;
}
static float32_t CW_generateFallSignal(float32_t power)
{
	if (current_cw_power > 0.0f)
		current_cw_power -= power * 0.007f;
	if (current_cw_power < 0.0f)
		current_cw_power = 0.0f;
	return current_cw_power;
}

float32_t CW_GenerateSignal(float32_t power)
{
	//Do no signal before start TX delay
	//if ((HAL_GetTick() - TRX_TX_StartTime) < CALIBRATE.TX_StartDelay)
		//return 0.0f;
		
	//Keyer disabled
	if (!TRX.CW_KEYER)
	{
		if (!CW_key_serial && !TRX_ptt_hard && !CW_key_dot_hard && !CW_key_dash_hard)
			return CW_generateFallSignal(power);
		return CW_generateRiseSignal(power);
	}
	
	//USB CW (Serial)
	if(CW_key_serial)
		return CW_generateRiseSignal(power);

	//Keyer
	const float32_t CW_DotToDashRate = 3.0f;
	uint32_t dot_length_ms = 1200 / TRX.CW_KEYER_WPM;
	uint32_t dash_length_ms = (float32_t)dot_length_ms * CW_DotToDashRate;
	uint32_t sim_space_length_ms = dot_length_ms;
	uint32_t curTime = HAL_GetTick();
	
	//Iambic keyer start mode
	if(CW_key_dot_hard && !CW_key_dash_hard)
		iambic_first_button_pressed = false;
	if(!CW_key_dot_hard && CW_key_dash_hard)
		iambic_first_button_pressed = true;
	if(CW_key_dot_hard && CW_key_dash_hard)
		iambic_sequence_started = true;
	
	//DOT .
	if (KEYER_symbol_status == 0 && CW_key_dot_hard)
	{
		KEYER_symbol_start_time = curTime;
		KEYER_symbol_status = 1;
	}
	if (KEYER_symbol_status == 1 && (KEYER_symbol_start_time + dot_length_ms) > curTime)
	{
		CW_Key_Timeout_est = TRX.CW_Key_timeout;
		return CW_generateRiseSignal(power);
	}
	if (KEYER_symbol_status == 1 && (KEYER_symbol_start_time + dot_length_ms) < curTime)
	{
		iambic_last_symbol = false;
		KEYER_symbol_start_time = curTime;
		KEYER_symbol_status = 3;
	}

	//DASH -
	if (KEYER_symbol_status == 0 && CW_key_dash_hard)
	{
		KEYER_symbol_start_time = curTime;
		KEYER_symbol_status = 2;
	}
	if (KEYER_symbol_status == 2 && (KEYER_symbol_start_time + dash_length_ms) > curTime)
	{
		CW_Key_Timeout_est = TRX.CW_Key_timeout;
		return CW_generateRiseSignal(power);
	}
	if (KEYER_symbol_status == 2 && (KEYER_symbol_start_time + dash_length_ms) < curTime)
	{
		iambic_last_symbol = true;
		KEYER_symbol_start_time = curTime;
		KEYER_symbol_status = 3;
	}

	//SPACE
	if (KEYER_symbol_status == 3 && (KEYER_symbol_start_time + sim_space_length_ms) > curTime)
	{
		CW_Key_Timeout_est = TRX.CW_Key_timeout;
		return CW_generateFallSignal(power);
	}
	if (KEYER_symbol_status == 3 && (KEYER_symbol_start_time + sim_space_length_ms) < curTime)
	{
		//if(!TRX.CW_Iambic) //classic keyer
			KEYER_symbol_status = 0;
		/*else //iambic keyer
		{
			//start iambic sequence
			if(iambic_sequence_started)
			{
				if(!iambic_last_symbol) // iambic dot . , next dash -
				{
					KEYER_symbol_start_time = curTime;
					KEYER_symbol_status = 2;
					if(iambic_first_button_pressed && (!CW_key_dot_hard || !CW_key_dash_hard)) //iambic dash-dot sequence compleated
					{
						//println("-.e");
						iambic_sequence_started = false;
						KEYER_symbol_status = 0;
					}
				}
				else // iambic dash - , next dot .
				{
					KEYER_symbol_start_time = curTime;
					KEYER_symbol_status = 1;
					if(!iambic_first_button_pressed && (!CW_key_dot_hard || !CW_key_dash_hard)) //iambic dot-dash sequence compleated
					{
						//println(".-e");
						KEYER_symbol_status = 0;
						iambic_sequence_started = false;
					}
				}
			}
			else //no sequence, classic mode
				KEYER_symbol_status = 0;
		}*/
	}

	return CW_generateFallSignal(power);
}
