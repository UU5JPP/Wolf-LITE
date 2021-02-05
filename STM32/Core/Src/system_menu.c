#include "system_menu.h"
#include "lcd.h"
#include "settings.h"
#include "audio_filters.h"
#include "bootloader.h"
#include "functions.h"
#include "swr_analyzer.h"
#include "fonts.h"
#include "agc.h"
#include "screen_layout.h"
#include "bands.h"

static void SYSMENU_HANDL_TRX_RFPower(int8_t direction);
static void SYSMENU_HANDL_TRX_BandMap(int8_t direction);
static void SYSMENU_HANDL_TRX_AutoGain(int8_t direction);
static void SYSMENU_HANDL_TRX_TWO_SIGNAL_TUNE(int8_t direction);
static void SYSMENU_HANDL_TRX_RFFilters(int8_t direction);
static void SYSMENU_HANDL_TRX_MICIN(int8_t direction);
static void SYSMENU_HANDL_TRX_LINEIN(int8_t direction);
static void SYSMENU_HANDL_TRX_USBIN(int8_t direction);
static void SYSMENU_HANDL_TRX_SHIFT_INTERVAL(int8_t direction);
static void SYSMENU_HANDL_TRX_FRQ_STEP(int8_t direction);
static void SYSMENU_HANDL_TRX_FRQ_FAST_STEP(int8_t direction);
static void SYSMENU_HANDL_TRX_FRQ_ENC_STEP(int8_t direction);
static void SYSMENU_HANDL_TRX_FRQ_ENC_FAST_STEP(int8_t direction);
static void SYSMENU_HANDL_TRX_ENC_ACCELERATE(int8_t direction);
static void SYSMENU_HANDL_TRX_ATT_STEP(int8_t direction);
static void SYSMENU_HANDL_TRX_DEBUG_CONSOLE(int8_t direction);
static void SYSMENU_HANDL_TRX_SetCallsign(int8_t direction);
static void SYSMENU_HANDL_TRX_TRANSV_ENABLE(int8_t direction);
static void SYSMENU_HANDL_TRX_TRANSV_OFFSET(int8_t direction);

static void SYSMENU_HANDL_AUDIO_VOLUME(int8_t direction);
static void SYSMENU_HANDL_AUDIO_IFGain(int8_t direction);
static void SYSMENU_HANDL_AUDIO_AGC_GAIN_TARGET(int8_t direction);
static void SYSMENU_HANDL_AUDIO_MIC_Gain(int8_t direction);
static void SYSMENU_HANDL_AUDIO_DNR_THRES(int8_t direction);
static void SYSMENU_HANDL_AUDIO_DNR_AVERAGE(int8_t direction);
static void SYSMENU_HANDL_AUDIO_DNR_MINMAL(int8_t direction);
static void SYSMENU_HANDL_AUDIO_SSB_HPF_pass(int8_t direction);
static void SYSMENU_HANDL_AUDIO_SSB_RX_LPF_pass(int8_t direction);
static void SYSMENU_HANDL_AUDIO_SSB_TX_LPF_pass(int8_t direction);
static void SYSMENU_HANDL_AUDIO_CW_LPF_pass(int8_t direction);
static void SYSMENU_HANDL_AUDIO_CW_HPF_pass(int8_t direction);
static void SYSMENU_HANDL_AUDIO_AM_RX_LPF_pass(int8_t direction);
static void SYSMENU_HANDL_AUDIO_AM_TX_LPF_pass(int8_t direction);
static void SYSMENU_HANDL_AUDIO_FM_RX_LPF_pass(int8_t direction);
static void SYSMENU_HANDL_AUDIO_FM_TX_LPF_pass(int8_t direction);
static void SYSMENU_HANDL_AUDIO_MIC_EQ_LOW(int8_t direction);
static void SYSMENU_HANDL_AUDIO_MIC_EQ_MID(int8_t direction);
static void SYSMENU_HANDL_AUDIO_MIC_EQ_HIG(int8_t direction);
static void SYSMENU_HANDL_AUDIO_RX_EQ_LOW(int8_t direction);
static void SYSMENU_HANDL_AUDIO_RX_EQ_MID(int8_t direction);
static void SYSMENU_HANDL_AUDIO_RX_EQ_HIG(int8_t direction);
static void SYSMENU_HANDL_AUDIO_RX_AGC_SSB_Speed(int8_t direction);
static void SYSMENU_HANDL_AUDIO_RX_AGC_CW_Speed(int8_t direction);
static void SYSMENU_HANDL_AUDIO_TX_AGCSpeed(int8_t direction);
static void SYSMENU_HANDL_AUDIO_FMSquelch(int8_t direction);
static void SYSMENU_HANDL_AUDIO_VAD_Squelch(int8_t direction);
static void SYSMENU_HANDL_AUDIO_Beeper(int8_t direction);

static void SYSMENU_HANDL_CW_GENERATOR_SHIFT_HZ(int8_t direction);
static void SYSMENU_HANDL_CW_SelfHear(int8_t direction);
static void SYSMENU_HANDL_CW_Keyer(int8_t direction);
static void SYSMENU_HANDL_CW_Keyer_WPM(int8_t direction);
static void SYSMENU_HANDL_CW_Key_timeout(int8_t direction);
static void SYSMENU_HANDL_CW_GaussFilter(int8_t direction);

static void SYSMENU_HANDL_SCREEN_FFT_Enabled(int8_t direction);
static void SYSMENU_HANDL_SCREEN_COLOR_THEME(int8_t direction);
static void SYSMENU_HANDL_SCREEN_FFT_Averaging(int8_t direction);
static void SYSMENU_HANDL_SCREEN_FFT_Window(int8_t direction);
static void SYSMENU_HANDL_SCREEN_FFT_Zoom(int8_t direction);
static void SYSMENU_HANDL_SCREEN_FFT_Color(int8_t direction);
static void SYSMENU_HANDL_SCREEN_FFT_Grid(int8_t direction);
static void SYSMENU_HANDL_SCREEN_FFT_Background(int8_t direction);
static void SYSMENU_HANDL_SCREEN_FFT_Compressor(int8_t direction);
static void SYSMENU_HANDL_SCREEN_LCD_position(int8_t direction);//LCD povorot

static void SYSMENU_HANDL_ADC_PGA(int8_t direction);
static void SYSMENU_HANDL_ADC_RAND(int8_t direction);
static void SYSMENU_HANDL_ADC_SHDN(int8_t direction);
static void SYSMENU_HANDL_ADC_DITH(int8_t direction);
static void SYSMENU_HANDL_ADC_DRIVER(int8_t direction);

static void SYSMENU_HANDL_SETTIME(int8_t direction);
static void SYSMENU_HANDL_Bootloader(int8_t direction);

static void SYSMENU_HANDL_CALIB_ENCODER_SLOW_RATE(int8_t direction);
static void SYSMENU_HANDL_CALIB_ENCODER_INVERT(int8_t direction);
static void SYSMENU_HANDL_CALIB_ENCODER2_INVERT(int8_t direction);
static void SYSMENU_HANDL_CALIB_ENCODER_DEBOUNCE(int8_t direction);
static void SYSMENU_HANDL_CALIB_ENCODER2_DEBOUNCE(int8_t direction);
static void SYSMENU_HANDL_CALIB_ENCODER_ON_FALLING(int8_t direction);
static void SYSMENU_HANDL_CALIB_CIC_SHIFT(int8_t direction);
static void SYSMENU_HANDL_CALIB_CICCOMP_SHIFT(int8_t direction);
static void SYSMENU_HANDL_CALIB_TXCICCOMP_SHIFT(int8_t direction);
static void SYSMENU_HANDL_CALIB_DAC_SHIFT(int8_t direction);
static void SYSMENU_HANDL_CALIB_RF_GAIN_LF(int8_t direction);
static void SYSMENU_HANDL_CALIB_RF_GAIN_HF_LOW(int8_t direction);
static void SYSMENU_HANDL_CALIB_RF_GAIN_HF(int8_t direction);
static void SYSMENU_HANDL_CALIB_RF_GAIN_HF_HIGH(int8_t direction);
static void SYSMENU_HANDL_CALIB_S_METER(int8_t direction);
static void SYSMENU_HANDL_CALIB_VOLT(int8_t direction);
static void SYSMENU_HANDL_CALIB_ADC_OFFSET(int8_t direction);
static void SYSMENU_HANDL_CALIB_SWR_TRANS_RATE(int8_t direction);

static void SYSMENU_HANDL_TRXMENU(int8_t direction);
static void SYSMENU_HANDL_AUDIOMENU(int8_t direction);
static void SYSMENU_HANDL_CWMENU(int8_t direction);
static void SYSMENU_HANDL_LCDMENU(int8_t direction);
static void SYSMENU_HANDL_ADCMENU(int8_t direction);
static void SYSMENU_HANDL_CALIBRATIONMENU(int8_t direction);
static void SYSMENU_HANDL_SERVICESMENU(int8_t direction);

static void SYSMENU_HANDL_SWR_BAND_START(int8_t direction);
static void SYSMENU_HANDL_SWR_HF_START(int8_t direction);

static const struct sysmenu_item_handler sysmenu_handlers[] =
	{
		{"TRX Settings", SYSMENU_MENU, 0, SYSMENU_HANDL_TRXMENU},
		{"AUDIO Settings", SYSMENU_MENU, 0, SYSMENU_HANDL_AUDIOMENU},
		{"CW Settings", SYSMENU_MENU, 0, SYSMENU_HANDL_CWMENU},
		{"SCREEN Settings", SYSMENU_MENU, 0, SYSMENU_HANDL_LCDMENU},
		{"ADC/DAC Settings", SYSMENU_MENU, 0, SYSMENU_HANDL_ADCMENU},
		{"Services", SYSMENU_MENU, 0, SYSMENU_HANDL_SERVICESMENU},
		{"Set Clock Time", SYSMENU_RUN, 0, SYSMENU_HANDL_SETTIME},
		{"Flash update", SYSMENU_RUN, 0, SYSMENU_HANDL_Bootloader},
		{"Calibration", SYSMENU_HIDDEN_MENU, 0, SYSMENU_HANDL_CALIBRATIONMENU},
};
static const uint8_t sysmenu_item_count = sizeof(sysmenu_handlers) / sizeof(sysmenu_handlers[0]);

static const struct sysmenu_item_handler sysmenu_trx_handlers[] =
	{
		{"RF Power", SYSMENU_UINT8, (uint32_t *)&TRX.RF_Power, SYSMENU_HANDL_TRX_RFPower},
		{"LCD position", SYSMENU_UINT8, (uint32_t *)&TRX.LCD_position, SYSMENU_HANDL_SCREEN_LCD_position},// LCD povorot
		{"Band Map", SYSMENU_BOOLEAN, (uint32_t *)&TRX.BandMapEnabled, SYSMENU_HANDL_TRX_BandMap},
		{"AutoGainer", SYSMENU_BOOLEAN, (uint32_t *)&TRX.AutoGain, SYSMENU_HANDL_TRX_AutoGain},
		{"Two Signal TUNE", SYSMENU_BOOLEAN, (uint32_t *)&TRX.TWO_SIGNAL_TUNE, SYSMENU_HANDL_TRX_TWO_SIGNAL_TUNE},
		{"Shift Interval", SYSMENU_UINT16, (uint32_t *)&TRX.SHIFT_INTERVAL, SYSMENU_HANDL_TRX_SHIFT_INTERVAL},
		{"Freq Step", SYSMENU_UINT16, (uint32_t *)&TRX.FRQ_STEP, SYSMENU_HANDL_TRX_FRQ_STEP},
		{"Freq Step FAST", SYSMENU_UINT16, (uint32_t *)&TRX.FRQ_FAST_STEP, SYSMENU_HANDL_TRX_FRQ_FAST_STEP},
		{"Encoder Accelerate", SYSMENU_BOOLEAN, (uint32_t *)&TRX.Encoder_Accelerate, SYSMENU_HANDL_TRX_ENC_ACCELERATE},
		{"Att step, dB", SYSMENU_UINT8, (uint32_t *)&TRX.ATT_STEP, SYSMENU_HANDL_TRX_ATT_STEP},
		{"DEBUG Console", SYSMENU_BOOLEAN, (uint32_t *)&TRX.Debug_Console, SYSMENU_HANDL_TRX_DEBUG_CONSOLE},
		{"MIC IN", SYSMENU_BOOLEAN, (uint32_t *)&TRX.InputType_MIC, SYSMENU_HANDL_TRX_MICIN},
		{"LINE IN", SYSMENU_BOOLEAN, (uint32_t *)&TRX.InputType_LINE, SYSMENU_HANDL_TRX_LINEIN},
		{"USB IN", SYSMENU_BOOLEAN, (uint32_t *)&TRX.InputType_USB, SYSMENU_HANDL_TRX_USBIN},
		{"Callsign", SYSMENU_RUN, 0, SYSMENU_HANDL_TRX_SetCallsign},
		{"Transverter Enable", SYSMENU_BOOLEAN, (uint32_t *)&TRX.Transverter_Enabled, SYSMENU_HANDL_TRX_TRANSV_ENABLE},
		{"Transverter Offset, mHz", SYSMENU_UINT16, (uint32_t *)&TRX.Transverter_Offset_Mhz, SYSMENU_HANDL_TRX_TRANSV_OFFSET},
};
static const uint8_t sysmenu_trx_item_count = sizeof(sysmenu_trx_handlers) / sizeof(sysmenu_trx_handlers[0]);

static const struct sysmenu_item_handler sysmenu_audio_handlers[] =
	{
		{"Volume", SYSMENU_UINT8, (uint32_t *)&TRX.Volume, SYSMENU_HANDL_AUDIO_VOLUME},
		{"IF Gain, dB", SYSMENU_UINT8, (uint32_t *)&TRX.IF_Gain, SYSMENU_HANDL_AUDIO_IFGain},
		{"AGC Gain target, LKFS", SYSMENU_INT8, (uint32_t *)&TRX.AGC_GAIN_TARGET, SYSMENU_HANDL_AUDIO_AGC_GAIN_TARGET},
		{"Mic Gain", SYSMENU_UINT8, (uint32_t *)&TRX.MIC_GAIN, SYSMENU_HANDL_AUDIO_MIC_Gain},
		{"DNR Threshold", SYSMENU_UINT8, (uint32_t *)&TRX.DNR_SNR_THRESHOLD, SYSMENU_HANDL_AUDIO_DNR_THRES},
		{"DNR Average", SYSMENU_UINT8, (uint32_t *)&TRX.DNR_AVERAGE, SYSMENU_HANDL_AUDIO_DNR_AVERAGE},
		{"DNR Minimal", SYSMENU_UINT8, (uint32_t *)&TRX.DNR_MINIMAL, SYSMENU_HANDL_AUDIO_DNR_MINMAL},
		{"SSB HPF Pass", SYSMENU_UINT16, (uint32_t *)&TRX.SSB_HPF_Filter, SYSMENU_HANDL_AUDIO_SSB_HPF_pass},
		{"SSB RX LPF Pass", SYSMENU_UINT16, (uint32_t *)&TRX.RX_SSB_LPF_Filter, SYSMENU_HANDL_AUDIO_SSB_RX_LPF_pass},
		{"SSB TX LPF Pass", SYSMENU_UINT16, (uint32_t *)&TRX.TX_SSB_LPF_Filter, SYSMENU_HANDL_AUDIO_SSB_TX_LPF_pass},
		{"CW HPF Pass", SYSMENU_UINT16, (uint32_t *)&TRX.CW_HPF_Filter, SYSMENU_HANDL_AUDIO_CW_HPF_pass},
		{"CW LPF Pass", SYSMENU_UINT16, (uint32_t *)&TRX.CW_LPF_Filter, SYSMENU_HANDL_AUDIO_CW_LPF_pass},
		{"AM RX LPF Pass", SYSMENU_UINT16, (uint32_t *)&TRX.RX_AM_LPF_Filter, SYSMENU_HANDL_AUDIO_AM_RX_LPF_pass},
		{"AM TX LPF Pass", SYSMENU_UINT16, (uint32_t *)&TRX.TX_AM_LPF_Filter, SYSMENU_HANDL_AUDIO_AM_TX_LPF_pass},
		{"FM RX LPF Pass", SYSMENU_UINT16, (uint32_t *)&TRX.RX_FM_LPF_Filter, SYSMENU_HANDL_AUDIO_FM_RX_LPF_pass},
		{"FM TX LPF Pass", SYSMENU_UINT16, (uint32_t *)&TRX.TX_FM_LPF_Filter, SYSMENU_HANDL_AUDIO_FM_TX_LPF_pass},
		{"FM Squelch", SYSMENU_UINT8, (uint32_t *)&TRX.FM_SQL_threshold, SYSMENU_HANDL_AUDIO_FMSquelch},
		{"MIC EQ Low", SYSMENU_INT8, (uint32_t *)&TRX.MIC_EQ_LOW, SYSMENU_HANDL_AUDIO_MIC_EQ_LOW},
		{"MIC EQ Mid", SYSMENU_INT8, (uint32_t *)&TRX.MIC_EQ_MID, SYSMENU_HANDL_AUDIO_MIC_EQ_MID},
		{"MIC EQ High", SYSMENU_INT8, (uint32_t *)&TRX.MIC_EQ_HIG, SYSMENU_HANDL_AUDIO_MIC_EQ_HIG},
		{"RX EQ Low", SYSMENU_INT8, (uint32_t *)&TRX.RX_EQ_LOW, SYSMENU_HANDL_AUDIO_RX_EQ_LOW},
		{"RX EQ Mid", SYSMENU_INT8, (uint32_t *)&TRX.RX_EQ_MID, SYSMENU_HANDL_AUDIO_RX_EQ_MID},
		{"RX EQ High", SYSMENU_INT8, (uint32_t *)&TRX.RX_EQ_HIG, SYSMENU_HANDL_AUDIO_RX_EQ_HIG},
		{"RX AGC SSB Speed", SYSMENU_UINT8, (uint32_t *)&TRX.RX_AGC_SSB_speed, SYSMENU_HANDL_AUDIO_RX_AGC_SSB_Speed},
		{"RX AGC CW Speed", SYSMENU_UINT8, (uint32_t *)&TRX.RX_AGC_CW_speed, SYSMENU_HANDL_AUDIO_RX_AGC_CW_Speed},
		{"TX AGC Speed", SYSMENU_UINT8, (uint32_t *)&TRX.TX_AGC_speed, SYSMENU_HANDL_AUDIO_TX_AGCSpeed},
		{"Beeper", SYSMENU_UINT8, (uint32_t *)&TRX.Beeper, SYSMENU_HANDL_AUDIO_Beeper},
};
static const uint8_t sysmenu_audio_item_count = sizeof(sysmenu_audio_handlers) / sizeof(sysmenu_audio_handlers[0]);

static const struct sysmenu_item_handler sysmenu_cw_handlers[] =
	{
		{"CW Key timeout", SYSMENU_UINT16, (uint32_t *)&TRX.CW_Key_timeout, SYSMENU_HANDL_CW_Key_timeout},
		{"CW Generator shift", SYSMENU_UINT16, (uint32_t *)&TRX.CW_GENERATOR_SHIFT_HZ, SYSMENU_HANDL_CW_GENERATOR_SHIFT_HZ},
		{"CW Self Hear", SYSMENU_BOOLEAN, (uint32_t *)&TRX.CW_SelfHear, SYSMENU_HANDL_CW_SelfHear},
		{"CW Keyer", SYSMENU_BOOLEAN, (uint32_t *)&TRX.CW_KEYER, SYSMENU_HANDL_CW_Keyer},
		{"CW Keyer WPM", SYSMENU_UINT8, (uint32_t *)&TRX.CW_KEYER_WPM, SYSMENU_HANDL_CW_Keyer_WPM},
		{"CW Gauss filter", SYSMENU_BOOLEAN, (uint32_t *)&TRX.CW_GaussFilter, SYSMENU_HANDL_CW_GaussFilter},
};
static const uint8_t sysmenu_cw_item_count = sizeof(sysmenu_cw_handlers) / sizeof(sysmenu_cw_handlers[0]);

static const struct sysmenu_item_handler sysmenu_screen_handlers[] =
	{
		{"FFT Zoom", SYSMENU_UINT8, (uint32_t *)&TRX.FFT_Zoom, SYSMENU_HANDL_SCREEN_FFT_Zoom},
		{"Color Theme", SYSMENU_UINT8, (uint32_t *)&TRX.ColorThemeId, SYSMENU_HANDL_SCREEN_COLOR_THEME},
		{"FFT Color", SYSMENU_UINT8, (uint32_t *)&TRX.FFT_Color, SYSMENU_HANDL_SCREEN_FFT_Color},
		{"FFT Grid", SYSMENU_UINT8, (uint32_t *)&TRX.FFT_Grid, SYSMENU_HANDL_SCREEN_FFT_Grid},
		{"FFT Background", SYSMENU_BOOLEAN, (uint32_t *)&TRX.FFT_Background, SYSMENU_HANDL_SCREEN_FFT_Background},
		{"FFT Enabled", SYSMENU_BOOLEAN, (uint32_t *)&TRX.FFT_Enabled, SYSMENU_HANDL_SCREEN_FFT_Enabled},
		{"FFT Compressor", SYSMENU_BOOLEAN, (uint32_t *)&TRX.FFT_Compressor, SYSMENU_HANDL_SCREEN_FFT_Compressor},
		{"FFT Averaging", SYSMENU_UINT8, (uint32_t *)&TRX.FFT_Averaging, SYSMENU_HANDL_SCREEN_FFT_Averaging},
		{"FFT Window", SYSMENU_UINT8, (uint32_t *)&TRX.FFT_Window, SYSMENU_HANDL_SCREEN_FFT_Window},
};
static const uint8_t sysmenu_screen_item_count = sizeof(sysmenu_screen_handlers) / sizeof(sysmenu_screen_handlers[0]);

static const struct sysmenu_item_handler sysmenu_adc_handlers[] =
	{
		{"ADC Driver", SYSMENU_BOOLEAN, (uint32_t *)&TRX.ADC_Driver, SYSMENU_HANDL_ADC_DRIVER},
		{"ADC Shutdown", SYSMENU_BOOLEAN, (uint32_t *)&TRX.ADC_SHDN, SYSMENU_HANDL_ADC_SHDN},
};
static const uint8_t sysmenu_adc_item_count = sizeof(sysmenu_adc_handlers) / sizeof(sysmenu_adc_handlers[0]);

static const struct sysmenu_item_handler sysmenu_calibration_handlers[] =
	{
		{"Encoder invert", SYSMENU_BOOLEAN, (uint32_t *)&CALIBRATE.ENCODER_INVERT, SYSMENU_HANDL_CALIB_ENCODER_INVERT},
		{"Encoder2 invert", SYSMENU_BOOLEAN, (uint32_t *)&CALIBRATE.ENCODER2_INVERT, SYSMENU_HANDL_CALIB_ENCODER2_INVERT},
		{"Encoder debounce", SYSMENU_UINT8, (uint32_t *)&CALIBRATE.ENCODER_DEBOUNCE, SYSMENU_HANDL_CALIB_ENCODER_DEBOUNCE},
		{"Encoder2 debounce", SYSMENU_UINT8, (uint32_t *)&CALIBRATE.ENCODER2_DEBOUNCE, SYSMENU_HANDL_CALIB_ENCODER2_DEBOUNCE},
		{"Encoder slow rate", SYSMENU_UINT8, (uint32_t *)&CALIBRATE.ENCODER_SLOW_RATE, SYSMENU_HANDL_CALIB_ENCODER_SLOW_RATE},
		{"Encoder on falling", SYSMENU_BOOLEAN, (uint32_t *)&CALIBRATE.ENCODER_ON_FALLING, SYSMENU_HANDL_CALIB_ENCODER_ON_FALLING},
		{"CIC Shift", SYSMENU_UINT8, (uint32_t *)&CALIBRATE.CIC_GAINER_val, SYSMENU_HANDL_CALIB_CIC_SHIFT},
		{"CICCOMP Shift", SYSMENU_UINT8, (uint32_t *)&CALIBRATE.CICFIR_GAINER_val, SYSMENU_HANDL_CALIB_CICCOMP_SHIFT},
		{"TX CICCOMP Shift", SYSMENU_UINT8, (uint32_t *)&CALIBRATE.TXCICFIR_GAINER_val, SYSMENU_HANDL_CALIB_TXCICCOMP_SHIFT},
		{"DAC Shift", SYSMENU_UINT8, (uint32_t *)&CALIBRATE.DAC_GAINER_val, SYSMENU_HANDL_CALIB_DAC_SHIFT},
		{"RF GAIN LF", SYSMENU_UINT8, (uint32_t *)&CALIBRATE.rf_out_power_lf, SYSMENU_HANDL_CALIB_RF_GAIN_LF},	//-V641
		{"RF GAIN HF LOW", SYSMENU_UINT8, (uint32_t *)&CALIBRATE.rf_out_power_hf_low, SYSMENU_HANDL_CALIB_RF_GAIN_HF_LOW},	//-V641
		{"RF GAIN HF", SYSMENU_UINT8, (uint32_t *)&CALIBRATE.rf_out_power_hf, SYSMENU_HANDL_CALIB_RF_GAIN_HF},	//-V641
		{"RF GAIN HF HIGH", SYSMENU_UINT8, (uint32_t *)&CALIBRATE.rf_out_power_hf_high, SYSMENU_HANDL_CALIB_RF_GAIN_HF_HIGH},	//-V641
		{"S METER", SYSMENU_INT16, (uint32_t *)&CALIBRATE.smeter_calibration, SYSMENU_HANDL_CALIB_S_METER},
		{"SWR TRANS RATE", SYSMENU_FLOAT32, (uint32_t *)&CALIBRATE.swr_trans_rate, SYSMENU_HANDL_CALIB_SWR_TRANS_RATE},
		{"VOLT CALIBR", SYSMENU_FLOAT32, (uint32_t *)&CALIBRATE.volt_cal_rate, SYSMENU_HANDL_CALIB_VOLT},
};
static const uint8_t sysmenu_calibration_item_count = sizeof(sysmenu_calibration_handlers) / sizeof(sysmenu_calibration_handlers[0]);

static const struct sysmenu_item_handler sysmenu_services_handlers[] =
{
	{"Band SWR", SYSMENU_RUN, 0, SYSMENU_HANDL_SWR_BAND_START},
	{"HF SWR", SYSMENU_RUN, 0, SYSMENU_HANDL_SWR_HF_START},
};
static const uint8_t sysmenu_services_item_count = sizeof(sysmenu_services_handlers) / sizeof(sysmenu_services_handlers[0]);

//COMMON MENU
static void drawSystemMenuElement(char *title, SystemMenuType type, uint32_t *value, bool onlyVal);
static void SYSMENU_WIFI_DrawSelectAPMenu(bool full_redraw);
static void SYSMENU_WIFI_SelectAPMenuMove(int8_t dir);
static void SYSMENU_WIFI_DrawAPpasswordMenu(bool full_redraw);
static void SYSMENU_WIFI_RotatePasswordChar(int8_t dir);
static void SYSMENU_TRX_DrawCallsignMenu(bool full_redraw);
static void SYSMENU_TRX_RotateCallsignChar(int8_t dir);

static struct sysmenu_item_handler *sysmenu_handlers_selected = (struct sysmenu_item_handler*)&sysmenu_handlers[0];
static uint8_t *sysmenu_item_count_selected = (uint8_t*)&sysmenu_item_count;
static uint8_t systemMenuIndex = 0;
static uint8_t systemMenuRootIndex = 0;
static uint16_t sysmenu_y = 5;
static uint8_t sysmenu_i = 0;
static bool sysmenu_onroot = true;
bool sysmenu_hiddenmenu_enabled = false;
static bool sysmenu_services_opened = false;
static bool sysmenu_infowindow_opened = false;
static bool sysmenu_item_selected_by_enc2 = false;
static bool sysmenu_trx_setCallsign_menu_opened = false;
static uint8_t sysmenu_trx_selected_callsign_char_index = 0;

//Time menu
static bool sysmenu_timeMenuOpened = false;
static uint8_t TimeMenuSelection = 0;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

//TRX MENU

static void SYSMENU_HANDL_TRXMENU(int8_t direction)
{
	sysmenu_handlers_selected = (struct sysmenu_item_handler*)&sysmenu_trx_handlers[0];
	sysmenu_item_count_selected = (uint8_t*)&sysmenu_trx_item_count;
	sysmenu_onroot = false;
	systemMenuIndex = 0;
	LCD_UpdateQuery.SystemMenuRedraw = true;
}

void SYSMENU_TRX_RFPOWER_HOTKEY(void)
{
	sysmenu_handlers_selected = (struct sysmenu_item_handler*)&sysmenu_trx_handlers[0];
	sysmenu_item_count_selected = (uint8_t*)&sysmenu_trx_item_count;
	sysmenu_onroot = false;
	systemMenuIndex = 0;
	LCD_redraw(false);
}

void SYSMENU_TRX_STEP_HOTKEY(void)
{
	sysmenu_handlers_selected = (struct sysmenu_item_handler*)&sysmenu_trx_handlers[0];
	sysmenu_item_count_selected = (uint8_t*)&sysmenu_trx_item_count;
	sysmenu_onroot = false;
	systemMenuIndex = 7;
	if (TRX.Fast)
		systemMenuIndex = 8;
	LCD_redraw(false);
}

static void SYSMENU_HANDL_TRX_BandMap(int8_t direction)
{
	if (direction > 0)
		TRX.BandMapEnabled = true;
	if (direction < 0)
		TRX.BandMapEnabled = false;
}

static void SYSMENU_HANDL_TRX_AutoGain(int8_t direction)
{
	if (direction > 0)
		TRX.AutoGain = true;
	if (direction < 0)
		TRX.AutoGain = false;
	FPGA_NeedSendParams = true;
}

static void SYSMENU_HANDL_TRX_TWO_SIGNAL_TUNE(int8_t direction)
{
	if (direction > 0)
		TRX.TWO_SIGNAL_TUNE = true;
	if (direction < 0)
		TRX.TWO_SIGNAL_TUNE = false;
}

static void SYSMENU_HANDL_TRX_RFPower(int8_t direction)
{
	TRX.RF_Power += direction;
	if (TRX.RF_Power < 1)
		TRX.RF_Power = 1;
	if (TRX.RF_Power > 100)
		TRX.RF_Power = 100;
}

static void SYSMENU_HANDL_TRX_MICIN(int8_t direction)
{
	if (direction > 0)
		TRX.InputType_MIC = true;
	if (direction < 0)
		TRX.InputType_MIC = false;
	TRX.InputType_LINE = false;
	TRX.InputType_USB = false;
	LCD_UpdateQuery.SystemMenu = true;
	TRX_Restart_Mode();
}

static void SYSMENU_HANDL_TRX_LINEIN(int8_t direction)
{
	if (direction > 0)
		TRX.InputType_LINE = true;
	if (direction < 0)
		TRX.InputType_LINE = false;
	TRX.InputType_MIC = false;
	TRX.InputType_USB = false;
	LCD_UpdateQuery.SystemMenu = true;
	TRX_Restart_Mode();
}

static void SYSMENU_HANDL_TRX_USBIN(int8_t direction)
{
	if (direction > 0)
		TRX.InputType_USB = true;
	if (direction < 0)
		TRX.InputType_USB = false;
	TRX.InputType_MIC = false;
	TRX.InputType_LINE = false;
	LCD_UpdateQuery.SystemMenu = true;
	TRX_Restart_Mode();
}

static void SYSMENU_HANDL_TRX_DEBUG_CONSOLE(int8_t direction)
{
	if (direction > 0)
		TRX.Debug_Console = true;
	if (direction < 0)
		TRX.Debug_Console = false;
}

static void SYSMENU_HANDL_TRX_SHIFT_INTERVAL(int8_t direction)
{
	TRX.SHIFT_INTERVAL += direction * 100;
	if (TRX.SHIFT_INTERVAL < 100)
		TRX.SHIFT_INTERVAL = 100;
	if (TRX.SHIFT_INTERVAL > 10000)
		TRX.SHIFT_INTERVAL = 10000;
}

static void SYSMENU_HANDL_TRX_FRQ_STEP(int8_t direction)
{
	const uint16_t freq_steps[] = {1, 10, 25, 50, 100};
	for (uint8_t i = 0; i < ARRLENTH(freq_steps); i++)
		if (TRX.FRQ_STEP == freq_steps[i])
		{
			if (direction < 0)
			{
				if (i > 0)
					TRX.FRQ_STEP = freq_steps[i - 1];
				else
					TRX.FRQ_STEP = freq_steps[0];
				return;
			}
			else
			{
				if (i < (ARRLENTH(freq_steps) - 1))
					TRX.FRQ_STEP = freq_steps[i + 1];
				else
					TRX.FRQ_STEP = freq_steps[ARRLENTH(freq_steps) - 1];
				return;
			}
		}
	TRX.FRQ_STEP = freq_steps[0];
}

static void SYSMENU_HANDL_TRX_FRQ_FAST_STEP(int8_t direction)
{
	const uint16_t freq_steps[] = {10, 25, 50, 100, 500, 1000};
	for (uint8_t i = 0; i < ARRLENTH(freq_steps); i++)
		if (TRX.FRQ_FAST_STEP == freq_steps[i])
		{
			if (direction < 0)
			{
				if (i > 0)
					TRX.FRQ_FAST_STEP = freq_steps[i - 1];
				else
					TRX.FRQ_FAST_STEP = freq_steps[0];
				return;
			}
			else
			{
				if (i < (ARRLENTH(freq_steps) - 1))
					TRX.FRQ_FAST_STEP = freq_steps[i + 1];
				else
					TRX.FRQ_FAST_STEP = freq_steps[ARRLENTH(freq_steps) - 1];
				return;
			}
		}
	TRX.FRQ_FAST_STEP = freq_steps[0];
}

static void SYSMENU_HANDL_TRX_ENC_ACCELERATE(int8_t direction)
{
	if (direction > 0)
		TRX.Encoder_Accelerate = true;
	if (direction < 0)
		TRX.Encoder_Accelerate = false;
}

static void SYSMENU_HANDL_TRX_ATT_STEP(int8_t direction)
{
	TRX.ATT_STEP += direction;
	if (TRX.ATT_STEP < 1)
		TRX.ATT_STEP = 1;
	if (TRX.ATT_STEP > 15)
		TRX.ATT_STEP = 15;
}

static void SYSMENU_TRX_DrawCallsignMenu(bool full_redraw)
{
	if (full_redraw)
	{
		LCDDriver_Fill(BG_COLOR);
		LCDDriver_printText("CALLSIGN:", 5, 5, FG_COLOR, BG_COLOR, 2);
	}

	LCDDriver_printText(TRX.CALLSIGN, 10, 37, COLOR_GREEN, BG_COLOR, 2);
	LCDDriver_drawFastHLine(8 + sysmenu_trx_selected_callsign_char_index * 12, 54, 12, COLOR_RED);
}

static void SYSMENU_TRX_RotateCallsignChar(int8_t dir)
{
	bool full_redraw = false;
	if (TRX.CALLSIGN[sysmenu_trx_selected_callsign_char_index] == 0)
		full_redraw = true;
	TRX.CALLSIGN[sysmenu_trx_selected_callsign_char_index] += dir;

	// do not show special characters
	if (TRX.CALLSIGN[sysmenu_trx_selected_callsign_char_index] >= 1 && TRX.CALLSIGN[sysmenu_trx_selected_callsign_char_index] <= 32 && dir > 0)
		TRX.CALLSIGN[sysmenu_trx_selected_callsign_char_index] = 33;
	if (TRX.CALLSIGN[sysmenu_trx_selected_callsign_char_index] >= 1 && TRX.CALLSIGN[sysmenu_trx_selected_callsign_char_index] <= 32 && dir < 0)
		TRX.CALLSIGN[sysmenu_trx_selected_callsign_char_index] = 0;
	if (TRX.CALLSIGN[sysmenu_trx_selected_callsign_char_index] >= 127)
		TRX.CALLSIGN[sysmenu_trx_selected_callsign_char_index] = 0;
	if (TRX.CALLSIGN[sysmenu_trx_selected_callsign_char_index] == 0)
		full_redraw = true;

	if (full_redraw)
		LCD_UpdateQuery.SystemMenuRedraw = true;
	else
		LCD_UpdateQuery.SystemMenu = true;
}

static void SYSMENU_HANDL_TRX_SetCallsign(int8_t direction)
{
	sysmenu_trx_setCallsign_menu_opened = true;
	SYSMENU_TRX_DrawCallsignMenu(true);
	LCD_UpdateQuery.SystemMenuRedraw = true;
}

static void SYSMENU_HANDL_TRX_TRANSV_ENABLE(int8_t direction)
{
	if (direction > 0)
		TRX.Transverter_Enabled = true;
	if (direction < 0)
		TRX.Transverter_Enabled = false;
}

static void SYSMENU_HANDL_TRX_TRANSV_OFFSET(int8_t direction)
{
	TRX.Transverter_Offset_Mhz += direction;
	if (TRX.Transverter_Offset_Mhz < 1)
		TRX.Transverter_Offset_Mhz = 1;
	if (TRX.Transverter_Offset_Mhz > 500)
		TRX.Transverter_Offset_Mhz = 500;
}

//AUDIO MENU

static void SYSMENU_HANDL_AUDIOMENU(int8_t direction)
{
	sysmenu_handlers_selected = (struct sysmenu_item_handler*)&sysmenu_audio_handlers[0];
	sysmenu_item_count_selected = (uint8_t*)&sysmenu_audio_item_count;
	sysmenu_onroot = false;
	systemMenuIndex = 0;
	LCD_UpdateQuery.SystemMenuRedraw = true;
}

void SYSMENU_AUDIO_VOLUME_HOTKEY(void)
{
	sysmenu_handlers_selected = (struct sysmenu_item_handler*)&sysmenu_audio_handlers[0];
	sysmenu_item_count_selected = (uint8_t*)&sysmenu_audio_item_count;
	sysmenu_onroot = false;
	systemMenuIndex = 0;
	LCD_redraw(false);
}

void SYSMENU_AUDIO_BW_SSB_HOTKEY(void)
{
	sysmenu_handlers_selected = (struct sysmenu_item_handler*)&sysmenu_audio_handlers[0];
	sysmenu_item_count_selected = (uint8_t*)&sysmenu_audio_item_count;
	sysmenu_onroot = false;
	systemMenuIndex = 8;
	LCD_redraw(false);
}

void SYSMENU_AUDIO_BW_CW_HOTKEY(void)
{
	sysmenu_handlers_selected = (struct sysmenu_item_handler*)&sysmenu_audio_handlers[0];
	sysmenu_item_count_selected = (uint8_t*)&sysmenu_audio_item_count;
	sysmenu_onroot = false;
	systemMenuIndex = 10;
	LCD_redraw(false);
}

void SYSMENU_AUDIO_BW_AM_HOTKEY(void)
{
	sysmenu_handlers_selected = (struct sysmenu_item_handler*)&sysmenu_audio_handlers[0];
	sysmenu_item_count_selected = (uint8_t*)&sysmenu_audio_item_count;
	sysmenu_onroot = false;
	systemMenuIndex = 11;
	LCD_redraw(false);
}

void SYSMENU_AUDIO_BW_FM_HOTKEY(void)
{
	sysmenu_handlers_selected = (struct sysmenu_item_handler*)&sysmenu_audio_handlers[0];
	sysmenu_item_count_selected = (uint8_t*)&sysmenu_audio_item_count;
	sysmenu_onroot = false;
	systemMenuIndex = 12;
	LCD_redraw(false);
}

void SYSMENU_AUDIO_HPF_SSB_HOTKEY(void)
{
	sysmenu_handlers_selected = (struct sysmenu_item_handler*)&sysmenu_audio_handlers[0];
	sysmenu_item_count_selected = (uint8_t*)&sysmenu_audio_item_count;
	sysmenu_onroot = false;
	systemMenuIndex = 7;
	LCD_redraw(false);
}

void SYSMENU_AUDIO_HPF_CW_HOTKEY(void)
{
	sysmenu_handlers_selected = (struct sysmenu_item_handler*)&sysmenu_audio_handlers[0];
	sysmenu_item_count_selected = (uint8_t*)&sysmenu_audio_item_count;
	systemMenuIndex = 9;
	LCD_redraw(false);
}

void SYSMENU_AUDIO_SQUELCH_HOTKEY(void)
{
	sysmenu_handlers_selected = (struct sysmenu_item_handler*)&sysmenu_audio_handlers[0];
	sysmenu_item_count_selected = (uint8_t*)&sysmenu_audio_item_count;
	sysmenu_onroot = false;
	systemMenuIndex = 13;
	LCD_redraw(false);
}

void SYSMENU_AUDIO_AGC_HOTKEY(void)
{
	sysmenu_handlers_selected = (struct sysmenu_item_handler*)&sysmenu_audio_handlers[0];
	sysmenu_item_count_selected = (uint8_t*)&sysmenu_audio_item_count;
	sysmenu_onroot = false;
	systemMenuIndex = 21;
	LCD_redraw(false);
}

static void SYSMENU_HANDL_AUDIO_IFGain(int8_t direction)
{
	TRX.IF_Gain += direction;
	if (TRX.IF_Gain < 1)
		TRX.IF_Gain = 1;
	if (TRX.IF_Gain > 80)
		TRX.IF_Gain = 80;
}

static void SYSMENU_HANDL_AUDIO_VOLUME(int8_t direction)
{
	TRX.Volume += direction;
	if (TRX.Volume < 1)
		TRX.Volume = 1;
	if (TRX.Volume > 100)
		TRX.Volume = 100;
}

static void SYSMENU_HANDL_AUDIO_AGC_GAIN_TARGET(int8_t direction)
{
	TRX.AGC_GAIN_TARGET += direction;
	if (TRX.AGC_GAIN_TARGET < -80)
		TRX.AGC_GAIN_TARGET = -80;
	if (TRX.AGC_GAIN_TARGET > -10)
		TRX.AGC_GAIN_TARGET = -10;
}

static void SYSMENU_HANDL_AUDIO_MIC_Gain(int8_t direction)
{
	TRX.MIC_GAIN += direction;
	if (TRX.MIC_GAIN < 1)
		TRX.MIC_GAIN = 1;
	if (TRX.MIC_GAIN > 20)
		TRX.MIC_GAIN = 20;
}

static void SYSMENU_HANDL_AUDIO_DNR_THRES(int8_t direction)
{
	TRX.DNR_SNR_THRESHOLD += direction;
	if (TRX.DNR_SNR_THRESHOLD < 1)
		TRX.DNR_SNR_THRESHOLD = 1;
	if (TRX.DNR_SNR_THRESHOLD > 100)
		TRX.DNR_SNR_THRESHOLD = 100;
}

static void SYSMENU_HANDL_AUDIO_DNR_AVERAGE(int8_t direction)
{
	TRX.DNR_AVERAGE += direction;
	if (TRX.DNR_AVERAGE < 1)
		TRX.DNR_AVERAGE = 1;
	if (TRX.DNR_AVERAGE > 100)
		TRX.DNR_AVERAGE = 100;
}

static void SYSMENU_HANDL_AUDIO_DNR_MINMAL(int8_t direction)
{
	TRX.DNR_MINIMAL += direction;
	if (TRX.DNR_MINIMAL < 1)
		TRX.DNR_MINIMAL = 1;
	if (TRX.DNR_MINIMAL > 100)
		TRX.DNR_MINIMAL = 100;
}

static void SYSMENU_HANDL_AUDIO_MIC_EQ_LOW(int8_t direction)
{
	TRX.MIC_EQ_LOW += direction;
	if (TRX.MIC_EQ_LOW < -10)
		TRX.MIC_EQ_LOW = -10;
	if (TRX.MIC_EQ_LOW > 10)
		TRX.MIC_EQ_LOW = 10;
	NeedReinitAudioFilters = true;
}

static void SYSMENU_HANDL_AUDIO_MIC_EQ_MID(int8_t direction)
{
	TRX.MIC_EQ_MID += direction;
	if (TRX.MIC_EQ_MID < -10)
		TRX.MIC_EQ_MID = -10;
	if (TRX.MIC_EQ_MID > 10)
		TRX.MIC_EQ_MID = 10;
	NeedReinitAudioFilters = true;
}

static void SYSMENU_HANDL_AUDIO_MIC_EQ_HIG(int8_t direction)
{
	TRX.MIC_EQ_HIG += direction;
	if (TRX.MIC_EQ_HIG < -10)
		TRX.MIC_EQ_HIG = -10;
	if (TRX.MIC_EQ_HIG > 10)
		TRX.MIC_EQ_HIG = 10;
	NeedReinitAudioFilters = true;
}

static void SYSMENU_HANDL_AUDIO_RX_EQ_LOW(int8_t direction)
{
	TRX.RX_EQ_LOW += direction;
	if (TRX.RX_EQ_LOW < -10)
		TRX.RX_EQ_LOW = -10;
	if (TRX.RX_EQ_LOW > 10)
		TRX.RX_EQ_LOW = 10;
	NeedReinitAudioFilters = true;
}

static void SYSMENU_HANDL_AUDIO_RX_EQ_MID(int8_t direction)
{
	TRX.RX_EQ_MID += direction;
	if (TRX.RX_EQ_MID < -10)
		TRX.RX_EQ_MID = -10;
	if (TRX.RX_EQ_MID > 10)
		TRX.RX_EQ_MID = 10;
	NeedReinitAudioFilters = true;
}

static void SYSMENU_HANDL_AUDIO_RX_EQ_HIG(int8_t direction)
{
	TRX.RX_EQ_HIG += direction;
	if (TRX.RX_EQ_HIG < -10)
		TRX.RX_EQ_HIG = -10;
	if (TRX.RX_EQ_HIG > 10)
		TRX.RX_EQ_HIG = 10;
	NeedReinitAudioFilters = true;
}

static void SYSMENU_HANDL_AUDIO_RX_AGC_SSB_Speed(int8_t direction)
{
	TRX.RX_AGC_SSB_speed += direction;
	if (TRX.RX_AGC_SSB_speed < 1)
		TRX.RX_AGC_SSB_speed = 1;
	if (TRX.RX_AGC_SSB_speed > 20)
		TRX.RX_AGC_SSB_speed = 20;
}

static void SYSMENU_HANDL_AUDIO_RX_AGC_CW_Speed(int8_t direction)
{
	TRX.RX_AGC_CW_speed += direction;
	if (TRX.RX_AGC_CW_speed < 1)
		TRX.RX_AGC_CW_speed = 1;
	if (TRX.RX_AGC_CW_speed > 20)
		TRX.RX_AGC_CW_speed = 20;
}

static void SYSMENU_HANDL_AUDIO_TX_AGCSpeed(int8_t direction)
{
	TRX.TX_AGC_speed += direction;
	if (TRX.TX_AGC_speed < 1)
		TRX.TX_AGC_speed = 1;
	if (TRX.TX_AGC_speed > 10)
		TRX.TX_AGC_speed = 10;
}

static void SYSMENU_HANDL_AUDIO_FMSquelch(int8_t direction)
{
	if (direction < 0 && TRX.FM_SQL_threshold == 0)
		return;
	TRX.FM_SQL_threshold += direction;
	if (TRX.FM_SQL_threshold > 10)
		TRX.FM_SQL_threshold = 10;

	int8_t band = getBandFromFreq(CurrentVFO()->Freq, true);
	if (band > 0)
		TRX.BANDS_SAVED_SETTINGS[band].FM_SQL_threshold = TRX.FM_SQL_threshold;
}

static void SYSMENU_HANDL_AUDIO_SSB_HPF_pass(int8_t direction)
{
	if (direction > 0)
	{
		if (TRX.SSB_HPF_Filter == 0)
			TRX.SSB_HPF_Filter = 60;
		else if (TRX.SSB_HPF_Filter == 60)
			TRX.SSB_HPF_Filter = 100;
		else if (TRX.SSB_HPF_Filter == 100)
			TRX.SSB_HPF_Filter = 200;
		else if (TRX.SSB_HPF_Filter == 200)
			TRX.SSB_HPF_Filter = 300;
		else if (TRX.SSB_HPF_Filter == 300)
			TRX.SSB_HPF_Filter = 400;
		else if (TRX.SSB_HPF_Filter == 400)
			TRX.SSB_HPF_Filter = 500;
	}
	else
	{
		if (TRX.SSB_HPF_Filter == 60)
			TRX.SSB_HPF_Filter = 0;
		else if (TRX.SSB_HPF_Filter == 100)
			TRX.SSB_HPF_Filter = 60;
		else if (TRX.SSB_HPF_Filter == 200)
			TRX.SSB_HPF_Filter = 100;
		else if (TRX.SSB_HPF_Filter == 300)
			TRX.SSB_HPF_Filter = 200;
		else if (TRX.SSB_HPF_Filter == 400)
			TRX.SSB_HPF_Filter = 300;
		else if (TRX.SSB_HPF_Filter == 500)
			TRX.SSB_HPF_Filter = 400;
	}
	TRX_setMode(SecondaryVFO()->Mode, SecondaryVFO());
	TRX_setMode(CurrentVFO()->Mode, CurrentVFO());
}

static void SYSMENU_HANDL_AUDIO_CW_HPF_pass(int8_t direction)
{
	if (direction > 0)
	{
		if (TRX.CW_HPF_Filter == 0)
			TRX.CW_HPF_Filter = 60;
		else if (TRX.CW_HPF_Filter == 60)
			TRX.CW_HPF_Filter = 100;
	}
	else
	{
		if (TRX.CW_HPF_Filter == 60)
			TRX.CW_HPF_Filter = 0;
		else if (TRX.CW_HPF_Filter == 100)
			TRX.CW_HPF_Filter = 60;
	}
	TRX_setMode(SecondaryVFO()->Mode, SecondaryVFO());
	TRX_setMode(CurrentVFO()->Mode, CurrentVFO());
}

static void SYSMENU_HANDL_AUDIO_CW_LPF_pass(int8_t direction)
{
	if (direction > 0)
	{
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
	}
	else
	{
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
	}

	TRX_setMode(SecondaryVFO()->Mode, SecondaryVFO());
	TRX_setMode(CurrentVFO()->Mode, CurrentVFO());
}

static void SYSMENU_HANDL_AUDIO_SSB_RX_LPF_pass(int8_t direction)
{
	if (direction > 0)
	{
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
	}
	else
	{
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
	}

	TRX_setMode(SecondaryVFO()->Mode, SecondaryVFO());
	TRX_setMode(CurrentVFO()->Mode, CurrentVFO());
}

static void SYSMENU_HANDL_AUDIO_SSB_TX_LPF_pass(int8_t direction)
{
	if (direction > 0)
	{
		if (TRX.TX_SSB_LPF_Filter == 0)
			TRX.TX_SSB_LPF_Filter = 1400;
		if (TRX.TX_SSB_LPF_Filter == 1400)
			TRX.TX_SSB_LPF_Filter = 1600;
		else if (TRX.TX_SSB_LPF_Filter == 1600)
			TRX.TX_SSB_LPF_Filter = 1800;
		else if (TRX.TX_SSB_LPF_Filter == 1800)
			TRX.TX_SSB_LPF_Filter = 2100;
		else if (TRX.TX_SSB_LPF_Filter == 2100)
			TRX.TX_SSB_LPF_Filter = 2300;
		else if (TRX.TX_SSB_LPF_Filter == 2300)
			TRX.TX_SSB_LPF_Filter = 2500;
		else if (TRX.TX_SSB_LPF_Filter == 2500)
			TRX.TX_SSB_LPF_Filter = 2700;
		else if (TRX.TX_SSB_LPF_Filter == 2700)
			TRX.TX_SSB_LPF_Filter = 2900;
		else if (TRX.TX_SSB_LPF_Filter == 2900)
			TRX.TX_SSB_LPF_Filter = 3000;
		else if (TRX.TX_SSB_LPF_Filter == 3000)
			TRX.TX_SSB_LPF_Filter = 3200;
		else if (TRX.TX_SSB_LPF_Filter == 3200)
			TRX.TX_SSB_LPF_Filter = 3400;
	}
	else
	{
		if (TRX.TX_SSB_LPF_Filter == 1600)
			TRX.TX_SSB_LPF_Filter = 1400;
		else if (TRX.TX_SSB_LPF_Filter == 1800)
			TRX.TX_SSB_LPF_Filter = 1600;
		else if (TRX.TX_SSB_LPF_Filter == 2100)
			TRX.TX_SSB_LPF_Filter = 1800;
		else if (TRX.TX_SSB_LPF_Filter == 2300)
			TRX.TX_SSB_LPF_Filter = 2100;
		else if (TRX.TX_SSB_LPF_Filter == 2500)
			TRX.TX_SSB_LPF_Filter = 2300;
		else if (TRX.TX_SSB_LPF_Filter == 2700)
			TRX.TX_SSB_LPF_Filter = 2500;
		else if (TRX.TX_SSB_LPF_Filter == 2900)
			TRX.TX_SSB_LPF_Filter = 2700;
		else if (TRX.TX_SSB_LPF_Filter == 3000)
			TRX.TX_SSB_LPF_Filter = 2900;
		else if (TRX.TX_SSB_LPF_Filter == 3200)
			TRX.TX_SSB_LPF_Filter = 3000;
		else if (TRX.TX_SSB_LPF_Filter == 3400)
			TRX.TX_SSB_LPF_Filter = 3200;
	}

	TRX_setMode(SecondaryVFO()->Mode, SecondaryVFO());
	TRX_setMode(CurrentVFO()->Mode, CurrentVFO());
}

static void SYSMENU_HANDL_AUDIO_AM_RX_LPF_pass(int8_t direction)
{
	if (direction > 0)
	{
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
	}
	else
	{
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
	}

	TRX_setMode(SecondaryVFO()->Mode, SecondaryVFO());
	TRX_setMode(CurrentVFO()->Mode, CurrentVFO());
}

static void SYSMENU_HANDL_AUDIO_AM_TX_LPF_pass(int8_t direction)
{
	if (direction > 0)
	{
		if (TRX.TX_AM_LPF_Filter == 2100)
			TRX.TX_AM_LPF_Filter = 2300;
		else if (TRX.TX_AM_LPF_Filter == 2300)
			TRX.TX_AM_LPF_Filter = 2500;
		else if (TRX.TX_AM_LPF_Filter == 2500)
			TRX.TX_AM_LPF_Filter = 2700;
		else if (TRX.TX_AM_LPF_Filter == 2700)
			TRX.TX_AM_LPF_Filter = 2900;
		else if (TRX.TX_AM_LPF_Filter == 2900)
			TRX.TX_AM_LPF_Filter = 3000;
		else if (TRX.TX_AM_LPF_Filter == 3000)
			TRX.TX_AM_LPF_Filter = 3200;
		else if (TRX.TX_AM_LPF_Filter == 3200)
			TRX.TX_AM_LPF_Filter = 3400;
		else if (TRX.TX_AM_LPF_Filter == 3400)
			TRX.TX_AM_LPF_Filter = 3600;
		else if (TRX.TX_AM_LPF_Filter == 3600)
			TRX.TX_AM_LPF_Filter = 3800;
		else if (TRX.TX_AM_LPF_Filter == 3800)
			TRX.TX_AM_LPF_Filter = 4000;
		else if (TRX.TX_AM_LPF_Filter == 4000)
			TRX.TX_AM_LPF_Filter = 4500;
		else if (TRX.TX_AM_LPF_Filter == 4500)
			TRX.TX_AM_LPF_Filter = 5000;
		else if (TRX.TX_AM_LPF_Filter == 5000)
			TRX.TX_AM_LPF_Filter = 6000;
		else if (TRX.TX_AM_LPF_Filter == 6000)
			TRX.TX_AM_LPF_Filter = 7000;
		else if (TRX.TX_AM_LPF_Filter == 7000)
			TRX.TX_AM_LPF_Filter = 8000;
		else if (TRX.TX_AM_LPF_Filter == 8000)
			TRX.TX_AM_LPF_Filter = 9000;
		else if (TRX.TX_AM_LPF_Filter == 9000)
			TRX.TX_AM_LPF_Filter = 10000;
	}
	else
	{
		if (TRX.TX_AM_LPF_Filter == 2300)
			TRX.TX_AM_LPF_Filter = 2100;
		else if (TRX.TX_AM_LPF_Filter == 2500)
			TRX.TX_AM_LPF_Filter = 2300;
		else if (TRX.TX_AM_LPF_Filter == 2700)
			TRX.TX_AM_LPF_Filter = 2500;
		else if (TRX.TX_AM_LPF_Filter == 2900)
			TRX.TX_AM_LPF_Filter = 2700;
		else if (TRX.TX_AM_LPF_Filter == 3000)
			TRX.TX_AM_LPF_Filter = 2900;
		else if (TRX.TX_AM_LPF_Filter == 3200)
			TRX.TX_AM_LPF_Filter = 3000;
		else if (TRX.TX_AM_LPF_Filter == 3400)
			TRX.TX_AM_LPF_Filter = 3200;
		else if (TRX.TX_AM_LPF_Filter == 3600)
			TRX.TX_AM_LPF_Filter = 3400;
		else if (TRX.TX_AM_LPF_Filter == 3800)
			TRX.TX_AM_LPF_Filter = 3400;
		else if (TRX.TX_AM_LPF_Filter == 4000)
			TRX.TX_AM_LPF_Filter = 3800;
		else if (TRX.TX_AM_LPF_Filter == 4500)
			TRX.TX_AM_LPF_Filter = 3800;
		else if (TRX.TX_AM_LPF_Filter == 5000)
			TRX.TX_AM_LPF_Filter = 4500;
		else if (TRX.TX_AM_LPF_Filter == 6000)
			TRX.TX_AM_LPF_Filter = 5000;
		else if (TRX.TX_AM_LPF_Filter == 7000)
			TRX.TX_AM_LPF_Filter = 6000;
		else if (TRX.TX_AM_LPF_Filter == 8000)
			TRX.TX_AM_LPF_Filter = 7000;
		else if (TRX.TX_AM_LPF_Filter == 9000)
			TRX.TX_AM_LPF_Filter = 8000;
		else if (TRX.TX_AM_LPF_Filter == 10000)
			TRX.TX_AM_LPF_Filter = 9000;
	}

	TRX_setMode(SecondaryVFO()->Mode, SecondaryVFO());
	TRX_setMode(CurrentVFO()->Mode, CurrentVFO());
}

static void SYSMENU_HANDL_AUDIO_FM_RX_LPF_pass(int8_t direction)
{
	if (direction > 0)
	{
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
	}
	else
	{
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
	}

	TRX_setMode(SecondaryVFO()->Mode, SecondaryVFO());
	TRX_setMode(CurrentVFO()->Mode, CurrentVFO());
}

static void SYSMENU_HANDL_AUDIO_FM_TX_LPF_pass(int8_t direction)
{
	if (direction > 0)
	{
		if (TRX.TX_FM_LPF_Filter == 5000)
			TRX.TX_FM_LPF_Filter = 6000;
		else if (TRX.TX_FM_LPF_Filter == 6000)
			TRX.TX_FM_LPF_Filter = 7000;
		else if (TRX.TX_FM_LPF_Filter == 7000)
			TRX.TX_FM_LPF_Filter = 8000;
		else if (TRX.TX_FM_LPF_Filter == 8000)
			TRX.TX_FM_LPF_Filter = 9000;
		else if (TRX.TX_FM_LPF_Filter == 9000)
			TRX.TX_FM_LPF_Filter = 10000;
		else if (TRX.TX_FM_LPF_Filter == 10000)
			TRX.TX_FM_LPF_Filter = 15000;
		else if (TRX.TX_FM_LPF_Filter == 15000)
			TRX.TX_FM_LPF_Filter = 20000;
	}
	else
	{
		if (TRX.TX_FM_LPF_Filter == 6000)
			TRX.TX_FM_LPF_Filter = 5000;
		else if (TRX.TX_FM_LPF_Filter == 7000)
			TRX.TX_FM_LPF_Filter = 6000;
		else if (TRX.TX_FM_LPF_Filter == 8000)
			TRX.TX_FM_LPF_Filter = 7000;
		else if (TRX.TX_FM_LPF_Filter == 9000)
			TRX.TX_FM_LPF_Filter = 8000;
		else if (TRX.TX_FM_LPF_Filter == 10000)
			TRX.TX_FM_LPF_Filter = 9000;
		else if (TRX.TX_FM_LPF_Filter == 15000)
			TRX.TX_FM_LPF_Filter = 10000;
		else if (TRX.TX_FM_LPF_Filter == 20000)
			TRX.TX_FM_LPF_Filter = 15000;
	}

	TRX_setMode(SecondaryVFO()->Mode, SecondaryVFO());
	TRX_setMode(CurrentVFO()->Mode, CurrentVFO());
}

static void SYSMENU_HANDL_AUDIO_Beeper(int8_t direction)
{
	if (direction > 0)
		TRX.Beeper = true;
	if (direction < 0)
		TRX.Beeper = false;
}

//CW MENU

static void SYSMENU_HANDL_CWMENU(int8_t direction)
{
	sysmenu_handlers_selected = (struct sysmenu_item_handler*)&sysmenu_cw_handlers[0];
	sysmenu_item_count_selected = (uint8_t*)&sysmenu_cw_item_count;
	sysmenu_onroot = false;
	systemMenuIndex = 0;
	LCD_redraw(false);
}

void SYSMENU_CW_WPM_HOTKEY(void)
{
	sysmenu_handlers_selected = (struct sysmenu_item_handler*)&sysmenu_cw_handlers[0];
	sysmenu_item_count_selected = (uint8_t*)&sysmenu_cw_item_count;
	sysmenu_onroot = false;
	systemMenuIndex = 4;
	LCD_redraw(false);
}

void SYSMENU_CW_KEYER_HOTKEY(void)
{
	sysmenu_handlers_selected = (struct sysmenu_item_handler*)&sysmenu_cw_handlers[0];
	sysmenu_item_count_selected = (uint8_t*)&sysmenu_cw_item_count;
	sysmenu_onroot = false;
	systemMenuIndex = 3;
	LCD_redraw(false);
}

static void SYSMENU_HANDL_CW_GENERATOR_SHIFT_HZ(int8_t direction)
{
	TRX.CW_GENERATOR_SHIFT_HZ += direction * 100;
	if (TRX.CW_GENERATOR_SHIFT_HZ < 100)
		TRX.CW_GENERATOR_SHIFT_HZ = 100;
	if (TRX.CW_GENERATOR_SHIFT_HZ > 10000)
		TRX.CW_GENERATOR_SHIFT_HZ = 10000;
}

static void SYSMENU_HANDL_CW_Key_timeout(int8_t direction)
{
	TRX.CW_Key_timeout += direction * 50;
	if (TRX.CW_Key_timeout < 50)
		TRX.CW_Key_timeout = 50;
	if (TRX.CW_Key_timeout > 5000)
		TRX.CW_Key_timeout = 5000;
}

static void SYSMENU_HANDL_CW_Keyer(int8_t direction)
{
	if (direction > 0)
		TRX.CW_KEYER = true;
	if (direction < 0)
		TRX.CW_KEYER = false;
}

static void SYSMENU_HANDL_CW_Keyer_WPM(int8_t direction)
{
	TRX.CW_KEYER_WPM += direction;
	if (TRX.CW_KEYER_WPM < 1)
		TRX.CW_KEYER_WPM = 1;
	if (TRX.CW_KEYER_WPM > 200)
		TRX.CW_KEYER_WPM = 200;
}

static void SYSMENU_HANDL_CW_SelfHear(int8_t direction)
{
	if (direction > 0)
		TRX.CW_SelfHear = true;
	if (direction < 0)
		TRX.CW_SelfHear = false;
}

static void SYSMENU_HANDL_CW_GaussFilter(int8_t direction)
{
	if (direction > 0)
		TRX.CW_GaussFilter = true;
	if (direction < 0)
		TRX.CW_GaussFilter = false;
	NeedReinitAudioFilters = true;
}

//-----------------------------------------------------------------------
static void SYSMENU_HANDL_SCREEN_LCD_position(int8_t direction)
{
	TRX.LCD_position += direction*2;
  if (TRX.LCD_position < 2)
     TRX.LCD_position = 2;
  if (TRX.LCD_position > 4)
     TRX.LCD_position = 4;
	LCD_Init();
	LCD_redraw(true);
}
//-----------------------------------------------------------------------

//SCREEN MENU

static void SYSMENU_HANDL_LCDMENU(int8_t direction)
{
	sysmenu_handlers_selected = (struct sysmenu_item_handler*)&sysmenu_screen_handlers[0];
	sysmenu_item_count_selected = (uint8_t*)&sysmenu_screen_item_count;
	sysmenu_onroot = false;
	systemMenuIndex = 0;
	LCD_UpdateQuery.SystemMenuRedraw = true;
}

static void SYSMENU_HANDL_SCREEN_FFT_Enabled(int8_t direction)
{
	if (direction > 0)
		TRX.FFT_Enabled = true;
	if (direction < 0)
		TRX.FFT_Enabled = false;
}

static void SYSMENU_HANDL_SCREEN_COLOR_THEME(int8_t direction)
{
	if(direction > 0 || TRX.ColorThemeId > 0)
		TRX.ColorThemeId += direction;
	if (TRX.ColorThemeId > (COLOR_THEMES_COUNT - 1))
		TRX.ColorThemeId = (COLOR_THEMES_COUNT - 1);
	
	COLOR = &COLOR_THEMES[TRX.ColorThemeId];
	FFT_Init();
	LCD_redraw(false);
}

static void SYSMENU_HANDL_SCREEN_FFT_Compressor(int8_t direction)
{
	if (direction > 0)
		TRX.FFT_Compressor = true;
	if (direction < 0)
		TRX.FFT_Compressor = false;
}

static void SYSMENU_HANDL_SCREEN_FFT_Averaging(int8_t direction)
{
	TRX.FFT_Averaging += direction;
	if (TRX.FFT_Averaging < 1)
		TRX.FFT_Averaging = 1;
	if (TRX.FFT_Averaging > 10)
		TRX.FFT_Averaging = 10;
}

static void SYSMENU_HANDL_SCREEN_FFT_Window(int8_t direction)
{
	TRX.FFT_Window += direction;
	if (TRX.FFT_Window < 1)
		TRX.FFT_Window = 1;
	if (TRX.FFT_Window > 3)
		TRX.FFT_Window = 3;
	FFT_PreInit();
	FFT_Init();
}

static void SYSMENU_HANDL_SCREEN_FFT_Zoom(int8_t direction)
{
	if (direction > 0)
	{
		if (TRX.FFT_Zoom == 1)
			TRX.FFT_Zoom = 2;
		else if (TRX.FFT_Zoom == 2)
			TRX.FFT_Zoom = 4;
		else if (TRX.FFT_Zoom == 4)
			TRX.FFT_Zoom = 8;
		else if (TRX.FFT_Zoom == 8)
			TRX.FFT_Zoom = 16;
	}
	else
	{
		if (TRX.FFT_Zoom == 2)
			TRX.FFT_Zoom = 1;
		else if (TRX.FFT_Zoom == 4)
			TRX.FFT_Zoom = 2;
		else if (TRX.FFT_Zoom == 8)
			TRX.FFT_Zoom = 4;
		else if (TRX.FFT_Zoom == 16)
			TRX.FFT_Zoom = 8;
	}
	FFT_Init();
}

static void SYSMENU_HANDL_SCREEN_FFT_Color(int8_t direction)
{
	TRX.FFT_Color += direction;
	if (TRX.FFT_Color < 1)
		TRX.FFT_Color = 1;
	if (TRX.FFT_Color > 7)
		TRX.FFT_Color = 7;
	FFT_Init();
}

static void SYSMENU_HANDL_SCREEN_FFT_Grid(int8_t direction)
{
	TRX.FFT_Grid += direction;
	if (TRX.FFT_Grid < 0)
		TRX.FFT_Grid = 0;
	if (TRX.FFT_Grid > 3)
		TRX.FFT_Grid = 3;
	FFT_Init();
}

static void SYSMENU_HANDL_SCREEN_FFT_Background(int8_t direction)
{
	if (direction > 0)
		TRX.FFT_Background = true;
	if (direction < 0)
		TRX.FFT_Background = false;
}

//ADC/DAC MENU

static void SYSMENU_HANDL_ADCMENU(int8_t direction)
{
	sysmenu_handlers_selected = (struct sysmenu_item_handler*)&sysmenu_adc_handlers[0];
	sysmenu_item_count_selected = (uint8_t*)&sysmenu_adc_item_count;
	sysmenu_onroot = false;
	systemMenuIndex = 0;
	LCD_UpdateQuery.SystemMenuRedraw = true;
}

static void SYSMENU_HANDL_ADC_DRIVER(int8_t direction)
{
	if (direction > 0)
		TRX.ADC_Driver = true;
	if (direction < 0)
		TRX.ADC_Driver = false;
	int8_t band = getBandFromFreq(CurrentVFO()->Freq, true);
	if (band > 0)
		TRX.BANDS_SAVED_SETTINGS[band].ADC_Driver = TRX.ADC_Driver;
	FPGA_NeedSendParams = true;
}

static void SYSMENU_HANDL_ADC_SHDN(int8_t direction)
{
	if (direction > 0)
		TRX.ADC_SHDN = true;
	if (direction < 0)
		TRX.ADC_SHDN = false;
	FPGA_NeedSendParams = true;
}

//SET TIME MENU

static void SYSMENU_HANDL_SETTIME(int8_t direction)
{
	if (!sysmenu_timeMenuOpened)
		LCDDriver_Fill(BG_COLOR);
	sysmenu_timeMenuOpened = true;
	static uint8_t Hours;
	static uint8_t Minutes;
	static uint8_t Seconds;
	static uint32_t Time;
	char ctmp[50];
	Time = RTC->TR;
	Hours = ((Time >> 20) & 0x03) * 10 + ((Time >> 16) & 0x0f);
	Minutes = ((Time >> 12) & 0x07) * 10 + ((Time >> 8) & 0x0f);
	Seconds = ((Time >> 4) & 0x07) * 10 + ((Time >> 0) & 0x0f);
	if (direction != 0)
	{
		if (TimeMenuSelection == 0)
		{
			if (Hours == 0 && direction < 0)
				return;
			Hours = (uint8_t)(Hours + direction);
		}
		if (TimeMenuSelection == 1)
		{
			if (Minutes == 0 && direction < 0)
				return;
			Minutes = (uint8_t)(Minutes + direction);
		}
		if (TimeMenuSelection == 2)
		{
			if (Seconds == 0 && direction < 0)
				return;
			Seconds = (uint8_t)(Seconds + direction);
		}
		if (Hours >= 24)
			Hours = 0;
		if (Minutes >= 60)
			Minutes = 0;
		if (Seconds >= 60)
			Seconds = 0;
		RTC_TimeTypeDef sTime;
		sTime.TimeFormat = RTC_HOURFORMAT12_PM;
		sTime.SubSeconds = 0;
		sTime.SecondFraction = 0;
		sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
		sTime.StoreOperation = RTC_STOREOPERATION_SET;
		sTime.Hours = Hours;
		sTime.Minutes = Minutes;
		sTime.Seconds = Seconds;
		BKPSRAM_Enable();
		HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
	}
	sprintf(ctmp, "%d", Hours);
	addSymbols(ctmp, ctmp, 2, "0", false);
	LCDDriver_printText(ctmp, 76, 100, COLOR->BUTTON_TEXT, TimeMenuSelection == 0 ? FG_COLOR : BG_COLOR, 3);
	LCDDriver_printText(":", 124, 100, COLOR->BUTTON_TEXT, BG_COLOR, 3);
	sprintf(ctmp, "%d", Minutes);
	addSymbols(ctmp, ctmp, 2, "0", false);
	LCDDriver_printText(ctmp, 148, 100, COLOR->BUTTON_TEXT, TimeMenuSelection == 1 ? FG_COLOR : BG_COLOR, 3);
	LCDDriver_printText(":", 194, 100, COLOR->BUTTON_TEXT, BG_COLOR, 3);
	sprintf(ctmp, "%d", Seconds);
	addSymbols(ctmp, ctmp, 2, "0", false);
	LCDDriver_printText(ctmp, 220, 100, COLOR->BUTTON_TEXT, TimeMenuSelection == 2 ? FG_COLOR : BG_COLOR, 3);
}

//FLASH MENU

static void SYSMENU_HANDL_Bootloader(int8_t direction)
{
	WM8731_CleanBuffer();
	TRX_NeedGoToBootloader = true;
	TRX_Inited = false;
	LCD_busy = true;
}

#pragma GCC diagnostic ignored "-Wsign-conversion"

//CALIBRATION MENU

static void SYSMENU_HANDL_CALIB_ENCODER_INVERT(int8_t direction)
{
	if (direction > 0)
		CALIBRATE.ENCODER_INVERT = true;
	if (direction < 0)
		CALIBRATE.ENCODER_INVERT = false;
}

static void SYSMENU_HANDL_CALIB_ENCODER2_INVERT(int8_t direction)
{
	if (direction > 0)
		CALIBRATE.ENCODER2_INVERT = true;
	if (direction < 0)
		CALIBRATE.ENCODER2_INVERT = false;
}

static void SYSMENU_HANDL_CALIB_ENCODER_DEBOUNCE(int8_t direction)
{
	if (CALIBRATE.ENCODER_DEBOUNCE > 0 || direction > 0)
		CALIBRATE.ENCODER_DEBOUNCE += direction;
	if (CALIBRATE.ENCODER_DEBOUNCE > 250)
		CALIBRATE.ENCODER_DEBOUNCE = 250;
}

static void SYSMENU_HANDL_CALIB_ENCODER2_DEBOUNCE(int8_t direction)
{
	if (CALIBRATE.ENCODER2_DEBOUNCE > 0 || direction > 0)
		CALIBRATE.ENCODER2_DEBOUNCE += direction;
	if (CALIBRATE.ENCODER2_DEBOUNCE > 250)
		CALIBRATE.ENCODER2_DEBOUNCE = 250;
}

static void SYSMENU_HANDL_CALIB_ENCODER_SLOW_RATE(int8_t direction)
{
	CALIBRATE.ENCODER_SLOW_RATE += direction;
	if (CALIBRATE.ENCODER_SLOW_RATE < 1)
		CALIBRATE.ENCODER_SLOW_RATE = 1;
	if (CALIBRATE.ENCODER_SLOW_RATE > 100)
		CALIBRATE.ENCODER_SLOW_RATE = 100;
}

static void SYSMENU_HANDL_CALIB_ENCODER_ON_FALLING(int8_t direction)
{
	if (direction > 0)
		CALIBRATE.ENCODER_ON_FALLING = true;
	if (direction < 0)
		CALIBRATE.ENCODER_ON_FALLING = false;
}

static void SYSMENU_HANDL_CALIBRATIONMENU(int8_t direction)
{
	if (!sysmenu_hiddenmenu_enabled)
		return;
	sysmenu_handlers_selected = (struct sysmenu_item_handler*)&sysmenu_calibration_handlers[0];
	sysmenu_item_count_selected = (uint8_t*)&sysmenu_calibration_item_count;
	sysmenu_onroot = false;
	systemMenuIndex = 0;
	LCD_UpdateQuery.SystemMenuRedraw = true;
}

static void SYSMENU_HANDL_CALIB_CIC_SHIFT(int8_t direction)
{
	CALIBRATE.CIC_GAINER_val += direction;
	if (CALIBRATE.CIC_GAINER_val < 16)
		CALIBRATE.CIC_GAINER_val = 16;
	if (CALIBRATE.CIC_GAINER_val > 86)
		CALIBRATE.CIC_GAINER_val = 86;
}

static void SYSMENU_HANDL_CALIB_CICCOMP_SHIFT(int8_t direction)
{
	CALIBRATE.CICFIR_GAINER_val += direction;
	if (CALIBRATE.CICFIR_GAINER_val < 16)
		CALIBRATE.CICFIR_GAINER_val = 16;
	if (CALIBRATE.CICFIR_GAINER_val > 30)
		CALIBRATE.CICFIR_GAINER_val = 30;
}

static void SYSMENU_HANDL_CALIB_TXCICCOMP_SHIFT(int8_t direction)
{
	CALIBRATE.TXCICFIR_GAINER_val += direction;
	if (CALIBRATE.TXCICFIR_GAINER_val < 16)
		CALIBRATE.TXCICFIR_GAINER_val = 16;
	if (CALIBRATE.TXCICFIR_GAINER_val > 30)
		CALIBRATE.TXCICFIR_GAINER_val = 30;
}

static void SYSMENU_HANDL_CALIB_DAC_SHIFT(int8_t direction)
{
	CALIBRATE.DAC_GAINER_val += direction;
	if (CALIBRATE.DAC_GAINER_val < 14)
		CALIBRATE.DAC_GAINER_val = 14;
	if (CALIBRATE.DAC_GAINER_val > 28)
		CALIBRATE.DAC_GAINER_val = 28;
}

static void SYSMENU_HANDL_CALIB_RF_GAIN_LF(int8_t direction)
{
	if (CALIBRATE.rf_out_power_lf > 0)
		CALIBRATE.rf_out_power_lf += direction;
	if (CALIBRATE.rf_out_power_lf == 0 && direction > 0)
		CALIBRATE.rf_out_power_lf += direction;
	if (CALIBRATE.rf_out_power_lf > 100)
		CALIBRATE.rf_out_power_lf = 100;
	
	TRX_MAX_TX_Amplitude = getMaxTXAmplitudeOnFreq(CurrentVFO()->Freq);
}

static void SYSMENU_HANDL_CALIB_RF_GAIN_HF_LOW(int8_t direction)
{
	if (CALIBRATE.rf_out_power_hf_low > 0)
		CALIBRATE.rf_out_power_hf_low += direction;
	if (CALIBRATE.rf_out_power_hf_low == 0 && direction > 0)
		CALIBRATE.rf_out_power_hf_low += direction;
	if (CALIBRATE.rf_out_power_hf_low > 100)
		CALIBRATE.rf_out_power_hf_low = 100;
	
	TRX_MAX_TX_Amplitude = getMaxTXAmplitudeOnFreq(CurrentVFO()->Freq);
}

static void SYSMENU_HANDL_CALIB_RF_GAIN_HF(int8_t direction)
{
	if (CALIBRATE.rf_out_power_hf > 0)
		CALIBRATE.rf_out_power_hf += direction;
	if (CALIBRATE.rf_out_power_hf == 0 && direction > 0)
		CALIBRATE.rf_out_power_hf += direction;
	if (CALIBRATE.rf_out_power_hf > 100)
		CALIBRATE.rf_out_power_hf = 100;
	
	TRX_MAX_TX_Amplitude = getMaxTXAmplitudeOnFreq(CurrentVFO()->Freq);
}

static void SYSMENU_HANDL_CALIB_RF_GAIN_HF_HIGH(int8_t direction)
{
	if (CALIBRATE.rf_out_power_hf_high > 0)
		CALIBRATE.rf_out_power_hf_high += direction;
	if (CALIBRATE.rf_out_power_hf_high == 0 && direction > 0)
		CALIBRATE.rf_out_power_hf_high += direction;
	if (CALIBRATE.rf_out_power_hf_high > 100)
		CALIBRATE.rf_out_power_hf_high = 100;
	
	TRX_MAX_TX_Amplitude = getMaxTXAmplitudeOnFreq(CurrentVFO()->Freq);
}

static void SYSMENU_HANDL_CALIB_RF_GAIN_VHF(int8_t direction)
{
	if (CALIBRATE.rf_out_power_vhf > 0)
		CALIBRATE.rf_out_power_vhf += direction;
	if (CALIBRATE.rf_out_power_vhf == 0 && direction > 0)
		CALIBRATE.rf_out_power_vhf += direction;
	if (CALIBRATE.rf_out_power_vhf > 100)
		CALIBRATE.rf_out_power_vhf = 100;
	
	TRX_MAX_TX_Amplitude = getMaxTXAmplitudeOnFreq(CurrentVFO()->Freq);
}

static void SYSMENU_HANDL_CALIB_S_METER(int8_t direction)
{
	CALIBRATE.smeter_calibration += direction;
	if (CALIBRATE.smeter_calibration < -50)
		CALIBRATE.smeter_calibration = -50;
	if (CALIBRATE.smeter_calibration > 50)
		CALIBRATE.smeter_calibration = 50;
}

#pragma GCC diagnostic ignored "-Wsign-conversion"

static void SYSMENU_HANDL_CALIB_SWR_TRANS_RATE(int8_t direction)
{
	CALIBRATE.swr_trans_rate += (float32_t)direction * 0.1f;
	if (CALIBRATE.swr_trans_rate < 1.0f)
		CALIBRATE.swr_trans_rate = 1.0f;
	if (CALIBRATE.swr_trans_rate > 50.0f)
		CALIBRATE.swr_trans_rate = 50.0f;
}

//VOLT-CALIBR
static void SYSMENU_HANDL_CALIB_VOLT(int8_t direction)
{
	CALIBRATE.volt_cal_rate += (float32_t)direction * 0.1f;
	if (CALIBRATE.volt_cal_rate < 1.0f)
		CALIBRATE.volt_cal_rate = 1.0f;
	if (CALIBRATE.volt_cal_rate > 50.0f)
		CALIBRATE.volt_cal_rate = 50.0f;
}

//SERVICES
void SYSMENU_HANDL_SERVICESMENU(int8_t direction)
{
	sysmenu_services_opened = true;
	sysmenu_handlers_selected = (struct sysmenu_item_handler*)&sysmenu_services_handlers[0];
	sysmenu_item_count_selected = (uint8_t*)&sysmenu_services_item_count;
	sysmenu_onroot = false;
	systemMenuIndex = 0;
	LCD_redraw(false);
}

//SWR BAND ANALYZER
static void SYSMENU_HANDL_SWR_BAND_START(int8_t direction)
{
	if (sysmenu_swr_opened)
	{
		SWR_EncRotate(direction);
	}
	else
	{
		sysmenu_swr_opened = true;
		int8_t band = getBandFromFreq(CurrentVFO()->Freq, true);
		SWR_Start(BANDS[band].startFreq - 100000, BANDS[band].endFreq + 100000);
		LCD_UpdateQuery.SystemMenuRedraw = true;
	}
}

//SWR HF ANALYZER
static void SYSMENU_HANDL_SWR_HF_START(int8_t direction)
{
	if (sysmenu_swr_opened)
	{
		SWR_EncRotate(direction);
	}
	else
	{
		sysmenu_swr_opened = true;
		SWR_Start(1000000, 60000000);
		LCD_UpdateQuery.SystemMenuRedraw = true;
	}
}

//COMMON MENU FUNCTIONS
void SYSMENU_drawSystemMenu(bool draw_background)
{
	if (LCD_busy)
	{
		LCD_UpdateQuery.SystemMenu = true;
		if(draw_background)
			LCD_UpdateQuery.SystemMenuRedraw = true;
		return;
	}
	if (!LCD_systemMenuOpened)
		return;
	if (sysmenu_timeMenuOpened)
	{
		SYSMENU_HANDL_SETTIME(0);
	}
	else if (sysmenu_trx_setCallsign_menu_opened)
	{
		SYSMENU_TRX_DrawCallsignMenu(draw_background);
	}
	else if (sysmenu_swr_opened)
	{
		SWR_Draw();
	}
	else if (sysmenu_infowindow_opened)
	{
		
	}
	else
	{
		LCD_busy = true;

		sysmenu_i = 0;
		sysmenu_y = 5;

		if (draw_background)
			LCDDriver_Fill(BG_COLOR);

		uint8_t current_selected_page = systemMenuIndex / LAY_SYSMENU_MAX_ITEMS_ON_PAGE;
		if (current_selected_page * LAY_SYSMENU_MAX_ITEMS_ON_PAGE > *sysmenu_item_count_selected)
			current_selected_page = 0;

		for (uint8_t m = 0; m < *sysmenu_item_count_selected; m++)
		{
			uint8_t current_page = m / LAY_SYSMENU_MAX_ITEMS_ON_PAGE;
			if (current_selected_page == current_page)
				drawSystemMenuElement(sysmenu_handlers_selected[m].title, sysmenu_handlers_selected[m].type, sysmenu_handlers_selected[m].value, false);
		}
		
		LCD_busy = false;
	}

	LCD_UpdateQuery.SystemMenu = false;
	if(draw_background)
		LCD_UpdateQuery.SystemMenuRedraw = false;
}

void eventRotateSystemMenu(int8_t direction)
{
	if (sysmenu_trx_setCallsign_menu_opened)
	{
		SYSMENU_TRX_RotateCallsignChar(direction);
		return;
	}
	if (sysmenu_timeMenuOpened)
	{
		SYSMENU_HANDL_SETTIME(direction);
		return;
	}
	if (sysmenu_handlers_selected[systemMenuIndex].type != SYSMENU_INFOLINE)
		sysmenu_handlers_selected[systemMenuIndex].menuHandler(direction);
	if (sysmenu_handlers_selected[systemMenuIndex].type != SYSMENU_RUN)
		LCD_UpdateQuery.SystemMenuCurrent = true;
}

void eventCloseSystemMenu(void)
{
	if (sysmenu_trx_setCallsign_menu_opened)
	{
		sysmenu_trx_setCallsign_menu_opened = false;
		systemMenuIndex = 0;
		SYSMENU_drawSystemMenu(true);
	}
	else if (sysmenu_timeMenuOpened)
	{
		sysmenu_timeMenuOpened = false;
		systemMenuIndex = 0;
		SYSMENU_drawSystemMenu(true);
	}
	else if (sysmenu_swr_opened)
	{
		sysmenu_swr_opened = false;
		SWR_Stop();
		systemMenuIndex = 0;
		SYSMENU_drawSystemMenu(true);
	}
	else if (sysmenu_infowindow_opened)
	{
		sysmenu_infowindow_opened = false;
		systemMenuIndex = 0;
		SYSMENU_drawSystemMenu(true);
	}
	else if (sysmenu_services_opened)
	{
		sysmenu_handlers_selected = (struct sysmenu_item_handler*)&sysmenu_handlers[0];
		sysmenu_item_count_selected = (uint8_t*)&sysmenu_item_count;
		sysmenu_onroot = true;
		systemMenuIndex = systemMenuRootIndex;
		sysmenu_services_opened = false;
		LCD_systemMenuOpened = false;
		LCD_UpdateQuery.Background = true;
		LCD_redraw(false);
	}
	else
	{
		if (sysmenu_onroot)
		{
			LCD_systemMenuOpened = false;
			LCD_UpdateQuery.Background = true;
			LCD_redraw(false);
		}
		else
		{
			sysmenu_handlers_selected = (struct sysmenu_item_handler*)&sysmenu_handlers[0];
			sysmenu_item_count_selected = (uint8_t*)&sysmenu_item_count;
			sysmenu_onroot = true;
			systemMenuIndex = systemMenuRootIndex;
			SYSMENU_drawSystemMenu(true);
		}
	}
	sysmenu_item_selected_by_enc2 = false;
	NeedSaveSettings = true;
	if (sysmenu_hiddenmenu_enabled)
		NeedSaveCalibration = true;
}

void eventCloseAllSystemMenu(void)
{
	sysmenu_handlers_selected = (struct sysmenu_item_handler*)&sysmenu_handlers[0];
	sysmenu_item_count_selected = (uint8_t*)&sysmenu_item_count;
	sysmenu_onroot = true;
	systemMenuIndex = systemMenuRootIndex;
	sysmenu_item_selected_by_enc2 = false;
	LCD_systemMenuOpened = false;
	LCD_UpdateQuery.Background = true;
	LCD_redraw(false);
}

void eventSecRotateSystemMenu(int8_t direction)
{
	//Callsign menu
	if (sysmenu_trx_setCallsign_menu_opened)
	{
		if (direction < 0 && sysmenu_trx_selected_callsign_char_index > 0)
		{
			sysmenu_trx_selected_callsign_char_index--;
			SYSMENU_TRX_DrawCallsignMenu(true);
		}
		else if (sysmenu_trx_selected_callsign_char_index < (MAX_CALLSIGN_LENGTH - 1))
		{
			sysmenu_trx_selected_callsign_char_index++;
			SYSMENU_TRX_DrawCallsignMenu(true);
		}
		return;
	}
	if (sysmenu_swr_opened)
		return;
	if (sysmenu_infowindow_opened)
		return;
	//time menu
	if (sysmenu_timeMenuOpened)
	{
		LCDDriver_Fill(BG_COLOR);
		if (direction < 0)
		{
			TimeMenuSelection--;
			if (TimeMenuSelection > 2)
				TimeMenuSelection = 2;
		}
		else
		{
			TimeMenuSelection++;
			if (TimeMenuSelection == 3)
				TimeMenuSelection = 0;
		}
		LCD_UpdateQuery.SystemMenu = true;
		return;
	}
	//other
	if (sysmenu_item_selected_by_enc2) //selected by secondary encoder
	{
		eventRotateSystemMenu(direction);
		return;
	}
	
	uint8_t current_page = systemMenuIndex / LAY_SYSMENU_MAX_ITEMS_ON_PAGE;
	LCDDriver_drawFastHLine(0, (5 + (systemMenuIndex - current_page * LAY_SYSMENU_MAX_ITEMS_ON_PAGE) * LAY_SYSMENU_ITEM_HEIGHT) + 17, LAY_SYSMENU_W, BG_COLOR);
	if (direction < 0)
	{
		if (systemMenuIndex > 0)
			systemMenuIndex--;
		else if (!sysmenu_hiddenmenu_enabled && sysmenu_onroot)
			systemMenuIndex = *sysmenu_item_count_selected - 2;
		else
			systemMenuIndex = *sysmenu_item_count_selected - 1;
	}
	else
	{
		if (systemMenuIndex < (*sysmenu_item_count_selected - 1))
		{
			systemMenuIndex++;
		}
		else
			systemMenuIndex = 0;
	}
	if (!sysmenu_hiddenmenu_enabled && sysmenu_handlers_selected[systemMenuIndex].type == SYSMENU_HIDDEN_MENU)
		systemMenuIndex = 0;
	while (systemMenuIndex > 0 && sysmenu_handlers_selected[systemMenuIndex].type == SYSMENU_INFOLINE)
		systemMenuIndex = 0;

	SYSMENU_redrawCurrentItem();
	if (sysmenu_onroot)
		systemMenuRootIndex = systemMenuIndex;
	uint8_t new_page = systemMenuIndex / LAY_SYSMENU_MAX_ITEMS_ON_PAGE;

	if (current_page != new_page)
		SYSMENU_drawSystemMenu(true);
}

void SYSMENU_redrawCurrentItem(void)
{
	uint8_t current_page = systemMenuIndex / LAY_SYSMENU_MAX_ITEMS_ON_PAGE;
	sysmenu_i = (uint8_t)(systemMenuIndex - current_page * LAY_SYSMENU_MAX_ITEMS_ON_PAGE);
	sysmenu_y = 5 + (systemMenuIndex - current_page * LAY_SYSMENU_MAX_ITEMS_ON_PAGE) * LAY_SYSMENU_ITEM_HEIGHT;
	drawSystemMenuElement(sysmenu_handlers_selected[systemMenuIndex].title, sysmenu_handlers_selected[systemMenuIndex].type, sysmenu_handlers_selected[systemMenuIndex].value, true);
}

static void drawSystemMenuElement(char *title, SystemMenuType type, uint32_t *value, bool onlyVal)
{
	if (!sysmenu_hiddenmenu_enabled && type == SYSMENU_HIDDEN_MENU)
		return;

	char ctmp[10] = {0};
	if (!onlyVal)
	{
		LCDDriver_Fill_RectXY(0, sysmenu_y, LAY_SYSMENU_W, sysmenu_y + 17, BG_COLOR);
		LCDDriver_printText(title, LAY_SYSMENU_X1, sysmenu_y, FG_COLOR, BG_COLOR, 2);
	}

	uint16_t x_pos = LAY_SYSMENU_X2;
	float32_t tmp_float = 0;
	switch (type)
	{
	case SYSMENU_UINT8:
		sprintf(ctmp, "%d", (uint8_t)*value);
		break;
	case SYSMENU_UINT16:
		sprintf(ctmp, "%d", (uint16_t)*value);
		break;
	case SYSMENU_UINT32:
		sprintf(ctmp, "%u", (uint32_t)*value);
		x_pos = LAY_SYSMENU_X2_BIGINT;
		break;
	case SYSMENU_UINT32R:
		sprintf(ctmp, "%u", (uint32_t)*value);
		x_pos = LAY_SYSMENU_X2R_BIGINT; //-V1048
		break;
	case SYSMENU_INT8:
		sprintf(ctmp, "%d", (int8_t)*value);
		break;
	case SYSMENU_INT16:
		sprintf(ctmp, "%d", (int16_t)*value);
		break;
	case SYSMENU_INT32:
		sprintf(ctmp, "%d", (int32_t)*value);
		x_pos = LAY_SYSMENU_X2_BIGINT;
		break;
	case SYSMENU_FLOAT32:
		memcpy(&tmp_float, value, sizeof(float32_t));
		sprintf(ctmp, "%.2f", (double)tmp_float);
		x_pos = LAY_SYSMENU_X2_BIGINT;
		break;
	case SYSMENU_BOOLEAN:
		sprintf(ctmp, "%d", (int8_t)*value);
		if ((uint8_t)*value == 1)
			sprintf(ctmp, "YES");
		else
			sprintf(ctmp, "NO");
		break;
	case SYSMENU_RUN:
		sprintf(ctmp, "RUN");
		break;
	case SYSMENU_MENU:
		sprintf(ctmp, "->");
		break;
	case SYSMENU_HIDDEN_MENU:
		sprintf(ctmp, "!!!");
		break;
	case SYSMENU_INFOLINE:
		break;
	}

	if (onlyVal)
		LCDDriver_Fill_RectWH(x_pos, sysmenu_y, 6 * 12, 13, BG_COLOR);
	if (type != SYSMENU_INFOLINE)
		LCDDriver_printText(ctmp, x_pos, sysmenu_y, FG_COLOR, BG_COLOR, 2);

	uint8_t current_selected_page = systemMenuIndex / LAY_SYSMENU_MAX_ITEMS_ON_PAGE;
	if (systemMenuIndex == sysmenu_i + current_selected_page * LAY_SYSMENU_MAX_ITEMS_ON_PAGE)
	{
		if (sysmenu_item_selected_by_enc2)
			LCDDriver_drawFastHLine(0, sysmenu_y + 17, LAY_SYSMENU_W, COLOR->BUTTON_TEXT);
		else
			LCDDriver_drawFastHLine(0, sysmenu_y + 17, LAY_SYSMENU_W, FG_COLOR);
	}
	sysmenu_i++;
	sysmenu_y += LAY_SYSMENU_ITEM_HEIGHT;
}

//secondary encoder click
void SYSMENU_eventSecEncoderClickSystemMenu(void)
{
	if (sysmenu_handlers_selected[systemMenuIndex].type == SYSMENU_MENU || sysmenu_handlers_selected[systemMenuIndex].type == SYSMENU_HIDDEN_MENU || sysmenu_handlers_selected[systemMenuIndex].type == SYSMENU_RUN || sysmenu_handlers_selected[systemMenuIndex].type == SYSMENU_INFOLINE)
	{
		sysmenu_item_selected_by_enc2 = false;
		eventRotateSystemMenu(1);
	}
	else
	{
		sysmenu_item_selected_by_enc2 = !sysmenu_item_selected_by_enc2;
		LCD_UpdateQuery.SystemMenuCurrent = true;
	}
}
