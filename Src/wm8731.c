#include "stm32f4xx_hal.h"
#include "wm8731.h"
#include "trx_manager.h"
#include "i2c.h"
#include "lcd.h"
#include "agc.h"
#include "usbd_audio_if.h"

//Public variables
uint32_t WM8731_DMA_samples = 0;									// count the number of samples passed to the audio codec
bool WM8731_DMA_state = true;										// what part of the buffer we are working with, true - compleate; false - half
bool WM8731_Buffer_underrun = false;								// lack of data in the buffer from the audio processor
IRAM1 int32_t CODEC_Audio_Buffer_RX[CODEC_AUDIO_BUFFER_SIZE] = {0}; // audio codec ring buffers
IRAM1 int32_t CODEC_Audio_Buffer_TX[CODEC_AUDIO_BUFFER_SIZE] = {0};
bool WM8731_Beeping;														//Beeping flag
bool WM8731_Muting; 														//Muting flag

//Private variables

//Prototypes
static uint8_t WM8731_SendI2CCommand(uint8_t reg, uint8_t value);																		  // send I2C command to codec
static HAL_StatusTypeDef HAL_I2S_TXRX_DMA(I2S_HandleTypeDef *hi2s, uint16_t *txData, uint16_t *rxData, uint16_t txSize, uint16_t rxSize); // Full-duplex implementation of I2S startup
static void I2SEx_Fix(I2S_HandleTypeDef *hi2s);

// start the I2S bus
void WM8731_start_i2s_and_dma(void)
{
	WM8731_CleanBuffer();
	if (HAL_I2S_GetState(&hi2s3) == HAL_I2S_STATE_READY)
	{
		HAL_I2SEx_TransmitReceive_DMA(&hi2s3, (uint16_t*)&CODEC_Audio_Buffer_RX[0], (uint16_t*)&CODEC_Audio_Buffer_TX[0], CODEC_AUDIO_BUFFER_SIZE);
		I2SEx_Fix(&hi2s3);
	}
}

// clear the audio codec and USB audio buffer
void WM8731_CleanBuffer(void)
{
	memset(CODEC_Audio_Buffer_RX, 0x00, sizeof CODEC_Audio_Buffer_RX);
	memset(CODEC_Audio_Buffer_TX, 0x00, sizeof CODEC_Audio_Buffer_TX);
	memset(USB_AUDIO_rx_buffer_a, 0x00, sizeof USB_AUDIO_rx_buffer_a);
	memset(USB_AUDIO_rx_buffer_b, 0x00, sizeof USB_AUDIO_rx_buffer_a);
	memset(USB_AUDIO_tx_buffer, 0x00, sizeof USB_AUDIO_tx_buffer);
	ResetAGC();
}

// send I2C command to codec
static uint8_t WM8731_SendI2CCommand(uint8_t reg, uint8_t value)
{
	uint8_t st = 2;
	uint8_t repeats = 0;
	while (st != 0 && repeats < 3)
	{
		i2c_beginTransmission_u8(&I2C_WM8731, B8(0011010)); //I2C_ADDRESS_WM8731 00110100
		i2c_write_u8(&I2C_WM8731, reg);					   // MSB
		i2c_write_u8(&I2C_WM8731, value);				   // MSB
		st = i2c_endTransmission(&I2C_WM8731);
		if (st != 0)
			repeats++;
		HAL_Delay(1);
	}
	return st;
}

// switch to TX mode (mute the speaker, etc.)
void WM8731_TX_mode(void)
{
	WM8731_SendI2CCommand(B8(00000101), B8(10000000)); //R2 Left Headphone Out
	WM8731_SendI2CCommand(B8(00000111), B8(10000000)); //R3 Right Headphone Out
	WM8731_SendI2CCommand(B8(00001010), B8(00011110)); //R5 Digital Audio Path Control
	if (TRX.InputType_LINE)							   //line
	{
		WM8731_SendI2CCommand(B8(00000000), B8(00010111)); //R0 Left Line In
		WM8731_SendI2CCommand(B8(00000010), B8(00010111)); //R1 Right Line In
		WM8731_SendI2CCommand(B8(00001000), B8(00000010)); //R4 Analogue Audio Path Control
		WM8731_SendI2CCommand(B8(00001100), B8(01101010)); //R6 Power Down Control
	}
	if (TRX.InputType_MIC) //mic
	{
		WM8731_SendI2CCommand(B8(00000001), B8(10000000)); //R0 Left Line In
		WM8731_SendI2CCommand(B8(00000011), B8(10000000)); //R1 Right Line In
		WM8731_SendI2CCommand(B8(00001000), B8(00000101)); //R4 Analogue Audio Path Control
		WM8731_SendI2CCommand(B8(00001100), B8(01101001)); //R6 Power Down Control
	}
}

// switch to RX mode (mute the microphone, etc.)
void WM8731_RX_mode(void)
{
	WM8731_SendI2CCommand(B8(00000000), B8(10000000)); //R0 Left Line In
	WM8731_SendI2CCommand(B8(00000010), B8(10000000)); //R1 Right Line In
	WM8731_SendI2CCommand(B8(00000101), B8(11111111)); //R2 Left Headphone Out
	WM8731_SendI2CCommand(B8(00000111), B8(11111111)); //R3 Right Headphone Out
	WM8731_SendI2CCommand(B8(00001000), B8(00010110)); //R4 Analogue Audio Path Control
	WM8731_SendI2CCommand(B8(00001010), B8(00010000)); //R5 Digital Audio Path Control
	WM8731_SendI2CCommand(B8(00001100), B8(01100111)); //R6 Power Down Control
}

// switch to mixed RX-TX mode (for LOOP)
void WM8731_TXRX_mode(void) //loopback
{
	WM8731_SendI2CCommand(B8(00000101), B8(11111111)); //R2 Left Headphone Out
	WM8731_SendI2CCommand(B8(00000111), B8(11111111)); //R3 Right Headphone Out
	WM8731_SendI2CCommand(B8(00001010), B8(00010000)); //R5 Digital Audio Path Control
	if (TRX.InputType_LINE)							   //line
	{
		WM8731_SendI2CCommand(B8(00000000), B8(00010111)); //R0 Left Line In
		WM8731_SendI2CCommand(B8(00000010), B8(00010111)); //R1 Right Line In
		WM8731_SendI2CCommand(B8(00001000), B8(00010010)); //R4 Analogue Audio Path Control
		WM8731_SendI2CCommand(B8(00001100), B8(01100010)); //R6 Power Down Control, internal crystal
	}
	if (TRX.InputType_MIC) //mic
	{
		WM8731_SendI2CCommand(B8(00000001), B8(10000000)); //R0 Left Line In
		WM8731_SendI2CCommand(B8(00000011), B8(10000000)); //R1 Right Line In
		WM8731_SendI2CCommand(B8(00001000), B8(00010101)); //R4 Analogue Audio Path Control
		WM8731_SendI2CCommand(B8(00001100), B8(01100001)); //R6 Power Down Control, internal crystal
	}
}

void WM8731_Mute(void)
{
	WM8731_Muting = true;
	HAL_GPIO_WritePin(MUTE_GPIO_Port, MUTE_Pin, GPIO_PIN_RESET);
}

void WM8731_UnMute(void)
{
	WM8731_Muting = false;
	HAL_GPIO_WritePin(MUTE_GPIO_Port, MUTE_Pin, GPIO_PIN_SET);
}

void WM8731_Beep(void)
{
	if(TRX.Beeper)
	{
		WM8731_Beeping = true;
		HAL_Delay(50);
		WM8731_Beeping = false;
	}
}

// initialize the audio codec over I2C
void WM8731_Init(void)
{
	if (WM8731_SendI2CCommand(B8(00011110), B8(00000000)) != 0) //R15 Reset Chip
	{
		sendToDebug_strln("[ERR] Audio codec not found");
		LCD_showError("Audio codec init error", true);
	}
	WM8731_SendI2CCommand(B8(00000101), B8(10000000)); //R2 Left Headphone Out Mute
	WM8731_SendI2CCommand(B8(00000111), B8(10000000)); //R3 Right Headphone Out Mute
	WM8731_SendI2CCommand(B8(00001110), B8(00001110)); //R7 Digital Audio Interface Format, Codec Slave, 32bits, I2S Format, MSB-First left-1 justified
	WM8731_SendI2CCommand(B8(00010000), B8(00000000)); //R8 Sampling Control normal mode, 256fs, SR=0 (MCLK@12.288Mhz, fs=48kHz))
	WM8731_SendI2CCommand(B8(00010010), B8(00000001)); //R9 reactivate digital audio interface
	WM8731_SendI2CCommand(B8(00000000), B8(10000000)); //R0 Left Line In
	WM8731_SendI2CCommand(B8(00000010), B8(10000000)); //R1 Right Line In
	WM8731_SendI2CCommand(B8(00001000), B8(00010110)); //R4 Analogue Audio Path Control
	WM8731_SendI2CCommand(B8(00001010), B8(00010000)); //R5 Digital Audio Path Control
	WM8731_SendI2CCommand(B8(00001100), B8(01100111)); //R6 Power Down Control
	WM8731_UnMute();
}

// RX Buffer is fully sent to the codec
void HAL_I2SEx_TxRxCpltCallback(I2S_HandleTypeDef *hi2s)
{
	if (hi2s->Instance == SPI3)
	{
		if (Processor_NeedRXBuffer) // if the audio codec did not provide data to the buffer, raise the error flag
		{
			WM8731_Buffer_underrun = true;
		}
		WM8731_DMA_state = true;
		Processor_NeedRXBuffer = true;
		if (CurrentVFO()->Mode == TRX_MODE_LOOPBACK)
			Processor_NeedTXBuffer = true;
		WM8731_DMA_samples += (CODEC_AUDIO_BUFFER_SIZE / 2);
	}
}

// RX Buffer half sent to the codec
void HAL_I2SEx_TxRxHalfCpltCallback(I2S_HandleTypeDef *hi2s)
{
	if (hi2s->Instance == SPI3)
	{
		if (Processor_NeedRXBuffer) // if the audio codec did not provide data to the buffer, raise the error flag
		{
			WM8731_Buffer_underrun = true;
		}
		WM8731_DMA_state = false;
		Processor_NeedRXBuffer = true;
		if (CurrentVFO()->Mode == TRX_MODE_LOOPBACK)
			Processor_NeedTXBuffer = true;
		WM8731_DMA_samples += (CODEC_AUDIO_BUFFER_SIZE / 2);
	}
}

static void UA3REO_I2SEx_TxRxDMAHalfCplt(DMA_HandleTypeDef *hdma)
{
	I2S_HandleTypeDef* hi2s = (I2S_HandleTypeDef*)((DMA_HandleTypeDef*)hdma)->Parent;
	HAL_I2SEx_TxRxHalfCpltCallback(hi2s);
}

static void UA3REO_I2SEx_TxRxDMACplt(DMA_HandleTypeDef *hdma)
{
	I2S_HandleTypeDef* hi2s = (I2S_HandleTypeDef*)((DMA_HandleTypeDef*)hdma)->Parent;
	HAL_I2SEx_TxRxCpltCallback(hi2s);
}

static void UA3REO_I2SEx_DMAErr(DMA_HandleTypeDef *hdma)
{
	sendToDebug_str("err");
}

static void I2SEx_Fix(I2S_HandleTypeDef *hi2s)
{
	hi2s->hdmarx->XferHalfCpltCallback = NULL;
	hi2s->hdmatx->XferHalfCpltCallback = UA3REO_I2SEx_TxRxDMAHalfCplt;
	hi2s->hdmarx->XferCpltCallback = NULL;
	hi2s->hdmatx->XferCpltCallback = UA3REO_I2SEx_TxRxDMACplt;
	hi2s->hdmarx->XferErrorCallback = NULL;
	hi2s->hdmatx->XferErrorCallback = UA3REO_I2SEx_DMAErr;
}