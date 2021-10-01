#ifndef SETTINGS_h
#define SETTINGS_h

#include "stm32f4xx_hal.h"
#include <stdio.h>
#include <stdbool.h>
#include "functions.h"
#include "bands.h"

#define SETT_VERSION 101				         	 // Settings config version
#define CALIB_VERSION 100							    	// Calibration config version
//#define ADC_CLOCK 64320000					    	// ADC generator frequency
//#define DAC_CLOCK 160800000					    	// DAC generator frequency
#define ADC_CLOCK (int32_t)(61440000 + (CALIBRATE.VCXO_CALIBR * 10))	// ADC generator frequency калибровка частоты генератора 
#define DAC_CLOCK 153600000			          	// DAC generator frequency
#define MAX_RX_FREQ_HZ 750000000			    	// Maximum receive frequency (from the ADC datasheet)
#define MAX_TX_FREQ_HZ (DAC_CLOCK / 2)			// Maximum transmission frequency
#define TRX_SAMPLERATE 48000								// audio stream sampling rate during processing
#define MAX_TX_AMPLITUDE 0.7f								// Maximum amplitude when transmitting to FPGA
#define AGC_MAX_GAIN 10.0f									// Maximum gain in AGC, dB
#define AGC_CLIPPING 6.0f				 						// Limit over target in AGC, dB
#define TUNE_POWER 100											// % of the power selected in the settings when starting TUNE (100 - full)
#define TX_AGC_MAXGAIN 5.0f									// Maximum microphone gain during compression
#define TX_AGC_NOISEGATE 0.00001f						// Minimum signal level for amplification (below - noise, cut off)
#define AUTOGAIN_TARGET_AMPLITUDE 20000.0f  // maximum amplitude, upon reaching which the autocorrector of the input circuits terminates, and in case of overflow it reduces the gain
#define AUTOGAIN_MAX_AMPLITUDE 30000.0f	    // maximum amplitude, upon reaching which the autocorrector of the input circuits terminates, and in case of overflow it reduces the gain
#define AUTOGAIN_CORRECTOR_WAITSTEP 5	   		// waiting for the averaging of the results when the auto-corrector of the input circuits is running
#define KEY_HOLD_TIME 500										// time of long pressing of the keyboard button for triggering, ms
#define MAX_RF_POWER 50.0f									// Maximum power (for meter scale)
#define SHOW_LOGO true											// Show logo on boot (from images.h)
#define POWERDOWN_TIMEOUT 1000							// time of pressing the shutdown button, for operation, ms
#define USB_RESTART_TIMEOUT 5000						// time after which USB restart occurs if there are no packets
#define ENCODER_ACCELERATION	50						//acceleration rate if rotate
#define ENCODER_MIN_RATE_ACCELERATION	1.2f  //encoder enable rounding if lower than value
#define TRX_MAX_SWR		5											//maximum SWR to enable protect (NOT IN TUNE MODE!)

#define BUTTONS_R7KBI true		  	//Author board buttons

// select LCD, comment on others
//#define LCD_ILI9481 true
//#define LCD_HX8357B true         // Alex
//#define LCD_HX8357C true         // Alex
//#define LCD_ILI9486 true
#define LCD_ILI9481_IPS true

#define SCREEN_ROTATE 2           // povorot displey 2,4

//SPI Speed
#define SPI_FRONT_UNIT_PRESCALER SPI_BAUDRATEPRESCALER_8
#define SPI_EEPROM_PRESCALER SPI_BAUDRATEPRESCALER_8

#define CODEC_BITS_FULL_SCALE 65536																// maximum signal amplitude in the bus // powf (2, FPGA_BUS_BITS)
#define ADC_FULL_SCALE 4095
#define FLOAT_FULL_SCALE_POW 4
#define USB_DEBUG_ENABLED true	// allow using USB as a console
#define SWD_DEBUG_ENABLED false // enable SWD as a console
#define LCD_DEBUG_ENABLED false // enable LCD as a console
#define ADC_INPUT_IMPEDANCE 200.0f //50ohm -> 1:4 trans
#define ADC_RANGE 1.0f
#define ADC_DRIVER_GAIN_DB 20.0f //on 14mhz
#define AUTOGAINER_TAGET (ADC_FULL_SCALE / 3)
#define AUTOGAINER_HYSTERESIS (ADC_FULL_SCALE / 10)

#define MAX_CALLSIGN_LENGTH 16

#define W25Q16_COMMAND_Write_Disable 0x04
#define W25Q16_COMMAND_Write_Enable 0x06
#define W25Q16_COMMAND_Erase_Chip 0xC7
#define W25Q16_COMMAND_Sector_Erase 0x20
#define W25Q16_COMMAND_32KBlock_Erase 0x52
#define W25Q16_COMMAND_Page_Program 0x02
#define W25Q16_COMMAND_Read_Data 0x03
#define W25Q16_COMMAND_FastRead_Data 0x0B
#define W25Q16_COMMAND_Power_Down 0xB9
#define W25Q16_COMMAND_Power_Up 0xAB
#define W25Q16_COMMAND_GetStatus 0x05
#define W25Q16_COMMAND_WriteStatus 0x01
#define W25Q16_SECTOR_SIZE 4096
#define EEPROM_SECTOR_CALIBRATION 0
#define EEPROM_SECTOR_SETTINGS 4
#define EEPROM_REPEAT_TRYES 5 // command tryes

typedef struct
{
	uint32_t Freq;
	uint_fast8_t Mode;
	uint_fast16_t HPF_Filter_Width;
	uint_fast16_t RX_LPF_Filter_Width;
	uint_fast16_t TX_LPF_Filter_Width;
	bool AutoNotchFilter;
	uint_fast16_t NotchFC;
	bool AGC;
} VFO;

// Save settings by band
typedef struct
{
	uint32_t Freq;
	uint8_t Mode;
	float32_t ATT_DB;
	bool ATT;
	bool ADC_Driver;
	uint8_t FM_SQL_threshold;
	bool AGC;
	uint8_t AutoGain_Stage;
} BAND_SAVED_SETTINGS_TYPE;

extern struct TRX_SETTINGS
{
	uint8_t flash_id;
	//TRX
	bool current_vfo; // false - A; true - B
	VFO VFO_A;
	VFO VFO_B;
	bool Fast;
	BAND_SAVED_SETTINGS_TYPE BANDS_SAVED_SETTINGS[BANDS_COUNT];
	float32_t ATT_DB;
	uint8_t ATT_STEP;
	bool ATT;
	uint8_t RF_Power;
	bool ShiftEnabled;
	uint16_t SHIFT_INTERVAL;
	bool TWO_SIGNAL_TUNE;
	uint16_t FRQ_STEP;
	uint16_t FRQ_FAST_STEP;
	bool Debug_Console;
	bool BandMapEnabled;
	bool InputType_MIC;
	bool InputType_LINE;
	bool InputType_USB;
	bool AutoGain;
	bool Locked;
	bool CLAR;
	bool Encoder_Accelerate;
	bool Encoder_OFF;
	char CALLSIGN[MAX_CALLSIGN_LENGTH];
	bool Transverter_Enabled;
	uint16_t Transverter_Offset_Mhz;
	//AUDIO
	uint8_t TX_Compressor_speed_SSB;
	uint8_t TX_Compressor_maxgain_SSB;
	uint8_t TX_Compressor_speed_AMFM;
	uint8_t TX_Compressor_maxgain_AMFM;
	uint8_t Volume;
	uint8_t IF_Gain;
	int8_t AGC_GAIN_TARGET;
	uint16_t RX_AGC_Hold;
	uint8_t MIC_GAIN;
	bool MIC_BOOST;
	int8_t RX_EQ_LOW;
//	int8_t LCD_position;
	int8_t RX_EQ_MID;
	int8_t RX_EQ_HIG;
	int8_t MIC_EQ_LOW;
	int8_t MIC_EQ_MID;
	int8_t MIC_EQ_HIG;
	uint8_t DNR_SNR_THRESHOLD;
	uint8_t DNR_AVERAGE;
	uint8_t DNR_MINIMAL;
	uint8_t RX_AGC_SSB_speed;
	uint8_t RX_AGC_CW_speed;
	uint8_t TX_AGC_speed;
	uint16_t CW_LPF_Filter;
	uint16_t CW_HPF_Filter;
	uint16_t RX_SSB_LPF_Filter;
	uint16_t TX_SSB_LPF_Filter;
	uint16_t SSB_HPF_Filter;
	uint16_t RX_AM_LPF_Filter;
	uint16_t TX_AM_LPF_Filter;
	uint16_t RX_FM_LPF_Filter;
	uint16_t TX_FM_LPF_Filter;
	uint8_t FM_SQL_threshold;
	bool Beeper;
	//CW
	uint16_t CW_GENERATOR_SHIFT_HZ;
	uint16_t CW_Key_timeout;
	uint16_t CW_SelfHear;
	bool CW_KEYER;
	uint16_t CW_KEYER_WPM;
	bool CW_GaussFilter;
	//SCREEN
	uint8_t ColorThemeId;
	bool FFT_Enabled;
	uint8_t FFT_Zoom;
	uint8_t FFT_Averaging;
	uint8_t FFT_Window;
//	uint8_t FFT_Color;
	uint8_t Freq_Font;
	
	bool FFT_Compressor;
	int8_t FFT_Grid;
	bool FFT_Background;
	bool FFT_HoldPeaks;
	//ADC
	bool ADC_Driver;
	bool ADC_SHDN;
	//
	uint8_t csum; //check sum
	uint8_t ENDBit; //end bit
} TRX;

extern struct TRX_CALIBRATE
{
	uint8_t flash_id; //eeprom check
	
	int16_t VCXO_CALIBR;
	bool ENCODER_INVERT;
	bool ENCODER2_INVERT;
	uint8_t ENCODER_DEBOUNCE;
	uint8_t ENCODER2_DEBOUNCE;
	uint8_t ENCODER_SLOW_RATE;
	bool ENCODER_ON_FALLING;
	uint8_t CICFIR_GAINER_val;
	uint8_t TXCICFIR_GAINER_val;
	uint8_t DAC_GAINER_val;
	uint8_t rf_out_power_lf;
	
	uint8_t rf_out_power_hf_low;
	uint8_t rf_out_power_hf;
	uint8_t rf_out_power_hf_high;
	uint8_t rf_out_power_vhf;
	int16_t smeter_calibration;
	float32_t swr_trans_rate;
	float32_t volt_cal_rate;
	
//	int16_t freq_correctur_160;
//	int16_t freq_correctur_80;
//	int16_t freq_correctur_40;
//	int16_t freq_correctur_30;
//	int16_t freq_correctur_20;
//	int16_t freq_correctur_17;
//	int16_t freq_correctur_15;
//	int16_t freq_correctur_12;
//	int16_t freq_correctur_10;
//	int16_t freq_correctur_sibi;
//	int16_t freq_correctur_52;

	uint8_t rf_out_power_160m;
	uint8_t rf_out_power_80m;
  uint8_t rf_out_power_40m;
	uint8_t rf_out_power_30m;
	uint8_t rf_out_power_20m;
	uint8_t rf_out_power_17m;
	uint8_t rf_out_power_15m;
	uint8_t rf_out_power_12m;
	uint8_t rf_out_power_10m;
	
	uint8_t csum; //check sum
	uint8_t ENDBit; //end bit
} CALIBRATE;

extern char version_string[19]; //1.2.3-yymmdd.hhmmss
extern volatile bool NeedSaveSettings;
extern volatile bool NeedSaveCalibration;
extern volatile bool EEPROM_Busy;
extern volatile bool LCD_inited;

extern void InitSettings(void);
extern void LoadSettings(bool clear);
extern void LoadCalibration(bool clear);
extern void SaveSettings(void);
extern void SaveCalibration(void);
extern void SaveSettingsToEEPROM(void);
extern void BKPSRAM_Enable(void);
extern void BKPSRAM_Disable(void);
extern VFO *CurrentVFO(void);
extern VFO *SecondaryVFO(void);

#endif
