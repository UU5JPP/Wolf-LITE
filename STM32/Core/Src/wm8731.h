#ifndef WM8731_h
#define WM8731_h

#include "stm32f4xx_hal.h"
#include <stdio.h>
#include "audio_processor.h"
#include "functions.h"

#define I2C_ADDRESS_WM8731 0x34                         //audio codec address
#define CODEC_AUDIO_BUFFER_SIZE (AUDIO_BUFFER_SIZE * 2) //the size of the circular buffer is 2 times larger than the FPGA buffer, we work in the first half, then on the other
#define CODEC_AUDIO_BUFFER_HALF_SIZE AUDIO_BUFFER_SIZE  //half buffer

//Public variables
extern int32_t CODEC_Audio_Buffer_RX[CODEC_AUDIO_BUFFER_SIZE];
extern int32_t CODEC_Audio_Buffer_TX[CODEC_AUDIO_BUFFER_SIZE];
extern bool WM8731_DMA_state;       //what part of the buffer are we working with, true - complete; false - half
extern bool WM8731_Buffer_underrun; //lack of data in the buffer from the audio processor
extern uint32_t WM8731_DMA_samples; //count the number of samples transmitted to the audio codec
extern bool WM8731_Beeping; 				//Beeping flag
extern bool WM8731_Muting; 				//Muting flag

//Public methods
extern void WM8731_Init(void);              //I2C audio codec initialization
extern void WM8731_start_i2s_and_dma(void); //I2S bus start
extern void WM8731_CleanBuffer(void);       //clear the audio codec and USB audio buffer
extern void WM8731_TXRX_mode(void);         //switch to mixed mode RX-TX (for LOOP)
extern void WM8731_Mute(void);							//mute audio out
extern void WM8731_UnMute(void);						//disable audio mute
extern void WM8731_Beep(void);							//beep on key press
#endif
