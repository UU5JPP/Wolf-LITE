#include "stm32f4xx_hal.h"
#include "main.h"
#include "front_unit.h"
#include "lcd.h"
#include "trx_manager.h"
#include "settings.h"
#include "system_menu.h"
#include "functions.h"
#include "audio_filters.h"
#include "auto_notch.h"
#include "agc.h"

static void FRONTPANEL_ENCODER_Rotated(float32_t direction);
static void FRONTPANEL_ENCODER2_Rotated(int8_t direction);
static uint16_t FRONTPANEL_ReadMCP3008_Value(uint8_t channel, GPIO_TypeDef *CS_PORT, uint16_t CS_PIN);
static void FRONTPANEL_ENCODER2_Rotated(int8_t direction);

static void FRONTPANEL_BUTTONHANDLER_MODE_P(void);
static void FRONTPANEL_BUTTONHANDLER_MODE_N(void);
static void FRONTPANEL_BUTTONHANDLER_BAND_P(void);
static void FRONTPANEL_BUTTONHANDLER_BAND_N(void);
static void FRONTPANEL_BUTTONHANDLER_SQUELCH(void);
static void FRONTPANEL_BUTTONHANDLER_WPM(void);
static void FRONTPANEL_BUTTONHANDLER_KEYER(void);
static void FRONTPANEL_BUTTONHANDLER_SHIFT(void);
static void FRONTPANEL_BUTTONHANDLER_CLAR(void);
static void FRONTPANEL_BUTTONHANDLER_STEP(void);
static void FRONTPANEL_BUTTONHANDLER_BANDMAP(void);
static void FRONTPANEL_BUTTONHANDLER_HIDDEN_ENABLE(void);
static void FRONTPANEL_BUTTONHANDLER_PRE(void);
static void FRONTPANEL_BUTTONHANDLER_ATT(void);
static void FRONTPANEL_BUTTONHANDLER_ATTHOLD(void);
static void FRONTPANEL_BUTTONHANDLER_AGC(void);
static void FRONTPANEL_BUTTONHANDLER_AGC_SPEED(void);
static void FRONTPANEL_BUTTONHANDLER_NOTCH(void);
static void FRONTPANEL_BUTTONHANDLER_FAST(void);
static void FRONTPANEL_BUTTONHANDLER_MUTE(void);
static void FRONTPANEL_BUTTONHANDLER_AsB(void);
static void FRONTPANEL_BUTTONHANDLER_ArB(void);
static void FRONTPANEL_BUTTONHANDLER_TUNE(void);
static void FRONTPANEL_BUTTONHANDLER_RF_POWER(void);
static void FRONTPANEL_BUTTONHANDLER_BW(void);
static void FRONTPANEL_BUTTONHANDLER_HPF(void);
static void FRONTPANEL_BUTTONHANDLER_MENU(void);
static void FRONTPANEL_BUTTONHANDLER_LOCK(void);
static void FRONTPANEL_BUTTONHANDLER_VOLUME(void);

static bool FRONTPanel_MCP3008_1_Enabled = true;

static int32_t ENCODER_slowler = 0;
static uint32_t ENCODER_AValDeb = 0;
static uint32_t ENCODER2_AValDeb = 0;

static bool enc2_func_mode = false; //false - fast-step, true - func mode (WPM, etc...)

static PERIPH_FrontPanel_Button PERIPH_FrontPanel_Static_Buttons[] = {
	{.port = 1, .channel = 7, .name = "", .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = NULL, .holdHandler = NULL}, //not used
	{.port = 1, .channel = 6, .name = "", .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = NULL, .holdHandler = NULL}, //not used
	{.port = 1, .channel = 5, .name = "MODE", .state = false, .prev_state = false, .work_in_menu = true, .clickHandler = FRONTPANEL_BUTTONHANDLER_MODE_N, .holdHandler = FRONTPANEL_BUTTONHANDLER_MODE_P}, //SB6
	{.port = 1, .channel = 0, .name = "BAND", .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_BAND_P, .holdHandler = FRONTPANEL_BUTTONHANDLER_BAND_N}, //SB1
};

static PERIPH_FrontPanel_Button PERIPH_FrontPanel_BottomScroll_Buttons[BOTTOM_SCROLLBUTTONS_GROUPS_COUNT][4] = {
	{
		{.port = 1, .channel = 1, .name = "PRE", .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_PRE, .holdHandler = FRONTPANEL_BUTTONHANDLER_PRE}, //SB2
		{.port = 1, .channel = 2, .name = "ATT", .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_ATT, .holdHandler = FRONTPANEL_BUTTONHANDLER_ATTHOLD}, //SB3
		{.port = 1, .channel = 3, .name = "BW", .state = false, .prev_state = false, .work_in_menu = true, .clickHandler = FRONTPANEL_BUTTONHANDLER_BW, .holdHandler = FRONTPANEL_BUTTONHANDLER_HPF}, //SB4
		{.port = 1, .channel = 4, .name = "A/B", .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_AsB, .holdHandler = FRONTPANEL_BUTTONHANDLER_ArB}, //SB5
	},
	{
		{.port = 1, .channel = 1, .name = "POWER", .state = false, .prev_state = false, .work_in_menu = true, .clickHandler = FRONTPANEL_BUTTONHANDLER_RF_POWER, .holdHandler = FRONTPANEL_BUTTONHANDLER_RF_POWER}, //SB2
		{.port = 1, .channel = 2, .name = "TUNE", .state = false, .prev_state = false, .work_in_menu = true, .clickHandler = FRONTPANEL_BUTTONHANDLER_TUNE, .holdHandler = FRONTPANEL_BUTTONHANDLER_TUNE}, //SB3
		{.port = 1, .channel = 3, .name = "NOTCH", .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_NOTCH, .holdHandler = FRONTPANEL_BUTTONHANDLER_NOTCH}, //SB4
		{.port = 1, .channel = 4, .name = "FAST", .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_FAST, .holdHandler = FRONTPANEL_BUTTONHANDLER_FAST}, //SB5
	},
	{
		{.port = 1, .channel = 1, .name = "AGC", .state = false, .prev_state = false, .work_in_menu = true, .clickHandler = FRONTPANEL_BUTTONHANDLER_AGC, .holdHandler = FRONTPANEL_BUTTONHANDLER_AGC_SPEED}, //SB2
		{.port = 1, .channel = 2, .name = "CLAR", .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_CLAR, .holdHandler = FRONTPANEL_BUTTONHANDLER_CLAR}, //SB3
		{.port = 1, .channel = 3, .name = "MUTE", .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_MUTE, .holdHandler = FRONTPANEL_BUTTONHANDLER_MUTE}, //SB4
		{.port = 1, .channel = 4, .name = "LOCK", .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_LOCK, .holdHandler = FRONTPANEL_BUTTONHANDLER_LOCK}, //SB5
	},
	{
		{.port = 1, .channel = 1, .name = "VOLUME", .state = false, .prev_state = false, .work_in_menu = true, .clickHandler = FRONTPANEL_BUTTONHANDLER_VOLUME, .holdHandler = FRONTPANEL_BUTTONHANDLER_VOLUME}, //SB2
		{.port = 1, .channel = 2, .name = "BANDMAP", .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_BANDMAP, .holdHandler = FRONTPANEL_BUTTONHANDLER_BANDMAP}, //SB3
		{.port = 1, .channel = 3, .name = "WPM", .state = false, .prev_state = false, .work_in_menu = true, .clickHandler = FRONTPANEL_BUTTONHANDLER_WPM, .holdHandler = FRONTPANEL_BUTTONHANDLER_WPM}, //SB4
		{.port = 1, .channel = 4, .name = "KEYER", .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_KEYER, .holdHandler = FRONTPANEL_BUTTONHANDLER_KEYER}, //SB5
	},
};

PERIPH_FrontPanel_Button* PERIPH_FrontPanel_BottomScroll_Buttons_Active = PERIPH_FrontPanel_BottomScroll_Buttons[0];
int8_t PERIPH_FrontPanel_BottomScroll_index = 0;

void FRONTPANEL_ENCODER_checkRotate(void)
{
	static uint32_t ENCstartMeasureTime = 0;
	static int16_t ENCticksInInterval = 0;
	static float32_t ENCAcceleration = 0;
	static uint8_t ENClastClkVal = 0;
	static bool ENCfirst = true;
	uint8_t ENCODER_DTVal = HAL_GPIO_ReadPin(ENC_DT_GPIO_Port, ENC_DT_Pin);
	uint8_t ENCODER_CLKVal = HAL_GPIO_ReadPin(ENC_CLK_GPIO_Port, ENC_CLK_Pin);

	if (ENCfirst)
	{
		ENClastClkVal = ENCODER_CLKVal;
		ENCfirst = false;
	}
	if ((HAL_GetTick() - ENCODER_AValDeb) < CALIBRATE.ENCODER_DEBOUNCE)
		return;

	if (ENClastClkVal != ENCODER_CLKVal)
	{
		if (!CALIBRATE.ENCODER_ON_FALLING || ENCODER_CLKVal == 0)
		{
			if (ENCODER_DTVal != ENCODER_CLKVal)
			{ // If pin A changed first - clockwise rotation
				ENCODER_slowler--;
				if (ENCODER_slowler <= -CALIBRATE.ENCODER_SLOW_RATE)
				{
					//acceleration
					ENCticksInInterval++;
					if((HAL_GetTick() - ENCstartMeasureTime) > ENCODER_ACCELERATION)
					{
						ENCstartMeasureTime = HAL_GetTick();
						ENCAcceleration = (10.0f + ENCticksInInterval - 1.0f) / 10.0f;
						ENCticksInInterval = 0;
					}
					//do rotate
					FRONTPANEL_ENCODER_Rotated(CALIBRATE.ENCODER_INVERT ? ENCAcceleration : -ENCAcceleration);
					ENCODER_slowler = 0;
				}
			}
			else
			{ // otherwise B changed its state first - counterclockwise rotation
				ENCODER_slowler++;
				if (ENCODER_slowler >= CALIBRATE.ENCODER_SLOW_RATE)
				{
					//acceleration
					ENCticksInInterval++;
					if((HAL_GetTick() - ENCstartMeasureTime) > ENCODER_ACCELERATION)
					{
						ENCstartMeasureTime = HAL_GetTick();
						ENCAcceleration = (10.0f + ENCticksInInterval - 1.0f) / 10.0f;
						ENCticksInInterval = 0;
					}
					//do rotate
					FRONTPANEL_ENCODER_Rotated(CALIBRATE.ENCODER_INVERT ? -ENCAcceleration : ENCAcceleration);
					ENCODER_slowler = 0;
				}
			}
		}
		ENCODER_AValDeb = HAL_GetTick();
		ENClastClkVal = ENCODER_CLKVal;
	}
}

void FRONTPANEL_ENCODER2_checkRotate(void)
{
	uint8_t ENCODER2_DTVal = HAL_GPIO_ReadPin(ENC2_DT_GPIO_Port, ENC2_DT_Pin);
	uint8_t ENCODER2_CLKVal = HAL_GPIO_ReadPin(ENC2_CLK_GPIO_Port, ENC2_CLK_Pin);

	if ((HAL_GetTick() - ENCODER2_AValDeb) < CALIBRATE.ENCODER2_DEBOUNCE)
		return;

	if (!CALIBRATE.ENCODER_ON_FALLING || ENCODER2_CLKVal == 0)
	{
		if (ENCODER2_DTVal != ENCODER2_CLKVal)
		{ // If pin A changed first - clockwise rotation
			FRONTPANEL_ENCODER2_Rotated(CALIBRATE.ENCODER2_INVERT ? 1 : -1);
		}
		else
		{ // otherwise B changed its state first - counterclockwise rotation
			FRONTPANEL_ENCODER2_Rotated(CALIBRATE.ENCODER2_INVERT ? -1 : 1);
		}
	}
	ENCODER2_AValDeb = HAL_GetTick();
}

static void FRONTPANEL_ENCODER_Rotated(float32_t direction) // rotated encoder, handler here, direction -1 - left, 1 - right
{
	if (TRX.Locked)
		return;
	if (LCD_systemMenuOpened)
	{
		eventRotateSystemMenu((int8_t)direction);
		return;
	}
	if(fabsf(direction) <= ENCODER_MIN_RATE_ACCELERATION)
		direction = (direction < 0.0f)? -1.0f : 1.0f;
	
	VFO *vfo = CurrentVFO();
	uint32_t newfreq = 0;
	if (TRX.Fast)
	{
		newfreq = (uint32_t)((int32_t)vfo->Freq + (int32_t)((float32_t)TRX.FRQ_FAST_STEP * direction));
		if ((vfo->Freq % TRX.FRQ_FAST_STEP) > 0 && fabsf(direction) <= 1.0f)
			newfreq = vfo->Freq / TRX.FRQ_FAST_STEP * TRX.FRQ_FAST_STEP;
	}
	else
	{
		newfreq = (uint32_t)((int32_t)vfo->Freq + (int32_t)((float32_t)TRX.FRQ_STEP * direction));
		if ((vfo->Freq % TRX.FRQ_STEP) > 0 && fabsf(direction) <= 1.0f)
			newfreq = vfo->Freq / TRX.FRQ_STEP * TRX.FRQ_STEP;
	}
	TRX_setFrequency(newfreq, vfo);
	LCD_UpdateQuery.FreqInfo = true;
	NeedSaveSettings = true;
}

static void FRONTPANEL_ENCODER2_Rotated(int8_t direction) // rotated encoder, handler here, direction -1 - left, 1 - right
{
	if (TRX.Locked)
		return;

	if (LCD_systemMenuOpened)
	{
		eventSecRotateSystemMenu(direction);
		return;
	}
	else
	{
		PERIPH_FrontPanel_BottomScroll_index += direction;
		if(PERIPH_FrontPanel_BottomScroll_index < 0)
			PERIPH_FrontPanel_BottomScroll_index = BOTTOM_SCROLLBUTTONS_GROUPS_COUNT - 1;
		if(PERIPH_FrontPanel_BottomScroll_index >= BOTTOM_SCROLLBUTTONS_GROUPS_COUNT)
			PERIPH_FrontPanel_BottomScroll_index = 0;
		PERIPH_FrontPanel_BottomScroll_Buttons_Active = PERIPH_FrontPanel_BottomScroll_Buttons[PERIPH_FrontPanel_BottomScroll_index];
		LCD_UpdateQuery.TopButtons = true;
	}
}

void FRONTPANEL_check_ENC2SW(void)
{
	static bool ENC2SW_Last = true;
	
	if (TRX.Locked)
		return;

	bool ENC2SW_Now = HAL_GPIO_ReadPin(ENC2_SW_GPIO_Port, ENC2_SW_Pin);
	if (ENC2SW_Last != ENC2SW_Now)
	{
		ENC2SW_Last = ENC2SW_Now;
		if (!ENC2SW_Now)
		{
			FRONTPANEL_BUTTONHANDLER_MENU();
		}
	}
}

void FRONTPANEL_Init(void)
{
	uint16_t test_value = FRONTPANEL_ReadMCP3008_Value(0, AD1_CS_GPIO_Port, AD1_CS_Pin);
	if (test_value == 65535)
	{
		FRONTPanel_MCP3008_1_Enabled = false;
		sendToDebug_strln("[ERR] Frontpanel MCP3008 - 1 not found, disabling... (FPGA SPI/I2S CLOCK ERROR?)");
		LCD_showError("MCP3008 - 1 init error (FPGA I2S CLK?)", true);
	}
	FRONTPANEL_Process();
}

void FRONTPANEL_Process(void)
{
	if (SPI_process)
		return;
	SPI_process = true;

	FRONTPANEL_check_ENC2SW();
	uint16_t buttons_count = sizeof(PERIPH_FrontPanel_Static_Buttons) / sizeof(PERIPH_FrontPanel_Button);
	uint16_t mcp3008_value = 0;
	bool process_static_buttons = true;

	//process buttons
	PERIPH_FrontPanel_Button* PERIPH_FrontPanel_Buttons = PERIPH_FrontPanel_Static_Buttons;
	for (int16_t b = 0; b < buttons_count; b++)
	{
		//check disabled ports
		if (PERIPH_FrontPanel_Buttons[b].port == 1 && !FRONTPanel_MCP3008_1_Enabled)
			continue;

		//get state from ADC MCP3008 (10bit - 1024values)
		if (PERIPH_FrontPanel_Buttons[b].port == 1)
			mcp3008_value = FRONTPANEL_ReadMCP3008_Value(PERIPH_FrontPanel_Buttons[b].channel, AD1_CS_GPIO_Port, AD1_CS_Pin);

		/*sendToDebug_str("port: ");
		sendToDebug_uint8(PERIPH_FrontPanel_Buttons[b].port, true);
		sendToDebug_str("channel: ");
		sendToDebug_uint8(PERIPH_FrontPanel_Buttons[b].channel, true);
		sendToDebug_str("value: ");
		sendToDebug_uint16(mcp3008_value, false);*/
		
		//set state
		if (mcp3008_value < MCP3008_THRESHOLD)
			PERIPH_FrontPanel_Buttons[b].state = true;
		else
			PERIPH_FrontPanel_Buttons[b].state = false;

		//check state
		if ((PERIPH_FrontPanel_Buttons[b].prev_state != PERIPH_FrontPanel_Buttons[b].state) && PERIPH_FrontPanel_Buttons[b].state)
		{
			PERIPH_FrontPanel_Buttons[b].start_hold_time = HAL_GetTick();
			PERIPH_FrontPanel_Buttons[b].afterhold = false;
		}

		//check hold state
		if ((PERIPH_FrontPanel_Buttons[b].prev_state == PERIPH_FrontPanel_Buttons[b].state) && PERIPH_FrontPanel_Buttons[b].state && ((HAL_GetTick() - PERIPH_FrontPanel_Buttons[b].start_hold_time) > KEY_HOLD_TIME) && !PERIPH_FrontPanel_Buttons[b].afterhold)
		{
			PERIPH_FrontPanel_Buttons[b].afterhold = true;
			if (!TRX.Locked || (PERIPH_FrontPanel_Buttons[b].clickHandler == FRONTPANEL_BUTTONHANDLER_LOCK)) //LOCK BUTTON
				if (!LCD_systemMenuOpened || PERIPH_FrontPanel_Buttons[b].work_in_menu)
					if (PERIPH_FrontPanel_Buttons[b].holdHandler != NULL)
					{
						WM8731_Beep();
						PERIPH_FrontPanel_Buttons[b].holdHandler();
					}
		}

		//check click state
		if ((PERIPH_FrontPanel_Buttons[b].prev_state != PERIPH_FrontPanel_Buttons[b].state) && !PERIPH_FrontPanel_Buttons[b].state && ((HAL_GetTick() - PERIPH_FrontPanel_Buttons[b].start_hold_time) < KEY_HOLD_TIME) && !PERIPH_FrontPanel_Buttons[b].afterhold && !TRX.Locked)
		{
			if (!LCD_systemMenuOpened || PERIPH_FrontPanel_Buttons[b].work_in_menu)
				if (PERIPH_FrontPanel_Buttons[b].clickHandler != NULL)
				{
					WM8731_Beep();
					PERIPH_FrontPanel_Buttons[b].clickHandler();
				}
		}

		//save prev state
		PERIPH_FrontPanel_Buttons[b].prev_state = PERIPH_FrontPanel_Buttons[b].state;
		
		if(process_static_buttons && b == (buttons_count - 1))
		{
			//repeat with dynamic buttons
			process_static_buttons = false;
			buttons_count = sizeof(PERIPH_FrontPanel_BottomScroll_Buttons[0]) / sizeof(PERIPH_FrontPanel_Button);
			PERIPH_FrontPanel_Buttons = PERIPH_FrontPanel_BottomScroll_Buttons_Active;
			b = -1;
		}
	}
	SPI_process = false;
}

void FRONTPANEL_BUTTONHANDLER_AsB(void) // A/B
{
	TRX_TemporaryMute();
	TRX.current_vfo = !TRX.current_vfo;
	TRX_setFrequency(CurrentVFO()->Freq, CurrentVFO());
	TRX_setMode(CurrentVFO()->Mode, CurrentVFO());
	LCD_UpdateQuery.TopButtons = true;
	LCD_UpdateQuery.FreqInfo = true;
	LCD_UpdateQuery.StatusInfoGUI = true;
	NeedSaveSettings = true;
	NeedReinitAudioFilters = true;
	LCD_redraw(false);
}

void FRONTPANEL_BUTTONHANDLER_TUNE(void)
{
	TRX_Tune = !TRX_Tune;
	TRX_ptt_hard = TRX_Tune;
	LCD_UpdateQuery.StatusInfoGUI = true;
	LCD_UpdateQuery.TopButtons = true;
	NeedSaveSettings = true;
	TRX_Restart_Mode();
}

void FRONTPANEL_BUTTONHANDLER_PRE(void)
{
	TRX.ADC_Driver = !TRX.ADC_Driver;
	int8_t band = getBandFromFreq(CurrentVFO()->Freq, true);
	if (band > 0)
		TRX.BANDS_SAVED_SETTINGS[band].ADC_Driver = TRX.ADC_Driver;
	LCD_UpdateQuery.TopButtons = true;
	NeedSaveSettings = true;
}

void FRONTPANEL_BUTTONHANDLER_ATT(void)
{
	TRX.ATT = !TRX.ATT;

	int8_t band = getBandFromFreq(CurrentVFO()->Freq, true);
	if (band > 0)
	{
		TRX.BANDS_SAVED_SETTINGS[band].ATT = TRX.ATT;
		TRX.BANDS_SAVED_SETTINGS[band].ATT_DB = TRX.ATT_DB;
	}

	LCD_UpdateQuery.TopButtons = true;
	NeedSaveSettings = true;
}

void FRONTPANEL_BUTTONHANDLER_ATTHOLD(void)
{
	TRX.ATT_DB += TRX.ATT_STEP;
	if (TRX.ATT_DB > 31.0f)
		TRX.ATT_DB = TRX.ATT_STEP;

	int8_t band = getBandFromFreq(CurrentVFO()->Freq, true);
	if (band > 0)
	{
		TRX.BANDS_SAVED_SETTINGS[band].ATT = TRX.ATT;
		TRX.BANDS_SAVED_SETTINGS[band].ATT_DB = TRX.ATT_DB;
	}

	LCD_UpdateQuery.TopButtons = true;
	NeedSaveSettings = true;
}

void FRONTPANEL_BUTTONHANDLER_FAST(void)
{
	TRX.Fast = !TRX.Fast;
	LCD_UpdateQuery.TopButtons = true;
	NeedSaveSettings = true;
}

static void FRONTPANEL_BUTTONHANDLER_MODE_P(void)
{
	//enable claibration hidden menu!
	if (LCD_systemMenuOpened)
	{
		sysmenu_hiddenmenu_enabled = true;
		LCD_redraw(false);
		LCD_UpdateQuery.TopButtons = true;
		LCD_UpdateQuery.StatusInfoBar = true;
		NeedSaveSettings = true;
		return;
	}
	
	int8_t mode = (int8_t)CurrentVFO()->Mode;
	if (mode == TRX_MODE_LSB)
		mode = TRX_MODE_USB;
	else if (mode == TRX_MODE_USB)
		mode = TRX_MODE_LSB;
	else if (mode == TRX_MODE_CW_L)
		mode = TRX_MODE_CW_U;
	else if (mode == TRX_MODE_CW_U)
		mode = TRX_MODE_CW_L;
	else if (mode == TRX_MODE_NFM)
		mode = TRX_MODE_WFM;
	else if (mode == TRX_MODE_WFM)
		mode = TRX_MODE_NFM;
	else if (mode == TRX_MODE_DIGI_L)
		mode = TRX_MODE_DIGI_U;
	else if (mode == TRX_MODE_DIGI_U)
		mode = TRX_MODE_DIGI_L;
	else if (mode == TRX_MODE_AM)
		mode = TRX_MODE_IQ;
	else if (mode == TRX_MODE_IQ)
	{
		mode = TRX_MODE_LOOPBACK;
		LCD_UpdateQuery.StatusInfoGUIRedraw = true;
	}
	else if (mode == TRX_MODE_LOOPBACK)
	{
		mode = TRX_MODE_AM;
		LCD_UpdateQuery.StatusInfoGUIRedraw = true;
	}

	TRX_setMode((uint8_t)mode, CurrentVFO());
	int8_t band = getBandFromFreq(CurrentVFO()->Freq, true);
	if (band > 0)
		TRX.BANDS_SAVED_SETTINGS[band].Mode = (uint8_t)mode;
	TRX_Temporary_Stop_BandMap = true;
}

static void FRONTPANEL_BUTTONHANDLER_MODE_N(void)
{
	int8_t mode = (int8_t)CurrentVFO()->Mode;
	if(mode == TRX_MODE_LOOPBACK)
		LCD_UpdateQuery.StatusInfoGUIRedraw = true;
	if (mode == TRX_MODE_LSB)
		mode = TRX_MODE_CW_L;
	else if (mode == TRX_MODE_USB)
		mode = TRX_MODE_CW_U;
	else if (mode == TRX_MODE_CW_L || mode == TRX_MODE_CW_U)
		mode = TRX_MODE_DIGI_U;
	else if (mode == TRX_MODE_DIGI_L || mode == TRX_MODE_DIGI_U)
		mode = TRX_MODE_NFM;
	else if (mode == TRX_MODE_NFM || mode == TRX_MODE_WFM)
		mode = TRX_MODE_AM;
	else
	{
		if (CurrentVFO()->Freq < 10000000)
			mode = TRX_MODE_LSB;
		else
			mode = TRX_MODE_USB;
	}

	TRX_setMode((uint8_t)mode, CurrentVFO());
	int8_t band = getBandFromFreq(CurrentVFO()->Freq, true);
	if (band > 0)
		TRX.BANDS_SAVED_SETTINGS[band].Mode = (uint8_t)mode;
	TRX_Temporary_Stop_BandMap = true;
}

static void FRONTPANEL_BUTTONHANDLER_BAND_P(void)
{
	int8_t band = getBandFromFreq(CurrentVFO()->Freq, true);
	band++;
	if (band >= BANDS_COUNT)
		band = 0;
	while (!BANDS[band].selectable)
	{
		band++;
		if (band >= BANDS_COUNT)
			band = 0;
	}

	TRX_setFrequency(TRX.BANDS_SAVED_SETTINGS[band].Freq, CurrentVFO());
	TRX_setMode(TRX.BANDS_SAVED_SETTINGS[band].Mode, CurrentVFO());
	TRX.ATT = TRX.BANDS_SAVED_SETTINGS[band].ATT;
	TRX.ATT_DB = TRX.BANDS_SAVED_SETTINGS[band].ATT_DB;
	TRX.ADC_Driver = TRX.BANDS_SAVED_SETTINGS[band].ADC_Driver;
	TRX.FM_SQL_threshold = TRX.BANDS_SAVED_SETTINGS[band].FM_SQL_threshold;
	TRX_AutoGain_Stage = TRX.BANDS_SAVED_SETTINGS[band].AutoGain_Stage;
	CurrentVFO()->AGC = TRX.BANDS_SAVED_SETTINGS[band].AGC;
	TRX_Temporary_Stop_BandMap = false;

	LCD_UpdateQuery.TopButtons = true;
	LCD_UpdateQuery.FreqInfo = true;
}

static void FRONTPANEL_BUTTONHANDLER_BAND_N(void)
{
	int8_t band = getBandFromFreq(CurrentVFO()->Freq, true);
	band--;
	if (band < 0)
		band = BANDS_COUNT - 1;
	while (!BANDS[band].selectable)
	{
		band--;
		if (band < 0)
			band = BANDS_COUNT - 1;
	}

	TRX_setFrequency(TRX.BANDS_SAVED_SETTINGS[band].Freq, CurrentVFO());
	TRX_setMode(TRX.BANDS_SAVED_SETTINGS[band].Mode, CurrentVFO());
	TRX.ATT = TRX.BANDS_SAVED_SETTINGS[band].ATT;
	TRX.ATT_DB = TRX.BANDS_SAVED_SETTINGS[band].ATT_DB;
	TRX.ADC_Driver = TRX.BANDS_SAVED_SETTINGS[band].ADC_Driver;
	TRX.FM_SQL_threshold = TRX.BANDS_SAVED_SETTINGS[band].FM_SQL_threshold;
	TRX_AutoGain_Stage = TRX.BANDS_SAVED_SETTINGS[band].AutoGain_Stage;
	CurrentVFO()->AGC = TRX.BANDS_SAVED_SETTINGS[band].AGC;
	TRX_Temporary_Stop_BandMap = false;

	LCD_UpdateQuery.TopButtons = true;
	LCD_UpdateQuery.FreqInfo = true;
}

void FRONTPANEL_BUTTONHANDLER_RF_POWER(void)
{
	if (!LCD_systemMenuOpened)
	{
		LCD_systemMenuOpened = true;
		SYSMENU_TRX_RFPOWER_HOTKEY();
	}
	else
	{
		eventCloseAllSystemMenu();
	}
	LCD_redraw(false);
}

void FRONTPANEL_BUTTONHANDLER_AGC(void)
{
	CurrentVFO()->AGC = !CurrentVFO()->AGC;
	int8_t band = getBandFromFreq(CurrentVFO()->Freq, true);
	if (band > 0)
		TRX.BANDS_SAVED_SETTINGS[band].AGC = CurrentVFO()->AGC;
	LCD_UpdateQuery.TopButtons = true;
	NeedSaveSettings = true;
}

void FRONTPANEL_BUTTONHANDLER_AGC_SPEED(void)
{
	if (!LCD_systemMenuOpened)
	{
		LCD_systemMenuOpened = true;
		SYSMENU_AUDIO_AGC_HOTKEY();
	}
	else
	{
		eventCloseAllSystemMenu();
	}
	LCD_redraw(false);
}

static void FRONTPANEL_BUTTONHANDLER_SQUELCH(void)
{
	if (!LCD_systemMenuOpened)
	{
		LCD_systemMenuOpened = true;
		SYSMENU_AUDIO_SQUELCH_HOTKEY();
	}
	else
	{
		eventCloseAllSystemMenu();
	}
	LCD_redraw(false);
}

static void FRONTPANEL_BUTTONHANDLER_WPM(void)
{
	if (!LCD_systemMenuOpened)
	{
		LCD_systemMenuOpened = true;
		SYSMENU_CW_WPM_HOTKEY();
	}
	else
	{
		eventCloseAllSystemMenu();
	}
	LCD_redraw(false);
}

static void FRONTPANEL_BUTTONHANDLER_KEYER(void)
{
	TRX.CW_KEYER = !TRX.CW_KEYER;
	if(TRX.CW_KEYER)
		LCD_showTooltip("KEYER ON");
	else
		LCD_showTooltip("KEYER OFF");
}

static void FRONTPANEL_BUTTONHANDLER_STEP(void)
{
	if (!LCD_systemMenuOpened)
	{
		LCD_systemMenuOpened = true;
		SYSMENU_TRX_STEP_HOTKEY();
	}
	else
	{
		eventCloseAllSystemMenu();
	}
	LCD_redraw(false);
}

void FRONTPANEL_BUTTONHANDLER_BW(void)
{
	if (!LCD_systemMenuOpened)
	{
		LCD_systemMenuOpened = true;
		if (CurrentVFO()->Mode == TRX_MODE_CW_L || CurrentVFO()->Mode == TRX_MODE_CW_U)
			SYSMENU_AUDIO_BW_CW_HOTKEY();
		else if (CurrentVFO()->Mode == TRX_MODE_NFM || CurrentVFO()->Mode == TRX_MODE_WFM)
			SYSMENU_AUDIO_BW_FM_HOTKEY();
		else if (CurrentVFO()->Mode == TRX_MODE_AM)
			SYSMENU_AUDIO_BW_AM_HOTKEY();
		else
			SYSMENU_AUDIO_BW_SSB_HOTKEY();
	}
	else
	{
		eventCloseAllSystemMenu();
	}
	LCD_redraw(false);
}

void FRONTPANEL_BUTTONHANDLER_VOLUME(void)
{
	if (!LCD_systemMenuOpened)
	{
		LCD_systemMenuOpened = true;
		SYSMENU_AUDIO_VOLUME_HOTKEY();
	}
	else
	{
		eventCloseAllSystemMenu();
	}
	LCD_redraw(false);
}

void FRONTPANEL_BUTTONHANDLER_HPF(void)
{
	if (!LCD_systemMenuOpened)
	{
		LCD_systemMenuOpened = true;
		if (CurrentVFO()->Mode == TRX_MODE_CW_L || CurrentVFO()->Mode == TRX_MODE_CW_U)
			SYSMENU_AUDIO_HPF_CW_HOTKEY();
		else
			SYSMENU_AUDIO_HPF_SSB_HOTKEY();
	}
	else
	{
		eventCloseAllSystemMenu();
	}
	LCD_redraw(false);
}

void FRONTPANEL_BUTTONHANDLER_ArB(void) //A=B
{
	if (TRX.current_vfo)
		memcpy(&TRX.VFO_A, &TRX.VFO_B, sizeof TRX.VFO_B);
	else
		memcpy(&TRX.VFO_B, &TRX.VFO_A, sizeof TRX.VFO_B);
	
	LCD_showTooltip("VFO COPIED");
	
	LCD_UpdateQuery.TopButtons = true;
	LCD_UpdateQuery.FreqInfo = true;
	NeedSaveSettings = true;
}

void FRONTPANEL_BUTTONHANDLER_NOTCH(void)
{
	TRX_TemporaryMute();
	
	if (CurrentVFO()->NotchFC > CurrentVFO()->LPF_Filter_Width)
	{
		CurrentVFO()->NotchFC = CurrentVFO()->LPF_Filter_Width;
	}
	
	if (!CurrentVFO()->AutoNotchFilter)
	{
		InitAutoNotchReduction();
		CurrentVFO()->AutoNotchFilter = true;
	}
	else
		CurrentVFO()->AutoNotchFilter = false;

	LCD_UpdateQuery.StatusInfoGUI = true;
	LCD_UpdateQuery.TopButtons = true;
	NeedSaveSettings = true;
}

void FRONTPANEL_BUTTONHANDLER_NOTCH_MANUAL(void)
{
	if (CurrentVFO()->NotchFC > CurrentVFO()->LPF_Filter_Width)
		CurrentVFO()->NotchFC = CurrentVFO()->LPF_Filter_Width;
	CurrentVFO()->AutoNotchFilter = false;

	LCD_UpdateQuery.TopButtons = true;
	LCD_UpdateQuery.StatusInfoGUI = true;
	NeedSaveSettings = true;
}

static void FRONTPANEL_BUTTONHANDLER_SHIFT(void)
{
	TRX.ShiftEnabled = !TRX.ShiftEnabled;
	LCD_UpdateQuery.TopButtons = true;
	NeedSaveSettings = true;
}

static void FRONTPANEL_BUTTONHANDLER_CLAR(void)
{
	TRX.CLAR = !TRX.CLAR;
	LCD_UpdateQuery.TopButtons = true;
	NeedSaveSettings = true;
}

void FRONTPANEL_BUTTONHANDLER_LOCK(void)
{
	if (!LCD_systemMenuOpened)
		TRX.Locked = !TRX.Locked;
	LCD_UpdateQuery.TopButtons = true;
	LCD_UpdateQuery.StatusInfoBar = true;
	NeedSaveSettings = true;
}

static void FRONTPANEL_BUTTONHANDLER_HIDDEN_ENABLE(void)
{
	if (LCD_systemMenuOpened)
	{
		sysmenu_hiddenmenu_enabled = true;
		LCD_redraw(false);
	}
	LCD_UpdateQuery.TopButtons = true;
	LCD_UpdateQuery.StatusInfoBar = true;
	NeedSaveSettings = true;
}

void FRONTPANEL_BUTTONHANDLER_MENU(void)
{
	if (!LCD_systemMenuOpened)
		LCD_systemMenuOpened = true;
	else
		eventCloseSystemMenu();
	LCD_redraw(false);
}

void FRONTPANEL_BUTTONHANDLER_MUTE(void)
{
	TRX_Mute = !TRX_Mute;
	LCD_UpdateQuery.TopButtons = true;
	NeedSaveSettings = true;
}

static void FRONTPANEL_BUTTONHANDLER_BANDMAP(void)
{
	TRX.BandMapEnabled = !TRX.BandMapEnabled;
	
	if(TRX.BandMapEnabled)
		LCD_showTooltip("BANDMAP ON");
	else
		LCD_showTooltip("BANDMAP OFF");
	
	LCD_UpdateQuery.TopButtons = true;
	NeedSaveSettings = true;
}

static uint16_t FRONTPANEL_ReadMCP3008_Value(uint8_t channel, GPIO_TypeDef *CS_PORT, uint16_t CS_PIN)
{
	uint8_t outData[3] = {0};
	uint8_t inData[3] = {0};
	uint16_t mcp3008_value = 0;

	outData[0] = 0x18 | channel;
	bool res = SPI_Transmit(outData, inData, 3, CS_PORT, CS_PIN, false, SPI_FRONT_UNIT_PRESCALER);
	if (res == false)
		return 65535;
	mcp3008_value = (uint16_t)(0 | ((inData[1] & 0x3F) << 4) | (inData[2] & 0xF0 >> 4));

	//sendToDebug_uint16(mcp3008_value, false);
	return mcp3008_value;
}
