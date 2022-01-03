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


void FRONTPANEL_BUTTONHANDLER_BAND_P(void);
void FRONTPANEL_BUTTONHANDLER_BAND_N(void);
static void FRONTPANEL_BUTTONHANDLER_SQUELCH(void);
static void FRONTPANEL_BUTTONHANDLER_WPM(void);
static void FRONTPANEL_BUTTONHANDLER_KEYER(void);
static void FRONTPANEL_BUTTONHANDLER_SHIFT(void);
static void FRONTPANEL_BUTTONHANDLER_CLAR(void);
static void FRONTPANEL_BUTTONHANDLER_STEP(void);
static void FRONTPANEL_BUTTONHANDLER_BANDMAP(void);
static void FRONTPANEL_BUTTONHANDLER_HIDDEN_ENABLE(void);
static void FRONTPANEL_BUTTONHANDLER_ATTHOLD(void);
void FRONTPANEL_BUTTONHANDLER_AGC(void);
static void FRONTPANEL_BUTTONHANDLER_AGC_SPEED(void);
void FRONTPANEL_BUTTONHANDLER_NOTCH(void);
void FRONTPANEL_BUTTONHANDLER_FAST(void);
void FRONTPANEL_BUTTONHANDLER_MUTE(void);
static void FRONTPANEL_BUTTONHANDLER_AsB(void);
static void FRONTPANEL_BUTTONHANDLER_ArB(void);
void FRONTPANEL_BUTTONHANDLER_TUNE(void);
static void FRONTPANEL_BUTTONHANDLER_BW(void);
static void FRONTPANEL_BUTTONHANDLER_HPF(void);
static void FRONTPANEL_BUTTONHANDLER_MENU(void);
void FRONTPANEL_BUTTONHANDLER_LOCK(void);
static void FRONTPANEL_BUTTONHANDLER_PWR_P(void);
static void FRONTPANEL_BUTTONHANDLER_PWR_N(void);
void FRONTPANEL_BUTTONHANDLER_ZOOM_P(void);
static void FRONTPANEL_ENC2SW_click_handler(uint32_t parameter);
static void FRONTPANEL_ENC2SW_hold_handler(uint32_t parameter);

static bool FRONTPanel_MCP3008_1_Enabled = true;

static int32_t ENCODER_slowler = 0;
static uint32_t ENCODER_AValDeb = 0;
static uint32_t ENCODER2_AValDeb = 0;
//static uint8_t enc2_func_mode = 0;
//static bool enc2_func_mode = false; //false - fast-step, true - func mode (WPM, etc...)

#if (defined(BUTTONS_R7KBI)) //
static PERIPH_FrontPanel_Button PERIPH_FrontPanel_Static_Buttons[] = {
	{.port = 1, .channel = 0, .name = "", .tres_min = 0, .tres_max = 1023, .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = NULL, .holdHandler = NULL}, //not used
	{.port = 1, .channel = 1, .name = "", .tres_min = 0, .tres_max = 1023, .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = NULL, .holdHandler = NULL}, //not used
	{.port = 1, .channel = 2, .name = "", .tres_min = 0, .tres_max = 1023, .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = NULL, .holdHandler = NULL}, //not used
	{.port = 1, .channel = 3, .name = "", .tres_min = 0, .tres_max = 1023, .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = NULL, .holdHandler = NULL}, //not used
	{.port = 1, .channel = 4, .name = "", .tres_min = 0, .tres_max = 1023, .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = NULL, .holdHandler = NULL}, //not used
	{.port = 1, .channel = 5, .name = "MODE", .tres_min = 300, .tres_max = 500, .state = false, .prev_state = false, .work_in_menu = true, .clickHandler = FRONTPANEL_BUTTONHANDLER_MODE_N, .holdHandler = FRONTPANEL_BUTTONHANDLER_MODE_P}, //SB6
	{.port = 1, .channel = 5, .name = "BAND", .tres_min = 10, .tres_max = 300, .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_BAND_P, .holdHandler = FRONTPANEL_BUTTONHANDLER_BAND_N}, //SB1
	{.port = 1, .channel = 7, .name = "MENU", .tres_min = 300, .tres_max = 500, .state = false, .prev_state = false, .work_in_menu = true, .clickHandler = FRONTPANEL_BUTTONHANDLER_MENU, .holdHandler = FRONTPANEL_BUTTONHANDLER_MENU}, //SB1
};

static PERIPH_FrontPanel_Button PERIPH_FrontPanel_BottomScroll_Buttons[BOTTOM_SCROLLBUTTONS_GROUPS_COUNT][5] = {
	{
		{.port = 1, .channel = 5, .name = "PRE", .tres_min = 500, .tres_max = 700, .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_PRE, .holdHandler = FRONTPANEL_BUTTONHANDLER_PRE}, //SB2
		{.port = 1, .channel = 6, .name = "ATT", .tres_min = 500, .tres_max = 700, .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_ATT, .holdHandler = FRONTPANEL_BUTTONHANDLER_ATTHOLD}, //SB3
		{.port = 1, .channel = 6, .name = "BW", .tres_min = 300, .tres_max = 500, .state = false, .prev_state = false, .work_in_menu = true, .clickHandler = FRONTPANEL_BUTTONHANDLER_BW, .holdHandler = FRONTPANEL_BUTTONHANDLER_HPF}, //SB4
		{.port = 1, .channel = 6, .name = "A/B", .tres_min = 10, .tres_max = 300, .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_AsB, .holdHandler = FRONTPANEL_BUTTONHANDLER_ArB}, //SB5
		{.port = 1, .channel = 7, .name = "POWER", .tres_min = 10, .tres_max = 300, .state = false, .prev_state = false, .work_in_menu = true, .clickHandler = FRONTPANEL_BUTTONHANDLER_RF_POWER, .holdHandler = FRONTPANEL_BUTTONHANDLER_RF_POWER}, //SB2
	},
	{
		{.port = 1, .channel = 5, .name = "AGC", .tres_min = 500, .tres_max = 700, .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_AGC, .holdHandler = FRONTPANEL_BUTTONHANDLER_AGC_SPEED}, //SB2
		{.port = 1, .channel = 6, .name = "ZOOM", .tres_min = 500, .tres_max = 700, .state = false, .prev_state = false, .work_in_menu = true, .clickHandler = FRONTPANEL_BUTTONHANDLER_ZOOM_P, .holdHandler = FRONTPANEL_BUTTONHANDLER_ZOOM_P}, //SB3
		{.port = 1, .channel = 6, .name = "NOTCH", .tres_min = 300, .tres_max = 500, .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_NOTCH, .holdHandler = FRONTPANEL_BUTTONHANDLER_NOTCH}, //SB4
		{.port = 1, .channel = 6, .name = "FAST", .tres_min = 10, .tres_max = 300, .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_FAST, .holdHandler = FRONTPANEL_BUTTONHANDLER_FAST}, //SB5
		{.port = 1, .channel = 7, .name = "CLAR", .tres_min = 10, .tres_max = 300, .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_CLAR, .holdHandler = FRONTPANEL_BUTTONHANDLER_CLAR}, //SB3
	},
	{
		{.port = 1, .channel = 5, .name = "VOLUME", .tres_min = 500, .tres_max = 700, .state = false, .prev_state = false, .work_in_menu = true, .clickHandler = FRONTPANEL_BUTTONHANDLER_VOLUME, .holdHandler = FRONTPANEL_BUTTONHANDLER_VOLUME}, //SB2
		{.port = 1, .channel = 6, .name = "BANDMAP", .tres_min = 500, .tres_max = 700, .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_BANDMAP, .holdHandler = FRONTPANEL_BUTTONHANDLER_BANDMAP}, //SB3
		{.port = 1, .channel = 6, .name = "MUTE", .tres_min = 300, .tres_max = 500, .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_MUTE, .holdHandler = FRONTPANEL_BUTTONHANDLER_MUTE}, //SB4
		{.port = 1, .channel = 6, .name = "LOCK", .tres_min = 10, .tres_max = 300, .state = false, .prev_state = false, .work_in_menu = true, .clickHandler = FRONTPANEL_BUTTONHANDLER_LOCK, .holdHandler = FRONTPANEL_BUTTONHANDLER_LOCK}, //SB5
		{.port = 1, .channel = 7, .name = "WPM", .tres_min = 10, .tres_max = 300, .state = false, .prev_state = false, .work_in_menu = true, .clickHandler = FRONTPANEL_BUTTONHANDLER_WPM, .holdHandler = FRONTPANEL_BUTTONHANDLER_WPM}, //SB4
	},
	{
		{.port = 1, .channel = 5, .name = "BAND-", .tres_min = 500, .tres_max = 700, .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_BAND_N, .holdHandler = FRONTPANEL_BUTTONHANDLER_BAND_N}, //SB2
		{.port = 1, .channel = 6, .name = "BAND+", .tres_min = 500, .tres_max = 700, .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_BAND_P, .holdHandler = FRONTPANEL_BUTTONHANDLER_BAND_P}, //SB3
		{.port = 1, .channel = 6, .name = "MODE-", .tres_min = 300, .tres_max = 500, .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_MODE_N, .holdHandler = FRONTPANEL_BUTTONHANDLER_MODE_N}, //SB4
		{.port = 1, .channel = 6, .name = "MODE+", .tres_min = 10, .tres_max = 300, .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_MODE_P, .holdHandler = FRONTPANEL_BUTTONHANDLER_MODE_P}, //SB5
		{.port = 1, .channel = 7, .name = "KEYER", .tres_min = 10, .tres_max = 300, .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_KEYER, .holdHandler = FRONTPANEL_BUTTONHANDLER_KEYER}, //SB5
	},
	{
		{.port = 1, .channel = 5, .name = "BW-", .tres_min = 500, .tres_max = 700, .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_BW_N, .holdHandler = FRONTPANEL_BUTTONHANDLER_BW_N}, //SB2
		{.port = 1, .channel = 6, .name = "BW+", .tres_min = 500, .tres_max = 700, .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_BW_P, .holdHandler = FRONTPANEL_BUTTONHANDLER_BW_P}, //SB3
		{.port = 1, .channel = 6, .name = "PWR-", .tres_min = 300, .tres_max = 500, .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_PWR_N, .holdHandler = FRONTPANEL_BUTTONHANDLER_PWR_N}, //SB4
		{.port = 1, .channel = 6, .name = "PWR+", .tres_min = 10, .tres_max = 300, .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_PWR_P, .holdHandler = FRONTPANEL_BUTTONHANDLER_PWR_P}, //SB5
		{.port = 1, .channel = 7, .name = "TUNE", .tres_min = 10, .tres_max = 300, .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_TUNE, .holdHandler = FRONTPANEL_BUTTONHANDLER_TUNE}, //SB2
		//{.port = 1, .channel = 7, .name = "ZOOM", .tres_min = 10, .tres_max = 300, .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_ZOOM_P, .holdHandler = FRONTPANEL_BUTTONHANDLER_ZOOM_P}, //SB2	
	},
	{
		{.port = 1, .channel = 5, .name = "MODE", .tres_min = 500, .tres_max = 700, .state = false, .prev_state = false, .work_in_menu = true, .clickHandler = FRONTPANEL_BUTTONHANDLER_MODE_N, .holdHandler = FRONTPANEL_BUTTONHANDLER_MODE_P}, //SB6
	  {.port = 1, .channel = 6, .name = "BAND", .tres_min = 500, .tres_max = 700, .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_BAND_P, .holdHandler = FRONTPANEL_BUTTONHANDLER_BAND_N}, //SB1
    {.port = 1, .channel = 6, .name = "PRE", .tres_min = 300, .tres_max = 500, .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_PRE, .holdHandler = FRONTPANEL_BUTTONHANDLER_PRE}, //SB2	
		{.port = 1, .channel = 6, .name = "A/B", .tres_min = 10, .tres_max = 300, .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_AsB, .holdHandler = FRONTPANEL_BUTTONHANDLER_ArB}, //SB5
		{.port = 1, .channel = 7, .name = "POWER", .tres_min = 10, .tres_max = 300, .state = false, .prev_state = false, .work_in_menu = true, .clickHandler = FRONTPANEL_BUTTONHANDLER_RF_POWER, .holdHandler = FRONTPANEL_BUTTONHANDLER_RF_POWER}, //SB2
	},
};
#else
static PERIPH_FrontPanel_Button PERIPH_FrontPanel_Static_Buttons[] = {
	{.port = 1, .channel = 0, .name = "", .tres_min = 0, .tres_max = 1023, .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = NULL, .holdHandler = NULL}, //not used
	{.port = 1, .channel = 1, .name = "", .tres_min = 0, .tres_max = 1023, .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = NULL, .holdHandler = NULL}, //not used
	{.port = 1, .channel = 2, .name = "", .tres_min = 0, .tres_max = 1023, .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = NULL, .holdHandler = NULL}, //not used
	{.port = 1, .channel = 7, .name = "PRE", .tres_min = 500, .tres_max = 700, .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_PRE, .holdHandler = FRONTPANEL_BUTTONHANDLER_PRE}, //SB13
	{.port = 1, .channel = 7, .name = "ATT", .tres_min = 700, .tres_max = 950, .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_ATT, .holdHandler = FRONTPANEL_BUTTONHANDLER_ATTHOLD}, //SB14
	{.port = 1, .channel = 7, .name = "MODE", .tres_min = 300, .tres_max = 500, .state = false, .prev_state = false, .work_in_menu = true, .clickHandler = FRONTPANEL_BUTTONHANDLER_MODE_N, .holdHandler = FRONTPANEL_BUTTONHANDLER_MODE_P}, //SB6
	{.port = 1, .channel = 7, .name = "BAND", .tres_min = 10, .tres_max = 300, .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_BAND_P, .holdHandler = FRONTPANEL_BUTTONHANDLER_BAND_N}, //SB1
	{.port = 1, .channel = 6, .name = "MENU", .tres_min = 10, .tres_max = 300, .state = false, .prev_state = false, .work_in_menu = true, .clickHandler = FRONTPANEL_BUTTONHANDLER_MENU, .holdHandler = FRONTPANEL_BUTTONHANDLER_MENU}, //SB1
};

static PERIPH_FrontPanel_Button PERIPH_FrontPanel_BottomScroll_Buttons[BOTTOM_SCROLLBUTTONS_GROUPS_COUNT][5] = {
	{
		{.port = 1, .channel = 5, .name = "PRE", .tres_min = 500, .tres_max = 700, .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_PRE, .holdHandler = FRONTPANEL_BUTTONHANDLER_PRE}, //SB2
		{.port = 1, .channel = 5, .name = "ATT", .tres_min = 300, .tres_max = 500, .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_ATT, .holdHandler = FRONTPANEL_BUTTONHANDLER_ATTHOLD}, //SB3
		{.port = 1, .channel = 5, .name = "BW", .tres_min = 10, .tres_max = 300, .state = false, .prev_state = false, .work_in_menu = true, .clickHandler = FRONTPANEL_BUTTONHANDLER_BW, .holdHandler = FRONTPANEL_BUTTONHANDLER_HPF}, //SB4
		{.port = 1, .channel = 6, .name = "A/B", .tres_min = 500, .tres_max = 700, .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_AsB, .holdHandler = FRONTPANEL_BUTTONHANDLER_ArB}, //SB5
		{.port = 1, .channel = 6, .name = "POWER", .tres_min = 300, .tres_max = 500, .state = false, .prev_state = false, .work_in_menu = true, .clickHandler = FRONTPANEL_BUTTONHANDLER_RF_POWER, .holdHandler = FRONTPANEL_BUTTONHANDLER_RF_POWER}, //SB2
	},
	{
		{.port = 1, .channel = 5, .name = "AGC",   .tres_min = 500, .tres_max = 700, .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_AGC, .holdHandler = FRONTPANEL_BUTTONHANDLER_AGC_SPEED}, //SB2
		{.port = 1, .channel = 5, .name = "ZOOM",  .tres_min = 300, .tres_max = 500, .state = false, .prev_state = false, .work_in_menu = true, .clickHandler = FRONTPANEL_BUTTONHANDLER_ZOOM_P, .holdHandler = FRONTPANEL_BUTTONHANDLER_ZOOM_P}, //SB3
		{.port = 1, .channel = 5, .name = "NOTCH", .tres_min = 10, .tres_max = 300, .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_NOTCH, .holdHandler = FRONTPANEL_BUTTONHANDLER_NOTCH}, //SB4
		{.port = 1, .channel = 6, .name = "FAST",  .tres_min = 500, .tres_max = 700, .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_FAST, .holdHandler = FRONTPANEL_BUTTONHANDLER_FAST}, //SB5
		{.port = 1, .channel = 6, .name = "CLAR",  .tres_min = 300, .tres_max = 500, .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_CLAR, .holdHandler = FRONTPANEL_BUTTONHANDLER_CLAR}, //SB3
	},
	{
		{.port = 1, .channel = 5, .name = "VOLUME",  .tres_min = 500, .tres_max = 700, .state = false, .prev_state = false, .work_in_menu = true, .clickHandler = FRONTPANEL_BUTTONHANDLER_VOLUME, .holdHandler = FRONTPANEL_BUTTONHANDLER_VOLUME}, //SB2
		{.port = 1, .channel = 5, .name = "BANDMAP", .tres_min = 300, .tres_max = 500, .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_BANDMAP, .holdHandler = FRONTPANEL_BUTTONHANDLER_BANDMAP}, //SB3
		{.port = 1, .channel = 5, .name = "MUTE",    .tres_min = 10, .tres_max = 300, .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_MUTE, .holdHandler = FRONTPANEL_BUTTONHANDLER_MUTE}, //SB4
		{.port = 1, .channel = 6, .name = "LOCK",    .tres_min = 500, .tres_max = 700, .state = false, .prev_state = false, .work_in_menu = true, .clickHandler = FRONTPANEL_BUTTONHANDLER_LOCK, .holdHandler = FRONTPANEL_BUTTONHANDLER_LOCK}, //SB5
		{.port = 1, .channel = 6, .name = "WPM",     .tres_min = 300, .tres_max = 500, .state = false, .prev_state = false, .work_in_menu = true, .clickHandler = FRONTPANEL_BUTTONHANDLER_WPM, .holdHandler = FRONTPANEL_BUTTONHANDLER_WPM}, //SB4
	},
	{
		{.port = 1, .channel = 5, .name = "BAND-", .tres_min = 500, .tres_max = 700, .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_BAND_N, .holdHandler = FRONTPANEL_BUTTONHANDLER_BAND_N}, //SB2
		{.port = 1, .channel = 5, .name = "BAND+", .tres_min = 300, .tres_max = 500, .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_BAND_P, .holdHandler = FRONTPANEL_BUTTONHANDLER_BAND_P}, //SB3
		{.port = 1, .channel = 5, .name = "MODE-", .tres_min = 10, .tres_max = 300, .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_MODE_N, .holdHandler = FRONTPANEL_BUTTONHANDLER_MODE_N}, //SB4
		{.port = 1, .channel = 6, .name = "MODE+", .tres_min = 500, .tres_max = 700, .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_MODE_P, .holdHandler = FRONTPANEL_BUTTONHANDLER_MODE_P}, //SB5
		{.port = 1, .channel = 6, .name = "KEYER", .tres_min = 300, .tres_max = 500, .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_KEYER, .holdHandler = FRONTPANEL_BUTTONHANDLER_KEYER}, //SB5
	},
	{
		{.port = 1, .channel = 5, .name = "BW-",  .tres_min = 500, .tres_max = 700, .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_BW_N, .holdHandler = FRONTPANEL_BUTTONHANDLER_BW_N}, //SB2
		{.port = 1, .channel = 5, .name = "BW+",  .tres_min = 300, .tres_max = 500, .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_BW_P, .holdHandler = FRONTPANEL_BUTTONHANDLER_BW_P}, //SB3
		{.port = 1, .channel = 5, .name = "PWR-", .tres_min = 10, .tres_max = 300, .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_PWR_N, .holdHandler = FRONTPANEL_BUTTONHANDLER_PWR_N}, //SB4
		{.port = 1, .channel = 6, .name = "PWR+", .tres_min = 500, .tres_max = 700, .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_PWR_P, .holdHandler = FRONTPANEL_BUTTONHANDLER_PWR_P}, //SB5
		{.port = 1, .channel = 6, .name = "TUNE", .tres_min = 300, .tres_max = 500, .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_TUNE, .holdHandler = FRONTPANEL_BUTTONHANDLER_TUNE}, //SB2
	},
	{
		{.port = 1, .channel = 5, .name = "MODE",  .tres_min = 500, .tres_max = 700, .state = false, .prev_state = false, .work_in_menu = true, .clickHandler = FRONTPANEL_BUTTONHANDLER_MODE_N, .holdHandler = FRONTPANEL_BUTTONHANDLER_MODE_P}, //SB6
	  {.port = 1, .channel = 5, .name = "BAND",  .tres_min = 300, .tres_max = 500, .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_BAND_P, .holdHandler = FRONTPANEL_BUTTONHANDLER_BAND_N}, //SB1
    {.port = 1, .channel = 5, .name = "PRE",   .tres_min = 10, .tres_max = 300, .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_PRE, .holdHandler = FRONTPANEL_BUTTONHANDLER_PRE}, //SB2	
		{.port = 1, .channel = 6, .name = "A/B",   .tres_min = 500, .tres_max = 700, .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_AsB, .holdHandler = FRONTPANEL_BUTTONHANDLER_ArB}, //SB5
		{.port = 1, .channel = 6, .name = "POWER", .tres_min = 300, .tres_max = 500, .state = false, .prev_state = false, .work_in_menu = true, .clickHandler = FRONTPANEL_BUTTONHANDLER_RF_POWER, .holdHandler = FRONTPANEL_BUTTONHANDLER_RF_POWER}, //SB2
	},
};
#endif

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
	if (TRX_on_TX() && TRX.Encoder_OFF == true)
		return;
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
	//if (TRX.Locked)
		//return;

	if (LCD_systemMenuOpened)
	{
		eventSecRotateSystemMenu(direction);
		return;
	}
	else
	{
		if (TRX.TX_func_mode == 0) //function buttons scroll
		{
			PERIPH_FrontPanel_BottomScroll_index += direction;
			if(PERIPH_FrontPanel_BottomScroll_index < 0)
				PERIPH_FrontPanel_BottomScroll_index = BOTTOM_SCROLLBUTTONS_GROUPS_COUNT - 1;
			if(PERIPH_FrontPanel_BottomScroll_index >= BOTTOM_SCROLLBUTTONS_GROUPS_COUNT)
				PERIPH_FrontPanel_BottomScroll_index = 0;
			PERIPH_FrontPanel_BottomScroll_Buttons_Active = PERIPH_FrontPanel_BottomScroll_Buttons[PERIPH_FrontPanel_BottomScroll_index];
			LCD_UpdateQuery.TopButtons = true;
		}
		if (TRX.TX_func_mode == 1) //set volume
		{
			int16_t newvolume = (int16_t)TRX.Volume + direction * TRX.Volume_Step; // 
			newvolume /= TRX.Volume_Step;
			newvolume *= TRX.Volume_Step;
			if(newvolume > 100)
				newvolume = 100;
			if(newvolume < 0)
				newvolume = 0;
			TRX.Volume = newvolume;
			char str[32] = {0};
			sprintf(str, "VOL: %d%%",TRX.Volume);
			LCD_showTooltip(str);
		}
//##################################################################################
		if (TRX.TX_func_mode == 2) //fast step mode
		{
			
			VFO *vfo = CurrentVFO();
			uint32_t newfreq = 0;
			float64_t freq_round = 0;
			float64_t step = 0;
	if (TRX.Fast)
	{
		step = (float32_t)TRX.FRQ_FAST_STEP * 2; // Fast
		freq_round = roundf((float64_t)vfo->Freq / step) * step;
		newfreq = (uint32_t)((int32_t)freq_round + (int32_t)step * direction);
		
//		newfreq = (uint32_t)((int32_t)vfo->Freq + (int32_t)((float32_t)TRX.FRQ_FAST_STEP * direction));
//		if ((vfo->Freq % TRX.FRQ_FAST_STEP) > 0 && fabsf(direction) <= 1.0f)
//			newfreq = vfo->Freq / TRX.FRQ_FAST_STEP * TRX.FRQ_FAST_STEP;
	}
	else
	{
		step = (float32_t)TRX.FRQ_STEP * 2; // Regular
		freq_round = roundf((float64_t)vfo->Freq / step) * step;
		newfreq = (uint32_t)((int32_t)freq_round + (int32_t)step * direction);
		
//		newfreq = (uint32_t)((int32_t)vfo->Freq + (int32_t)((float32_t)TRX.FRQ_STEP * direction));
//		if ((vfo->Freq % TRX.FRQ_STEP) > 0 && fabsf(direction) <= 1.0f)
//			newfreq = vfo->Freq / TRX.FRQ_STEP * TRX.FRQ_STEP;
	}
	TRX_setFrequency(newfreq, vfo);
	LCD_UpdateQuery.FreqInfo = true;

		}
//##################################################################################
	}
}


void FRONTPANEL_check_ENC2SW(void)
{	
	static uint32_t menu_enc2_click_starttime = 0;
	static bool ENC2SW_Last = true;
	static bool ENC2SW_clicked = false;
	static bool ENC2SW_hold_start = false;
	static bool ENC2SW_holded = false;
	ENC2SW_clicked = false;
	ENC2SW_holded = false;

	if (TRX.Locked)
		return;

	bool ENC2SW_AND_TOUCH_Now = HAL_GPIO_ReadPin(ENC2_SW_GPIO_Port, ENC2_SW_Pin);
	//check hold and click
	if (ENC2SW_Last != ENC2SW_AND_TOUCH_Now)
	{
		ENC2SW_Last = ENC2SW_AND_TOUCH_Now;
		if (!ENC2SW_AND_TOUCH_Now)
		{
			menu_enc2_click_starttime = HAL_GetTick();
			ENC2SW_hold_start = true;
		}
	}
	if (!ENC2SW_AND_TOUCH_Now && ENC2SW_hold_start)
	{
		if ((HAL_GetTick() - menu_enc2_click_starttime) > KEY_HOLD_TIME)
		{
			ENC2SW_holded = true;
			ENC2SW_hold_start = false;
		}
	}
	if (ENC2SW_AND_TOUCH_Now && ENC2SW_hold_start)
	{
		if ((HAL_GetTick() - menu_enc2_click_starttime) > 1)
		{
			ENC2SW_clicked = true;
			ENC2SW_hold_start = false;
		}
	}

	//ENC2 Button hold
	if (ENC2SW_holded)
	{
		FRONTPANEL_ENC2SW_hold_handler(0);
	}

	//ENC2 Button click
	if (ENC2SW_clicked)
	{
		menu_enc2_click_starttime = HAL_GetTick();
		FRONTPANEL_ENC2SW_click_handler(0);
	
	}
}

static void FRONTPANEL_ENC2SW_click_handler(uint32_t parameter)
{
	//ENC2 CLICK
	if (!LCD_systemMenuOpened)
	{
		TRX.TX_func_mode++; //enc2 rotary mode
		if(TRX.TX_func_mode >= 3)
			TRX.TX_func_mode = 0;
		if (TRX.TX_func_mode == 0)
			LCD_showTooltip("BUTTONS");
		if (TRX.TX_func_mode == 1)
			LCD_showTooltip("SET VOLUME");
		if (TRX.TX_func_mode == 2)
		  LCD_showTooltip("FAST STEP");
		 
	
	
	}
	else
	{
		if (LCD_systemMenuOpened)
		{
			//navigate in menu
			SYSMENU_eventSecEncoderClickSystemMenu();
		}
	}
}

static void FRONTPANEL_ENC2SW_hold_handler(uint32_t parameter)
{
	FRONTPANEL_BUTTONHANDLER_MENU();
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
		sendToDebug_uint16(mcp3008_value, false);
		sendToDebug_flush();*/
		
		//set state
		if (mcp3008_value >= PERIPH_FrontPanel_Buttons[b].tres_min && mcp3008_value < PERIPH_FrontPanel_Buttons[b].tres_max)
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
//----------------------------------------------------------------------------
void FRONTPANEL_BUTTONHANDLER_ZOOM_P(void)
{
    if (TRX.FFT_Zoom == 1)
      TRX.FFT_Zoom = 2;
    else if (TRX.FFT_Zoom == 2)
      TRX.FFT_Zoom = 4;
    else if (TRX.FFT_Zoom == 4)
      TRX.FFT_Zoom = 8;
    else if (TRX.FFT_Zoom == 8)
      TRX.FFT_Zoom = 1;
   FFT_Init();
	//LCD_redraw(false);
}

//----------------------------------------------------------------------------

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
	LCD_UpdateQuery.StatusInfoGUIRedraw = true;
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

void FRONTPANEL_BUTTONHANDLER_MODE_P(void)
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

void FRONTPANEL_BUTTONHANDLER_MODE_N(void)
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

void FRONTPANEL_BUTTONHANDLER_BAND_P(void)
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

void FRONTPANEL_BUTTONHANDLER_BAND_N(void)
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
	
	if (CurrentVFO()->NotchFC > CurrentVFO()->RX_LPF_Filter_Width)
	{
		CurrentVFO()->NotchFC = CurrentVFO()->RX_LPF_Filter_Width;
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
	if (CurrentVFO()->NotchFC > CurrentVFO()->RX_LPF_Filter_Width)
		CurrentVFO()->NotchFC = CurrentVFO()->RX_LPF_Filter_Width;
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

static void FRONTPANEL_BUTTONHANDLER_PWR_P(void)
{
	int16_t newval = (int16_t)TRX.RF_Power + 10;
	newval /= 10;
	newval *= 10;
	if(newval > 100)
		newval = 100;
	TRX.RF_Power = newval;
	char str[32] = {0};
	sprintf(str, "PWR: %d%%",TRX.RF_Power);
	LCD_showTooltip(str);
	NeedSaveSettings = true;
}

static void FRONTPANEL_BUTTONHANDLER_PWR_N(void)
{
	int16_t newval = (int16_t)TRX.RF_Power - 10;
	newval /= 10;
	newval *= 10;
	if(newval < 0)
		newval = 0;
	TRX.RF_Power = newval;
	char str[32] = {0};
	sprintf(str, "PWR: %d%%",TRX.RF_Power);
	LCD_showTooltip(str);
	NeedSaveSettings = true;
}

void FRONTPANEL_BUTTONHANDLER_BW_P(void)
{
	char str[32] = {0};
	switch (CurrentVFO()->Mode)
	{
		case TRX_MODE_LSB:
		case TRX_MODE_USB:
		case TRX_MODE_DIGI_L:
		case TRX_MODE_DIGI_U:
		case TRX_MODE_IQ:
		case TRX_MODE_LOOPBACK:
		case TRX_MODE_NO_TX:
			if (TRX.RX_SSB_LPF_Filter == 0)
				TRX.RX_SSB_LPF_Filter = 1400;
			if (TRX.RX_SSB_LPF_Filter == 1400)
				TRX.RX_SSB_LPF_Filter = 1600;
			else if (TRX.RX_SSB_LPF_Filter == 1600)
				TRX.RX_SSB_LPF_Filter = 1800;
			else if (TRX.RX_SSB_LPF_Filter == 1800)
				TRX.RX_SSB_LPF_Filter = 2100;
			else if (TRX.RX_SSB_LPF_Filter == 2100)
				TRX.RX_SSB_LPF_Filter = 2300;
			else if (TRX.RX_SSB_LPF_Filter == 2300)
				TRX.RX_SSB_LPF_Filter = 2500;
			else if (TRX.RX_SSB_LPF_Filter == 2500)
				TRX.RX_SSB_LPF_Filter = 2700;
			else if (TRX.RX_SSB_LPF_Filter == 2700)
				TRX.RX_SSB_LPF_Filter = 2900;
			else if (TRX.RX_SSB_LPF_Filter == 2900)
				TRX.RX_SSB_LPF_Filter = 3000;
			else if (TRX.RX_SSB_LPF_Filter == 3000)
				TRX.RX_SSB_LPF_Filter = 3200;
			else if (TRX.RX_SSB_LPF_Filter == 3200)
				TRX.RX_SSB_LPF_Filter = 3400;
			sprintf(str, "BW: %d",TRX.RX_SSB_LPF_Filter);
		break;
		
		case TRX_MODE_CW_L:
		case TRX_MODE_CW_U:
			if (TRX.CW_LPF_Filter == 100)
				TRX.CW_LPF_Filter = 150;
			else if (TRX.CW_LPF_Filter == 150)
				TRX.CW_LPF_Filter = 200;
			else if (TRX.CW_LPF_Filter == 200)
				TRX.CW_LPF_Filter = 250;
			else if (TRX.CW_LPF_Filter == 250)
				TRX.CW_LPF_Filter = 300;
			else if (TRX.CW_LPF_Filter == 300)
				TRX.CW_LPF_Filter = 350;
			else if (TRX.CW_LPF_Filter == 350)
				TRX.CW_LPF_Filter = 400;
			else if (TRX.CW_LPF_Filter == 400)
				TRX.CW_LPF_Filter = 450;
			else if (TRX.CW_LPF_Filter == 450)
				TRX.CW_LPF_Filter = 500;
			else if (TRX.CW_LPF_Filter == 500)
				TRX.CW_LPF_Filter = 550;
			else if (TRX.CW_LPF_Filter == 550)
				TRX.CW_LPF_Filter = 600;
			else if (TRX.CW_LPF_Filter == 600)
				TRX.CW_LPF_Filter = 650;
			else if (TRX.CW_LPF_Filter == 650)
				TRX.CW_LPF_Filter = 700;
			else if (TRX.CW_LPF_Filter == 700)
				TRX.CW_LPF_Filter = 750;
			else if (TRX.CW_LPF_Filter == 750)
				TRX.CW_LPF_Filter = 800;
			else if (TRX.CW_LPF_Filter == 800)
				TRX.CW_LPF_Filter = 850;
			else if (TRX.CW_LPF_Filter == 850)
				TRX.CW_LPF_Filter = 900;
			else if (TRX.CW_LPF_Filter == 900)
				TRX.CW_LPF_Filter = 950;
			else if (TRX.CW_LPF_Filter == 950)
				TRX.CW_LPF_Filter = 1000;
			sprintf(str, "BW: %d",TRX.CW_LPF_Filter);
		break;
		
		case TRX_MODE_NFM:
		case TRX_MODE_WFM:
			if (TRX.RX_FM_LPF_Filter == 5000)
				TRX.RX_FM_LPF_Filter = 6000;
			else if (TRX.RX_FM_LPF_Filter == 6000)
				TRX.RX_FM_LPF_Filter = 7000;
			else if (TRX.RX_FM_LPF_Filter == 7000)
				TRX.RX_FM_LPF_Filter = 8000;
			else if (TRX.RX_FM_LPF_Filter == 8000)
				TRX.RX_FM_LPF_Filter = 9000;
			else if (TRX.RX_FM_LPF_Filter == 9000)
				TRX.RX_FM_LPF_Filter = 10000;
			else if (TRX.RX_FM_LPF_Filter == 10000)
				TRX.RX_FM_LPF_Filter = 15000;
			else if (TRX.RX_FM_LPF_Filter == 15000)
				TRX.RX_FM_LPF_Filter = 20000;
			sprintf(str, "BW: %d",TRX.RX_FM_LPF_Filter);
		break;
		
		case TRX_MODE_AM:
			if (TRX.RX_AM_LPF_Filter == 2100)
				TRX.RX_AM_LPF_Filter = 2300;
			else if (TRX.RX_AM_LPF_Filter == 2300)
				TRX.RX_AM_LPF_Filter = 2500;
			else if (TRX.RX_AM_LPF_Filter == 2500)
				TRX.RX_AM_LPF_Filter = 2700;
			else if (TRX.RX_AM_LPF_Filter == 2700)
				TRX.RX_AM_LPF_Filter = 2900;
			else if (TRX.RX_AM_LPF_Filter == 2900)
				TRX.RX_AM_LPF_Filter = 3000;
			else if (TRX.RX_AM_LPF_Filter == 3000)
				TRX.RX_AM_LPF_Filter = 3200;
			else if (TRX.RX_AM_LPF_Filter == 3200)
				TRX.RX_AM_LPF_Filter = 3400;
			else if (TRX.RX_AM_LPF_Filter == 3400)
				TRX.RX_AM_LPF_Filter = 3600;
			else if (TRX.RX_AM_LPF_Filter == 3600)
				TRX.RX_AM_LPF_Filter = 3800;
			else if (TRX.RX_AM_LPF_Filter == 3800)
				TRX.RX_AM_LPF_Filter = 4000;
			else if (TRX.RX_AM_LPF_Filter == 4000)
				TRX.RX_AM_LPF_Filter = 4500;
			else if (TRX.RX_AM_LPF_Filter == 4500)
				TRX.RX_AM_LPF_Filter = 5000;
			else if (TRX.RX_AM_LPF_Filter == 5000)
				TRX.RX_AM_LPF_Filter = 6000;
			else if (TRX.RX_AM_LPF_Filter == 6000)
				TRX.RX_AM_LPF_Filter = 7000;
			else if (TRX.RX_AM_LPF_Filter == 7000)
				TRX.RX_AM_LPF_Filter = 8000;
			else if (TRX.RX_AM_LPF_Filter == 8000)
				TRX.RX_AM_LPF_Filter = 9000;
			else if (TRX.RX_AM_LPF_Filter == 9000)
				TRX.RX_AM_LPF_Filter = 10000;
			sprintf(str, "BW: %d",TRX.RX_AM_LPF_Filter);
		break;
	}
	
	TRX_setMode(SecondaryVFO()->Mode, SecondaryVFO());
	TRX_setMode(CurrentVFO()->Mode, CurrentVFO());
	LCD_showTooltip(str);
	NeedSaveSettings = true;
}

 void FRONTPANEL_BUTTONHANDLER_BW_N(void)
{
	char str[32] = {0};
	switch (CurrentVFO()->Mode)
	{
		case TRX_MODE_LSB:
		case TRX_MODE_USB:
		case TRX_MODE_DIGI_L:
		case TRX_MODE_DIGI_U:
		case TRX_MODE_IQ:
		case TRX_MODE_LOOPBACK:
		case TRX_MODE_NO_TX:
			if (TRX.RX_SSB_LPF_Filter == 1600)
				TRX.RX_SSB_LPF_Filter = 1400;
			else if (TRX.RX_SSB_LPF_Filter == 1800)
				TRX.RX_SSB_LPF_Filter = 1600;
			else if (TRX.RX_SSB_LPF_Filter == 2100)
				TRX.RX_SSB_LPF_Filter = 1800;
			else if (TRX.RX_SSB_LPF_Filter == 2300)
				TRX.RX_SSB_LPF_Filter = 2100;
			else if (TRX.RX_SSB_LPF_Filter == 2500)
				TRX.RX_SSB_LPF_Filter = 2300;
			else if (TRX.RX_SSB_LPF_Filter == 2700)
				TRX.RX_SSB_LPF_Filter = 2500;
			else if (TRX.RX_SSB_LPF_Filter == 2900)
				TRX.RX_SSB_LPF_Filter = 2700;
			else if (TRX.RX_SSB_LPF_Filter == 3000)
				TRX.RX_SSB_LPF_Filter = 2900;
			else if (TRX.RX_SSB_LPF_Filter == 3200)
				TRX.RX_SSB_LPF_Filter = 3000;
			else if (TRX.RX_SSB_LPF_Filter == 3400)
				TRX.RX_SSB_LPF_Filter = 3200;
			sprintf(str, "BW: %d",TRX.RX_SSB_LPF_Filter);
		break;
		
		case TRX_MODE_CW_L:
		case TRX_MODE_CW_U:
			if (TRX.CW_LPF_Filter == 1000)
				TRX.CW_LPF_Filter = 950;
			else if (TRX.CW_LPF_Filter == 950)
				TRX.CW_LPF_Filter = 900;
			else if (TRX.CW_LPF_Filter == 900)
				TRX.CW_LPF_Filter = 850;
			else if (TRX.CW_LPF_Filter == 850)
				TRX.CW_LPF_Filter = 800;
			else if (TRX.CW_LPF_Filter == 800)
				TRX.CW_LPF_Filter = 750;
			else if (TRX.CW_LPF_Filter == 750)
				TRX.CW_LPF_Filter = 700;
			else if (TRX.CW_LPF_Filter == 700)
				TRX.CW_LPF_Filter = 650;
			else if (TRX.CW_LPF_Filter == 650)
				TRX.CW_LPF_Filter = 600;
			else if (TRX.CW_LPF_Filter == 600)
				TRX.CW_LPF_Filter = 550;
			else if (TRX.CW_LPF_Filter == 550)
				TRX.CW_LPF_Filter = 500;
			else if (TRX.CW_LPF_Filter == 500)
				TRX.CW_LPF_Filter = 450;
			else if (TRX.CW_LPF_Filter == 450)
				TRX.CW_LPF_Filter = 400;
			else if (TRX.CW_LPF_Filter == 400)
				TRX.CW_LPF_Filter = 350;
			else if (TRX.CW_LPF_Filter == 350)
				TRX.CW_LPF_Filter = 300;
			else if (TRX.CW_LPF_Filter == 300)
				TRX.CW_LPF_Filter = 250;
			else if (TRX.CW_LPF_Filter == 250)
				TRX.CW_LPF_Filter = 200;
			else if (TRX.CW_LPF_Filter == 200)
				TRX.CW_LPF_Filter = 150;
			else if (TRX.CW_LPF_Filter == 150)
				TRX.CW_LPF_Filter = 100;
			sprintf(str, "BW: %d",TRX.CW_LPF_Filter);
		break;
		
		case TRX_MODE_NFM:
		case TRX_MODE_WFM:
			if (TRX.RX_FM_LPF_Filter == 6000)
				TRX.RX_FM_LPF_Filter = 5000;
			else if (TRX.RX_FM_LPF_Filter == 7000)
				TRX.RX_FM_LPF_Filter = 6000;
			else if (TRX.RX_FM_LPF_Filter == 8000)
				TRX.RX_FM_LPF_Filter = 7000;
			else if (TRX.RX_FM_LPF_Filter == 9000)
				TRX.RX_FM_LPF_Filter = 8000;
			else if (TRX.RX_FM_LPF_Filter == 10000)
				TRX.RX_FM_LPF_Filter = 9000;
			else if (TRX.RX_FM_LPF_Filter == 15000)
				TRX.RX_FM_LPF_Filter = 10000;
			else if (TRX.RX_FM_LPF_Filter == 20000)
				TRX.RX_FM_LPF_Filter = 15000;
			sprintf(str, "BW: %d",TRX.RX_FM_LPF_Filter);
		break;
		
		case TRX_MODE_AM:
			if (TRX.RX_AM_LPF_Filter == 2300)
				TRX.RX_AM_LPF_Filter = 2100;
			else if (TRX.RX_AM_LPF_Filter == 2500)
				TRX.RX_AM_LPF_Filter = 2300;
			else if (TRX.RX_AM_LPF_Filter == 2700)
				TRX.RX_AM_LPF_Filter = 2500;
			else if (TRX.RX_AM_LPF_Filter == 2900)
				TRX.RX_AM_LPF_Filter = 2700;
			else if (TRX.RX_AM_LPF_Filter == 3000)
				TRX.RX_AM_LPF_Filter = 2900;
			else if (TRX.RX_AM_LPF_Filter == 3200)
				TRX.RX_AM_LPF_Filter = 3000;
			else if (TRX.RX_AM_LPF_Filter == 3400)
				TRX.RX_AM_LPF_Filter = 3200;
			else if (TRX.RX_AM_LPF_Filter == 3600)
				TRX.RX_AM_LPF_Filter = 3400;
			else if (TRX.RX_AM_LPF_Filter == 3800)
				TRX.RX_AM_LPF_Filter = 3400;
			else if (TRX.RX_AM_LPF_Filter == 4000)
				TRX.RX_AM_LPF_Filter = 3800;
			else if (TRX.RX_AM_LPF_Filter == 4500)
				TRX.RX_AM_LPF_Filter = 3800;
			else if (TRX.RX_AM_LPF_Filter == 5000)
				TRX.RX_AM_LPF_Filter = 4500;
			else if (TRX.RX_AM_LPF_Filter == 6000)
				TRX.RX_AM_LPF_Filter = 5000;
			else if (TRX.RX_AM_LPF_Filter == 7000)
				TRX.RX_AM_LPF_Filter = 6000;
			else if (TRX.RX_AM_LPF_Filter == 8000)
				TRX.RX_AM_LPF_Filter = 7000;
			else if (TRX.RX_AM_LPF_Filter == 9000)
				TRX.RX_AM_LPF_Filter = 8000;
			else if (TRX.RX_AM_LPF_Filter == 10000)
				TRX.RX_AM_LPF_Filter = 9000;
			sprintf(str, "BW: %d",TRX.RX_AM_LPF_Filter);
		break;
	}
	
	TRX_setMode(SecondaryVFO()->Mode, SecondaryVFO());
	TRX_setMode(CurrentVFO()->Mode, CurrentVFO());
	LCD_showTooltip(str);
	NeedSaveSettings = true;
}
