#include "functions.h"
#include "stm32f4xx_hal.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "arm_math.h"
#include "fpga.h"
#include "trx_manager.h"
#include "usbd_debug_if.h"
#include "usbd_cat_if.h"
#include "lcd.h"

CPULOAD_t CPU_LOAD = {0};
static bool SPI_busy = false;
volatile bool SPI_process = false;

void dma_memcpy32(uint32_t *dest, uint32_t *src, uint32_t len)
{
	if (len == 0)
		return;
	HAL_DMA_Start(&hdma_memtomem_dma2_stream0, (uint32_t)src, (uint32_t)dest, len);
	HAL_DMA_PollForTransfer(&hdma_memtomem_dma2_stream0, HAL_DMA_FULL_TRANSFER, HAL_MAX_DELAY);
}

void readFromCircleBuffer32(uint32_t *source, uint32_t *dest, uint32_t index, uint32_t length, uint32_t words_to_read)
{
	if (index >= words_to_read)
	{
		dma_memcpy32(&dest[0], &source[index - words_to_read], words_to_read);
	}
	else
	{
		uint_fast16_t prev_part = words_to_read - index;
		dma_memcpy32(&dest[0], &source[length - prev_part], prev_part);
		dma_memcpy32(&dest[prev_part], &source[0], (words_to_read - prev_part));
	}
}

void readHalfFromCircleUSBBuffer24Bit(uint8_t *source, int32_t *dest, uint32_t index, uint32_t length)
{
	uint_fast16_t halflen = length / 2;
	uint_fast16_t readed_index = 0;
	if (index >= halflen)
	{
		for (uint_fast16_t i = (index - halflen); i < index; i++)
		{
			dest[readed_index] = (source[i * 3 + 0] << 8) | (source[i * 3 + 1] << 16) | (source[i * 3 + 2] << 24);
			readed_index++;
		}
	}
	else
	{
		uint_fast16_t prev_part = halflen - index;
		for (uint_fast16_t i = (length - prev_part); i < length; i++)
		{
			dest[readed_index] = (source[i * 3 + 0] << 8) | (source[i * 3 + 1] << 16) | (source[i * 3 + 2] << 24);
			readed_index++;
		}
		for (uint_fast16_t i = 0; i < (halflen - prev_part); i++)
		{
			dest[readed_index] = (source[i * 3 + 0] << 8) | (source[i * 3 + 1] << 16) | (source[i * 3 + 2] << 24);
			readed_index++;
		}
	}
}

void readHalfFromCircleUSBBuffer16Bit(uint8_t *source, int32_t *dest, uint32_t index, uint32_t length)
{
	uint_fast16_t halflen = length / 2;
	uint_fast16_t readed_index = 0;
	if (index >= halflen)
	{
		for (uint_fast16_t i = (index - halflen); i < index; i++)
		{
			dest[readed_index] = (source[i * 2 + 0] << 0) | (source[i * 2 + 1] << 8);
			readed_index++;
		}
	}
	else
	{
		uint_fast16_t prev_part = halflen - index;
		for (uint_fast16_t i = (length - prev_part); i < length; i++)
		{
			dest[readed_index] = (source[i * 2 + 0] << 0) | (source[i * 2 + 1] << 8);
			readed_index++;
		}
		for (uint_fast16_t i = 0; i < (halflen - prev_part); i++)
		{
			dest[readed_index] = (source[i * 2 + 0] << 0) | (source[i * 2 + 1] << 8);
			readed_index++;
		}
	}
}

void sendToDebug_str(char *data)
{
	if (SWD_DEBUG_ENABLED)
		printf("%s", data);
	if (USB_DEBUG_ENABLED)
		DEBUG_Transmit_FIFO((uint8_t *)data, (uint16_t)strlen(data));
	if (LCD_DEBUG_ENABLED)
	{
		static uint16_t dbg_lcd_y = 10;
		LCDDriver_printText(data, 0, dbg_lcd_y, COLOR_RED, BG_COLOR, 1);
		dbg_lcd_y += 9;
		if(dbg_lcd_y >= LCD_HEIGHT)
			dbg_lcd_y = 0;
	}
}

void sendToDebug_strln(char *data)
{
	sendToDebug_str(data);
	sendToDebug_newline();
}

void sendToDebug_str2(char *data1, char *data2)
{
	sendToDebug_str(data1);
	sendToDebug_str(data2);
}

void sendToDebug_str3(char *data1, char *data2, char *data3)
{
	sendToDebug_str(data1);
	sendToDebug_str(data2);
	sendToDebug_str(data3);
}

void sendToDebug_newline(void)
{
	sendToDebug_str("\n");
}

void sendToDebug_flush(void)
{
	uint_fast16_t tryes = 0;
	while (DEBUG_Transmit_FIFO_Events() == USBD_BUSY && tryes < 512)
		tryes++;
}

void sendToDebug_uint8(uint8_t data, bool _inline)
{
	char tmp[50] = ""; //-V808
	if (_inline)
		sprintf(tmp, "%d", data);
	else
		sprintf(tmp, "%d\n", data);
	sendToDebug_str(tmp);
}

void sendToDebug_hex(uint8_t data, bool _inline)
{
	char tmp[50] = ""; //-V808
	if (_inline)
		sprintf(tmp, "%02X", data);
	else
		sprintf(tmp, "%02X\n", data);
	sendToDebug_str(tmp);
}

void sendToDebug_uint16(uint16_t data, bool _inline)
{
	char tmp[50] = ""; //-V808
	if (_inline)
		sprintf(tmp, "%d", data);
	else
		sprintf(tmp, "%d\n", data);
	sendToDebug_str(tmp);
}
void sendToDebug_uint32(uint32_t data, bool _inline)
{
	char tmp[50] = ""; //-V808
	if (_inline)
		sprintf(tmp, "%u", data);
	else
		sprintf(tmp, "%u\n", data);
	sendToDebug_str(tmp);
}
void sendToDebug_int8(int8_t data, bool _inline)
{
	char tmp[50] = ""; //-V808
	if (_inline)
		sprintf(tmp, "%d", data);
	else
		sprintf(tmp, "%d\n", data);
	sendToDebug_str(tmp);
}
void sendToDebug_int16(int16_t data, bool _inline)
{
	char tmp[50] = ""; //-V808
	if (_inline)
		sprintf(tmp, "%d", data);
	else
		sprintf(tmp, "%d\n", data);
	sendToDebug_str(tmp);
}
void sendToDebug_int32(int32_t data, bool _inline)
{
	char tmp[50] = ""; //-V808
	if (_inline)
		sprintf(tmp, "%d", data);
	else
		sprintf(tmp, "%d\n", data);
	sendToDebug_str(tmp);
}

void sendToDebug_float32(float32_t data, bool _inline)
{
	char tmp[50] = ""; //-V808
	if (_inline)
		sprintf(tmp, "%f", (double)data);
	else
		sprintf(tmp, "%f\n", (double)data);
	sendToDebug_str(tmp);
}

uint32_t getRXPhraseFromFrequency(int32_t freq) // calculate the frequency from the phrase for FPGA (RX1 / RX2)
{
	if (freq < 0)
		return 0;
	bool inverted = false;
	int32_t _freq = freq;
	if (_freq > ADC_CLOCK / 2) //Go Nyquist
	{
		while (_freq > (ADC_CLOCK / 2))
		{
			_freq -= (ADC_CLOCK / 2);
			inverted = !inverted;
		}
		if (inverted)
		{
			_freq = (ADC_CLOCK / 2) - _freq;
		}
	}
	TRX_RX_IQ_swap = inverted;
	double res = round(((double)_freq / ADC_CLOCK) * 4194304); //freq in hz/oscil in hz*2^bits;
	return (uint32_t)res;
}

uint32_t getTXPhraseFromFrequency(int32_t freq) // calculate the frequency from the phrase for FPGA (TX)
{
	if (freq < 0)
		return 0;
	bool inverted = false;
	TRX_TX_IQ_swap = false;
	double res = round(((double)freq / DAC_CLOCK) * 4194304); //freq in hz/oscil in hz*2^bits = (freq/48000000)*4194304;
	return (uint32_t)res;
}

void addSymbols(char *dest, char *str, uint_fast8_t length, char *symbol, bool toEnd) // add zeroes
{
	char res[50] = "";
	strcpy(res, str);
	while (strlen(res) < length)
	{
		if (toEnd)
			strcat(res, symbol);
		else
		{
			char tmp[50] = "";
			strcat(tmp, symbol);
			strcat(tmp, res);
			strcpy(res, tmp);
		}
	}
	strcpy(dest, res);
}

float32_t log10f_fast(float32_t X)
{
	float32_t Y, F;
	int32_t E;
	F = frexpf(fabsf(X), &E);
	Y = 1.23149591368684f;
	Y *= F;
	Y += -4.11852516267426f;
	Y *= F;
	Y += 6.02197014179219f;
	Y *= F;
	Y += -3.13396450166353f;
	Y += E;
	return (Y * 0.3010299956639812f);
}

float32_t db2rateV(float32_t i) // from decibels to times (for voltage)
{
	return powf(10.0f, (i / 20.0f));
}

float32_t db2rateP(float32_t i) // from decibels to times (for power)
{
	return powf(10.0f, (i / 10.0f));
}

float32_t rate2dbV(float32_t i) // times to decibels (for voltage)
{
	return 20 * log10f_fast(i);
}

float32_t rate2dbP(float32_t i) // from times to decibels (for power)
{
	return 10 * log10f_fast(i);
}

#define VOLUME_LOW_DB (-40.0f)
#define VOLUME_EPSILON powf(10.0f, (VOLUME_LOW_DB / 20.0f))
float32_t volume2rate(float32_t i) // from the position of the volume knob to the gain
{
	if (i < (2.0f / 100.f)) //mute zone
		return 0.0f;
	return powf(VOLUME_EPSILON, (1.0f - i));
}

void shiftTextLeft(char *string, uint_fast16_t shiftLength)
{
	uint_fast16_t size = strlen(string);
	if (shiftLength >= size)
	{
		memset(string, '\0', size);
		return;
	}
	for (uint_fast16_t i = 0; i < size - shiftLength; i++)
	{
		string[i] = string[i + shiftLength];
		string[i + shiftLength] = '\0';
	}
}

float32_t getMaxTXAmplitudeOnFreq(uint32_t freq)
{
	if (freq > MAX_TX_FREQ_HZ)
		return 0.0f;
	
  if (freq < 2.5 * 1000000)
		return (float32_t)CALIBRATE.rf_out_power_160m / 100.0f * (float32_t)MAX_TX_AMPLITUDE;
	if (freq < 5.3 * 1000000)
		return (float32_t)CALIBRATE.rf_out_power_80m / 100.0f * (float32_t)MAX_TX_AMPLITUDE;
	if (freq < 8.5 * 1000000)
		return (float32_t)CALIBRATE.rf_out_power_40m / 100.0f * (float32_t)MAX_TX_AMPLITUDE;
	if (freq < 12.0 * 1000000)
		return (float32_t)CALIBRATE.rf_out_power_30m / 100.0f * (float32_t)MAX_TX_AMPLITUDE;
	if (freq < 16.0 * 1000000)
		return (float32_t)CALIBRATE.rf_out_power_20m / 100.0f * (float32_t)MAX_TX_AMPLITUDE;
	if (freq < 19.5 * 1000000)
		return (float32_t)CALIBRATE.rf_out_power_17m / 100.0f * (float32_t)MAX_TX_AMPLITUDE;
	if (freq < 22.5 * 1000000)
		return (float32_t)CALIBRATE.rf_out_power_15m / 100.0f * (float32_t)MAX_TX_AMPLITUDE;
	if (freq < 26.5 * 1000000)
		return (float32_t)CALIBRATE.rf_out_power_12m / 100.0f * (float32_t)MAX_TX_AMPLITUDE;
	if (freq < 40.0 * 1000000)
		return (float32_t)CALIBRATE.rf_out_power_10m / 100.0f * (float32_t)MAX_TX_AMPLITUDE;

	  return (float32_t)CALIBRATE.rf_out_power_40m / 100.0f * (float32_t)MAX_TX_AMPLITUDE;
}


float32_t generateSin(float32_t amplitude, uint32_t index, uint32_t samplerate, uint32_t freq)
{
	float32_t ret = amplitude * arm_sin_f32(((float32_t)index / (float32_t)samplerate) * PI * 2.0f * (float32_t)freq);
	return ret;
}

float32_t generateSinF(float32_t amplitude, float32_t *index, uint32_t samplerate, uint32_t freq)
{
	float32_t ret = amplitude * arm_sin_f32(*index * (2.0f * F_PI));
	*index += ((float32_t)freq / (float32_t)samplerate);
	while (*index >= 1.0f)
		*index -= 1.0f;
	return ret;
}

static uint32_t CPULOAD_startWorkTime = 0;
static uint32_t CPULOAD_startSleepTime = 0;
static uint32_t CPULOAD_WorkingTime = 0;
static uint32_t CPULOAD_SleepingTime = 0;
static uint32_t CPULOAD_SleepCounter = 0;
static bool CPULOAD_status = true; // true - wake up ; false - sleep

void CPULOAD_Init(void)
{
	//DBGMCU->CR |= (DBGMCU_CR_DBG_SLEEPD1_Msk | DBGMCU_CR_DBG_STOPD1_Msk | DBGMCU_CR_DBG_STANDBYD1_Msk);
	// allow using the counter
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
	// start the counter
	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
	// zero the value of the counting register
	DWT->CYCCNT = 0;
	CPULOAD_status = true;
}

void CPULOAD_GoToSleepMode(void)
{
	// Add to working time
	CPULOAD_WorkingTime += DWT->CYCCNT - CPULOAD_startWorkTime;
	// Save count cycle time
	CPULOAD_SleepCounter++;
	CPULOAD_startSleepTime = DWT->CYCCNT;
	CPULOAD_status = false;
	// Go to sleep mode Wait for wake up interrupt
	__WFI();
}

void CPULOAD_WakeUp(void)
{
	if (CPULOAD_status)
		return;
	CPULOAD_status = true;
	// Increase number of sleeping time in CPU cycles
	CPULOAD_SleepingTime += DWT->CYCCNT - CPULOAD_startSleepTime;
	// Save current time to get number of working CPU cycles
	CPULOAD_startWorkTime = DWT->CYCCNT;
}

void CPULOAD_Calc(void)
{
	// Save values
	CPU_LOAD.SCNT = CPULOAD_SleepingTime;
	CPU_LOAD.WCNT = CPULOAD_WorkingTime;
	CPU_LOAD.SINC = CPULOAD_SleepCounter;
	CPU_LOAD.Load = ((float)CPULOAD_WorkingTime / (float)(CPULOAD_SleepingTime + CPULOAD_WorkingTime) * 100);
	if (CPU_LOAD.SCNT == 0)
	{
		CPU_LOAD.Load = 100;
	}
	if (CPU_LOAD.SCNT == 0 && CPU_LOAD.WCNT == 0)
	{
		CPU_LOAD.Load = 255;
		//CPULOAD_Init();
	}
	// Reset time
	CPULOAD_SleepingTime = 0;
	CPULOAD_WorkingTime = 0;
	CPULOAD_SleepCounter = 0;
}

inline int32_t convertToSPIBigEndian(int32_t in)
{
	return (int32_t)(0xFFFF0000 & (uint32_t)(in << 16)) | (int32_t)(0x0000FFFF & (uint32_t)(in >> 16));
}

inline uint8_t rev8(uint8_t data)
{
	uint32_t tmp = data;
	return (uint8_t)(__RBIT(tmp) >> 24);
}

bool SPI_Transmit(uint8_t *out_data, uint8_t *in_data, uint16_t count, GPIO_TypeDef *CS_PORT, uint16_t CS_PIN, bool hold_cs, uint32_t prescaler)
{
	if (SPI_busy)
	{
		sendToDebug_strln("SPI Busy");
		return false;
	}
	
	//SPI speed
	if(hspi2.Init.BaudRatePrescaler != prescaler)
	{
		hspi2.Init.BaudRatePrescaler = prescaler;
		HAL_SPI_Init(&hspi2);
	}
	
	const int32_t timeout = 0x200; //HAL_MAX_DELAY
	SPI_busy = true;
	HAL_GPIO_WritePin(CS_PORT, CS_PIN, GPIO_PIN_RESET);
	HAL_StatusTypeDef res = 0;
	if (in_data == NULL)
	{
		res = HAL_SPI_Transmit(&hspi2, out_data, count, timeout);
	}
	else if (out_data == NULL)
	{
		memset(in_data, 0x00, count);
		res = HAL_SPI_Receive(&hspi2, in_data, count, timeout);
	}
	else
	{
		memset(in_data, 0x00, count);
		res = HAL_SPI_TransmitReceive(&hspi2, out_data, in_data, count, timeout);
	}
	if (!hold_cs)
		HAL_GPIO_WritePin(CS_PORT, CS_PIN, GPIO_PIN_SET);
	SPI_busy = false;
	if (res == HAL_TIMEOUT || res == HAL_ERROR)
		return false;
	else
		return true;
}
