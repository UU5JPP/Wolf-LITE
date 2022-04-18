#ifndef AUDIO_PROCESSOR_h
#define AUDIO_PROCESSOR_h

#include "stm32f4xx_hal.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "functions.h"
#include "settings.h"

#define AUDIO_BUFFER_SIZE (192 * 2)										   // the size of the buffer for working with sound 48kHz
#define AUDIO_BUFFER_HALF_SIZE (AUDIO_BUFFER_SIZE / 2)					   // buffer size for working with sound 48kHz
#define FPGA_TX_IQ_BUFFER_SIZE AUDIO_BUFFER_SIZE						   // size of TX data buffer for FPGA
#define FPGA_TX_IQ_BUFFER_HALF_SIZE (FPGA_TX_IQ_BUFFER_SIZE / 2)		   // half the size of the TX data buffer for FPGA
#define FPGA_RX_IQ_BUFFER_SIZE (FPGA_TX_IQ_BUFFER_SIZE) // size of the RX data buffer from the PGA
#define FPGA_RX_IQ_BUFFER_HALF_SIZE (FPGA_RX_IQ_BUFFER_SIZE / 2)		   // half the size of the RX data buffer from the PGA

#define FM_RX_LPF_ALPHA 0.05f			  // For NFM demodulator: "Alpha" (low-pass) factor to result in -6dB "knee" at approx. 270 Hz 0.05f
#define FM_RX_HPF_ALPHA 0.96f			  // For NFM demodulator: "Alpha" (high-pass) factor to result in -6dB "knee" at approx. 180 Hz 0.96f
#define FM_TX_HPF_ALPHA 0.95f			  // For FM modulator: "Alpha" (high-pass) factor to pre-emphasis
#define FM_SQUELCH_HYSTERESIS 0.3f		  // Hysteresis for FM squelch
#define FM_SQUELCH_PROC_DECIMATION 50	  // Number of times we go through the FM demod algorithm before we do a squelch calculation
#define FM_RX_SQL_SMOOTHING 0.005f		  // Smoothing factor for IIR squelch noise averaging
#define AUDIO_RX_NB_DELAY_BUFFER_ITEMS 32 // NoiseBlanker buffer size
#define AUDIO_RX_NB_DELAY_BUFFER_SIZE (AUDIO_RX_NB_DELAY_BUFFER_ITEMS * 2)
#define AUDIO_MAX_REVERBER_TAPS 10

// Public variables
extern volatile uint32_t AUDIOPROC_samples;								   // audio samples processed in the processor
extern int32_t Processor_AudioBuffer_A[AUDIO_BUFFER_SIZE];				   // buffer A of the audio processor
extern int32_t Processor_AudioBuffer_B[AUDIO_BUFFER_SIZE];				   // buffer B of the audio processor
extern volatile uint_fast8_t Processor_AudioBuffer_ReadyBuffer;			   // which buffer is currently in use, A or B
extern volatile bool Processor_NeedRXBuffer;							   // codec needs data from processor for RX
extern volatile bool Processor_NeedTXBuffer;							   // codec needs data from processor for TX
extern float32_t FPGA_Audio_Buffer_RX_Q_tmp[FPGA_RX_IQ_BUFFER_HALF_SIZE]; // copy of the working part of the FPGA buffers for processing
extern float32_t FPGA_Audio_Buffer_RX_I_tmp[FPGA_RX_IQ_BUFFER_HALF_SIZE];
extern float32_t FPGA_Audio_Buffer_TX_Q_tmp[FPGA_TX_IQ_BUFFER_HALF_SIZE];
extern float32_t FPGA_Audio_Buffer_TX_I_tmp[FPGA_TX_IQ_BUFFER_HALF_SIZE];
extern volatile float32_t Processor_TX_MAX_amplitude_OUT;		// TX uplift after ALC
extern volatile float32_t Processor_RX_Power_value;				// RX signal magnitude
extern volatile float32_t Processor_selected_RFpower_amplitude; // target TX signal amplitude
extern bool NeedReinitReverber;

// Public methods
extern void processRxAudio(void);	  // start audio processor for RX
extern void processTxAudio(void);	  // start audio processor for TX
extern void initAudioProcessor(void); // initialize audio processor

#endif
