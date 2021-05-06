#include "audio_processor.h"
#include "wm8731.h"
#include "audio_filters.h"
#include "agc.h"
#include "settings.h"
#include "usbd_audio_if.h"
#include "auto_notch.h"
#include "trx_manager.h"

// Public variables
volatile uint32_t AUDIOPROC_samples = 0;								  // audio samples processed in the processor
IRAM1 int32_t Processor_AudioBuffer_A[AUDIO_BUFFER_SIZE] = {0};				  // buffer A of the audio processor
IRAM1 int32_t Processor_AudioBuffer_B[AUDIO_BUFFER_SIZE] = {0};				  // buffer B of the audio processor
volatile uint_fast8_t Processor_AudioBuffer_ReadyBuffer = 0;			  // which buffer is currently in use, A or B
volatile bool Processor_NeedRXBuffer = false;							  // codec needs data from processor for RX
volatile bool Processor_NeedTXBuffer = false;							  // codec needs data from processor for TX
IRAM1 float32_t FPGA_Audio_Buffer_RX_Q_tmp[FPGA_RX_IQ_BUFFER_HALF_SIZE] = {0}; // copy of the working part of the FPGA buffers for processing
IRAM1 float32_t FPGA_Audio_Buffer_RX_I_tmp[FPGA_RX_IQ_BUFFER_HALF_SIZE] = {0};
IRAM1 float32_t FPGA_Audio_Buffer_TX_Q_tmp[FPGA_TX_IQ_BUFFER_HALF_SIZE] = {0};
IRAM1 float32_t FPGA_Audio_Buffer_TX_I_tmp[FPGA_TX_IQ_BUFFER_HALF_SIZE] = {0};
volatile float32_t Processor_RX_Power_value;					// RX signal magnitude
volatile float32_t Processor_selected_RFpower_amplitude = 0.0f; // target TX signal amplitude
volatile float32_t Processor_TX_MAX_amplitude_OUT;				// TX uplift after ALC

// Private variables
static uint32_t two_signal_gen_position = 0;																							   // signal position in a two-signal generator
static float32_t ALC_need_gain = 1.0f;																									   // current gain of ALC and audio compressor
static float32_t ALC_need_gain_target = 1.0f;																							   // Target Gain Of ALC And Audio Compressor
static float32_t DFM_RX_lpf_prev = 0, DFM_RX_hpf_prev_a = 0, DFM_RX_hpf_prev_b = 0; // used in FM detection and low / high pass processing
static float32_t DFM_RX_i_prev = 0, DFM_RX_q_prev = 0;									   // used in FM detection and low / high pass processing
static uint_fast8_t DFM_RX_fm_sql_count = 0;																	   // used for squelch processing and debouncing tone detection, respectively
static float32_t DFM_RX_fm_sql_avg = 0.0f;																								   // average SQL in FM
static bool DFM_RX_Squelched = false;
static float32_t current_if_gain = 0.0f;
static float32_t volume_gain = 0.0f;

// Prototypes
static void doRX_HILBERT(uint16_t size);	   // Hilbert filter for phase shift of signals
static void doRX_LPF_IQ(uint16_t size);	   // Low-pass filter for I and Q
static void doRX_LPF_I(uint16_t size);		   // LPF filter for I
static void doRX_GAUSS_I(uint16_t size);				 // Gauss filter for I
static void doRX_HPF_I(uint16_t size);		   // HPF filter for I
static void doRX_AGC(uint16_t size, uint_fast8_t mode);		   // automatic gain control
static void doRX_NOTCH(uint16_t size);					 // notch filter
static void doRX_SMETER(uint16_t size);	   // s-meter
static void doRX_COPYCHANNEL(uint16_t size);  // copy I to Q channel
static void DemodulateFM(uint16_t size);	   // FM demodulator
static void ModulateFM(uint16_t size);								   // FM modulator
static void doRX_EQ(uint16_t size);									   // receiver equalizer
static void doMIC_EQ(uint16_t size);								   // microphone equalizer
static void doRX_IFGain(uint16_t size);	//IF gain

// initialize audio processor
void initAudioProcessor(void)
{
	InitAudioFilters();
}

// start audio processor for RX
void processRxAudio(void)
{
	if (!Processor_NeedRXBuffer)
		return;

	VFO *current_vfo = CurrentVFO();

	AUDIOPROC_samples++;

	uint_fast16_t FPGA_Audio_Buffer_Index_tmp = FPGA_Audio_RXBuffer_Index;
	if (FPGA_Audio_Buffer_Index_tmp == 0)
		FPGA_Audio_Buffer_Index_tmp = FPGA_RX_IQ_BUFFER_SIZE;
	else
		FPGA_Audio_Buffer_Index_tmp--;

	// copy buffer from FPGA
	readFromCircleBuffer32((uint32_t *)&FPGA_Audio_Buffer_RX_Q[0], (uint32_t *)&FPGA_Audio_Buffer_RX_Q_tmp[0], FPGA_Audio_Buffer_Index_tmp, FPGA_RX_IQ_BUFFER_SIZE, FPGA_RX_IQ_BUFFER_HALF_SIZE);
	readFromCircleBuffer32((uint32_t *)&FPGA_Audio_Buffer_RX_I[0], (uint32_t *)&FPGA_Audio_Buffer_RX_I_tmp[0], FPGA_Audio_Buffer_Index_tmp, FPGA_RX_IQ_BUFFER_SIZE, FPGA_RX_IQ_BUFFER_HALF_SIZE);

	//Process DC corrector filter
	if (current_vfo->Mode != TRX_MODE_AM && current_vfo->Mode != TRX_MODE_NFM && current_vfo->Mode != TRX_MODE_WFM)
	{
		dc_filter(FPGA_Audio_Buffer_RX_I_tmp, FPGA_RX_IQ_BUFFER_HALF_SIZE, DC_FILTER_RX_I);
		dc_filter(FPGA_Audio_Buffer_RX_Q_tmp, FPGA_RX_IQ_BUFFER_HALF_SIZE, DC_FILTER_RX_Q);
	}

	//IQ Phase corrector https://github.com/df8oe/UHSDR/wiki/IQ---correction-and-mirror-frequencies
	if(AUDIOPROC_samples == 1)
	{
		float32_t teta1_new = 0;
		float32_t teta3_new = 0;
		static float32_t teta1 = 0;
		static float32_t teta3 = 0;
		for (uint16_t i = 0; i < FPGA_RX_IQ_BUFFER_HALF_SIZE; i++)
		{
			teta1_new += FPGA_Audio_Buffer_RX_Q_tmp[i] * (FPGA_Audio_Buffer_RX_I_tmp[i] < 0.0f ? -1.0f : 1.0f);
			teta3_new += FPGA_Audio_Buffer_RX_Q_tmp[i] * (FPGA_Audio_Buffer_RX_Q_tmp[i] < 0.0f ? -1.0f : 1.0f);
		}
		teta1_new = teta1_new / (float32_t)FPGA_RX_IQ_BUFFER_HALF_SIZE;
		teta3_new = teta3_new / (float32_t)FPGA_RX_IQ_BUFFER_HALF_SIZE;
		teta1 = 0.003f * teta1_new + 0.997f * teta1;
		teta3 = 0.003f * teta3_new + 0.997f * teta3;
		if (teta3 > 0.0f)
			TRX_IQ_phase_error = asinf(teta1 / teta3);
	}

	if(current_vfo->Mode != TRX_MODE_IQ)
		doRX_IFGain(FPGA_RX_IQ_BUFFER_HALF_SIZE);

	switch (current_vfo->Mode) // first receiver
	{
	case TRX_MODE_LSB:
	case TRX_MODE_CW_L:
		doRX_HILBERT(FPGA_RX_IQ_BUFFER_HALF_SIZE);
		arm_sub_f32(FPGA_Audio_Buffer_RX_I_tmp, FPGA_Audio_Buffer_RX_Q_tmp, FPGA_Audio_Buffer_RX_I_tmp, FPGA_RX_IQ_BUFFER_HALF_SIZE); // difference of I and Q - LSB
		doRX_HPF_I(FPGA_RX_IQ_BUFFER_HALF_SIZE);
		doRX_LPF_I(FPGA_RX_IQ_BUFFER_HALF_SIZE);
		doRX_GAUSS_I(FPGA_RX_IQ_BUFFER_HALF_SIZE);
		doRX_NOTCH(FPGA_RX_IQ_BUFFER_HALF_SIZE);
		doRX_SMETER(FPGA_RX_IQ_BUFFER_HALF_SIZE);
		doRX_AGC(FPGA_RX_IQ_BUFFER_HALF_SIZE, current_vfo->Mode);
		doRX_COPYCHANNEL(FPGA_RX_IQ_BUFFER_HALF_SIZE);
		break;
	case TRX_MODE_DIGI_L:
		doRX_HILBERT(FPGA_RX_IQ_BUFFER_HALF_SIZE);
		arm_sub_f32(FPGA_Audio_Buffer_RX_I_tmp, FPGA_Audio_Buffer_RX_Q_tmp, FPGA_Audio_Buffer_RX_I_tmp, FPGA_RX_IQ_BUFFER_HALF_SIZE); // difference of I and Q - LSB
		doRX_LPF_I(FPGA_RX_IQ_BUFFER_HALF_SIZE);
		doRX_SMETER(FPGA_RX_IQ_BUFFER_HALF_SIZE);
		doRX_AGC(FPGA_RX_IQ_BUFFER_HALF_SIZE, current_vfo->Mode);
		doRX_COPYCHANNEL(FPGA_RX_IQ_BUFFER_HALF_SIZE);
		break;
	case TRX_MODE_USB:
	case TRX_MODE_CW_U:
		doRX_HILBERT(FPGA_RX_IQ_BUFFER_HALF_SIZE);
		arm_add_f32(FPGA_Audio_Buffer_RX_I_tmp, FPGA_Audio_Buffer_RX_Q_tmp, FPGA_Audio_Buffer_RX_I_tmp, FPGA_RX_IQ_BUFFER_HALF_SIZE); // sum of I and Q - USB
		doRX_HPF_I(FPGA_RX_IQ_BUFFER_HALF_SIZE);
		doRX_LPF_I(FPGA_RX_IQ_BUFFER_HALF_SIZE);
		doRX_GAUSS_I(FPGA_RX_IQ_BUFFER_HALF_SIZE);
		doRX_NOTCH(FPGA_RX_IQ_BUFFER_HALF_SIZE);
		doRX_SMETER(FPGA_RX_IQ_BUFFER_HALF_SIZE);
		doRX_AGC(FPGA_RX_IQ_BUFFER_HALF_SIZE, current_vfo->Mode);
		doRX_COPYCHANNEL(FPGA_RX_IQ_BUFFER_HALF_SIZE);
		break;
	case TRX_MODE_DIGI_U:
		doRX_HILBERT(FPGA_RX_IQ_BUFFER_HALF_SIZE);
		arm_add_f32(FPGA_Audio_Buffer_RX_I_tmp, FPGA_Audio_Buffer_RX_Q_tmp, FPGA_Audio_Buffer_RX_I_tmp, FPGA_RX_IQ_BUFFER_HALF_SIZE); // sum of I and Q - USB
		doRX_LPF_I(FPGA_RX_IQ_BUFFER_HALF_SIZE);
		doRX_SMETER(FPGA_RX_IQ_BUFFER_HALF_SIZE);
		doRX_AGC(FPGA_RX_IQ_BUFFER_HALF_SIZE, current_vfo->Mode);
		doRX_COPYCHANNEL(FPGA_RX_IQ_BUFFER_HALF_SIZE);
		break;
	case TRX_MODE_AM:
		doRX_LPF_IQ(FPGA_RX_IQ_BUFFER_HALF_SIZE);
		arm_mult_f32(FPGA_Audio_Buffer_RX_I_tmp, FPGA_Audio_Buffer_RX_I_tmp, FPGA_Audio_Buffer_RX_I_tmp, FPGA_RX_IQ_BUFFER_HALF_SIZE);
		arm_mult_f32(FPGA_Audio_Buffer_RX_Q_tmp, FPGA_Audio_Buffer_RX_Q_tmp, FPGA_Audio_Buffer_RX_Q_tmp, FPGA_RX_IQ_BUFFER_HALF_SIZE);
		arm_add_f32(FPGA_Audio_Buffer_RX_I_tmp, FPGA_Audio_Buffer_RX_Q_tmp, FPGA_Audio_Buffer_RX_I_tmp, FPGA_RX_IQ_BUFFER_HALF_SIZE);
		//arm_vsqrt_f32(FPGA_Audio_Buffer_RX_I_tmp, FPGA_Audio_Buffer_RX_I_tmp, FPGA_AUDIO_BUFFER_HALF_SIZE);
		for (uint_fast16_t i = 0; i < FPGA_RX_IQ_BUFFER_HALF_SIZE; i++)
			arm_sqrt_f32(FPGA_Audio_Buffer_RX_I_tmp[i], &FPGA_Audio_Buffer_RX_I_tmp[i]);
		arm_scale_f32(FPGA_Audio_Buffer_RX_I_tmp, 0.5f, FPGA_Audio_Buffer_RX_I_tmp, FPGA_RX_IQ_BUFFER_HALF_SIZE);
		doRX_NOTCH(FPGA_RX_IQ_BUFFER_HALF_SIZE);
		doRX_SMETER(FPGA_RX_IQ_BUFFER_HALF_SIZE);
		doRX_AGC(FPGA_RX_IQ_BUFFER_HALF_SIZE, current_vfo->Mode);
		doRX_COPYCHANNEL(FPGA_RX_IQ_BUFFER_HALF_SIZE);
		break;
	case TRX_MODE_NFM:
		doRX_LPF_IQ(FPGA_RX_IQ_BUFFER_HALF_SIZE);
	case TRX_MODE_WFM:
		doRX_SMETER(FPGA_RX_IQ_BUFFER_HALF_SIZE);
		DemodulateFM(FPGA_RX_IQ_BUFFER_HALF_SIZE);
		//end decimate
		doRX_AGC(FPGA_RX_IQ_BUFFER_HALF_SIZE, current_vfo->Mode);
		doRX_COPYCHANNEL(FPGA_RX_IQ_BUFFER_HALF_SIZE);
		break;
	case TRX_MODE_IQ:
	default:
		doRX_SMETER(FPGA_RX_IQ_BUFFER_HALF_SIZE);
		break;
	}

	//Prepare data to DMA
	int32_t *Processor_AudioBuffer_current;
	if (Processor_AudioBuffer_ReadyBuffer == 0)
		Processor_AudioBuffer_current = &Processor_AudioBuffer_B[0];
	else
		Processor_AudioBuffer_current = &Processor_AudioBuffer_A[0];

	// receiver equalizer
	if (current_vfo->Mode != TRX_MODE_DIGI_L && current_vfo->Mode != TRX_MODE_DIGI_U && current_vfo->Mode != TRX_MODE_IQ)
		doRX_EQ(FPGA_RX_IQ_BUFFER_HALF_SIZE);
	
	// muting
	if (TRX_Mute)
		arm_scale_f32(FPGA_Audio_Buffer_RX_I_tmp, 0.0f, FPGA_Audio_Buffer_RX_I_tmp, FPGA_RX_IQ_BUFFER_HALF_SIZE);
	
	// create buffers for transmission to the codec
	for (uint_fast16_t i = 0; i < FPGA_RX_IQ_BUFFER_HALF_SIZE; i++)
	{
		arm_float_to_q31(&FPGA_Audio_Buffer_RX_I_tmp[i], &Processor_AudioBuffer_current[i * 2], 1);	 //left channel
		if (current_vfo->Mode == TRX_MODE_IQ)
			arm_float_to_q31(&FPGA_Audio_Buffer_RX_Q_tmp[i], &Processor_AudioBuffer_current[i * 2 + 1], 1); //right channel
		else
			arm_float_to_q31(&FPGA_Audio_Buffer_RX_I_tmp[i], &Processor_AudioBuffer_current[i * 2 + 1], 1); //right channel
	}
	if (Processor_AudioBuffer_ReadyBuffer == 0)
		Processor_AudioBuffer_ReadyBuffer = 1;
	else
		Processor_AudioBuffer_ReadyBuffer = 0;

	//Send to USB Audio
	if (USB_AUDIO_need_rx_buffer && TRX_Inited)
	{
		uint8_t *USB_AUDIO_rx_buffer_current;

		if (!USB_AUDIO_current_rx_buffer)
			USB_AUDIO_rx_buffer_current = &USB_AUDIO_rx_buffer_a[0];
		else
			USB_AUDIO_rx_buffer_current = &USB_AUDIO_rx_buffer_b[0];

		//drop LSB 32b->16b
		for (uint_fast16_t i = 0; i < (USB_AUDIO_RX_BUFFER_SIZE / BYTES_IN_SAMPLE_AUDIO_OUT_PACKET); i++)
		{
			USB_AUDIO_rx_buffer_current[i * BYTES_IN_SAMPLE_AUDIO_OUT_PACKET + 0] = (Processor_AudioBuffer_current[i] >> 16) & 0xFF;
			USB_AUDIO_rx_buffer_current[i * BYTES_IN_SAMPLE_AUDIO_OUT_PACKET + 1] = (Processor_AudioBuffer_current[i] >> 24) & 0xFF;
			//USB_AUDIO_rx_buffer_current[i * BYTES_IN_SAMPLE_AUDIO_OUT_PACKET + 2] = (Processor_AudioBuffer_current[i] >> 24) & 0xFF;
		}
		USB_AUDIO_need_rx_buffer = false;
	}

	//OUT Volume
	float32_t volume_gain_new = volume2rate((float32_t)TRX.Volume / 100.0f);
	volume_gain = 0.9f * volume_gain + 0.1f * volume_gain_new;
	for (uint_fast16_t i = 0; i < AUDIO_BUFFER_SIZE; i++)
	{
		Processor_AudioBuffer_current[i] = (int32_t)((float32_t)Processor_AudioBuffer_current[i] * volume_gain);
		Processor_AudioBuffer_current[i] = convertToSPIBigEndian(Processor_AudioBuffer_current[i]); //for 32bit audio
	}

	//Beep signal
	if(WM8731_Beeping)
	{
		float32_t signal = 0;
		int32_t out = 0;
		float32_t amplitude = volume2rate((float32_t)TRX.Volume / 100.0f) * 0.1f;
		for(uint32_t pos = 0; pos < AUDIO_BUFFER_HALF_SIZE; pos++)
		{
			signal = generateSin(amplitude, pos, TRX_SAMPLERATE, 1500);
			arm_float_to_q31(&signal, &out, 1);
			Processor_AudioBuffer_current[pos * 2] = convertToSPIBigEndian(out); //left channel
			Processor_AudioBuffer_current[pos * 2 + 1] = Processor_AudioBuffer_current[pos * 2];					//right channel
		}
	}

	//Mute codec
	if(WM8731_Muting)
	{
		for(uint32_t pos = 0; pos < AUDIO_BUFFER_HALF_SIZE; pos++)
		{
			Processor_AudioBuffer_current[pos * 2] = 0; //left channel
			Processor_AudioBuffer_current[pos * 2 + 1] = 0;					//right channel
		}
	}
	
	//Send to Codec DMA
	if (TRX_Inited)
	{
		if (WM8731_DMA_state) //complete
		{
			HAL_DMA_Start_IT(&hdma_memtomem_dma2_stream1, (uint32_t)&Processor_AudioBuffer_current[0], (uint32_t)&CODEC_Audio_Buffer_RX[AUDIO_BUFFER_SIZE], CODEC_AUDIO_BUFFER_HALF_SIZE); //*2 -> left_right
			//HAL_DMA_PollForTransfer(&hdma_memtomem_dma2_stream1, HAL_DMA_FULL_TRANSFER, HAL_MAX_DELAY);
		}
		else //half
		{
			HAL_DMA_Start_IT(&hdma_memtomem_dma2_stream2, (uint32_t)&Processor_AudioBuffer_current[0], (uint32_t)&CODEC_Audio_Buffer_RX[0], CODEC_AUDIO_BUFFER_HALF_SIZE); //*2 -> left_right
			//HAL_DMA_PollForTransfer(&hdma_memtomem_dma2_stream2, HAL_DMA_FULL_TRANSFER, HAL_MAX_DELAY);
		}
	}

	Processor_NeedRXBuffer = false;
}

// start audio processor for TX
void processTxAudio(void)
{
	if (!Processor_NeedTXBuffer)
		return;
	VFO *current_vfo = CurrentVFO();
	AUDIOPROC_samples++;
	uint_fast8_t mode = current_vfo->Mode;

	// get the amplitude for the selected power and range
	Processor_selected_RFpower_amplitude = log10f_fast(((float32_t)TRX.RF_Power * 0.9f + 10.0f) / 10.0f) * TRX_MAX_TX_Amplitude;
	if (Processor_selected_RFpower_amplitude < 0.0f)
		Processor_selected_RFpower_amplitude = 0.0f;

	if (mode == TRX_MODE_LOOPBACK && !TRX_Tune)
		Processor_selected_RFpower_amplitude = 0.5f;
	
	// zero beats
	if ((TRX_Tune && !TRX.TWO_SIGNAL_TUNE) || (TRX_Tune && (mode == TRX_MODE_CW_L || mode == TRX_MODE_CW_U)))
		Processor_selected_RFpower_amplitude = Processor_selected_RFpower_amplitude * 1.0f;
	
	if (TRX.InputType_USB) //USB AUDIO
	{
		uint32_t buffer_index = USB_AUDIO_GetTXBufferIndex_FS() / BYTES_IN_SAMPLE_AUDIO_OUT_PACKET; //buffer 8bit, data 16 bit
		if ((buffer_index % BYTES_IN_SAMPLE_AUDIO_OUT_PACKET) == 1)
			buffer_index -= (buffer_index % BYTES_IN_SAMPLE_AUDIO_OUT_PACKET);
		readHalfFromCircleUSBBuffer16Bit(&USB_AUDIO_tx_buffer[0], &Processor_AudioBuffer_A[0], buffer_index, (USB_AUDIO_TX_BUFFER_SIZE / BYTES_IN_SAMPLE_AUDIO_OUT_PACKET));
	}
	else //AUDIO CODEC AUDIO
	{
		uint32_t dma_index = CODEC_AUDIO_BUFFER_SIZE - (uint16_t)__HAL_DMA_GET_COUNTER(hi2s3.hdmarx) / 2;
		//uint32_t dma_index = (uint16_t)__HAL_DMA_GET_COUNTER(hi2s3.hdmarx);
		if ((dma_index % 2) == 1)
			dma_index--;
		//sendToDebug_uint32(CODEC_AUDIO_BUFFER_SIZE, false);
		readFromCircleBuffer32((uint32_t *)&CODEC_Audio_Buffer_TX[0], (uint32_t *)&Processor_AudioBuffer_A[0], dma_index, CODEC_AUDIO_BUFFER_SIZE, AUDIO_BUFFER_SIZE);
	}
	//sendToDebug_int32(convertToSPIBigEndian(CODEC_Audio_Buffer_TX[0]), false);
	//sendToDebug_int32(convertToSPIBigEndian(CODEC_Audio_Buffer_TX[380]), false);
	//sendToDebug_int32(convertToSPIBigEndian(CODEC_Audio_Buffer_TX[640]), false);
	//sendToDebug_newline();
	//sendToDebug_int32(convertToSPIBigEndian(Processor_AudioBuffer_A[0]), true);
	//sendToDebug_str(" ");
	//sendToDebug_int32(Processor_AudioBuffer_A[0], false);
	
	//One-signal zero-tune generator
	if (TRX_Tune && !TRX.TWO_SIGNAL_TUNE)
	{
		for (uint_fast16_t i = 0; i < AUDIO_BUFFER_HALF_SIZE; i++)
		{
			FPGA_Audio_Buffer_TX_I_tmp[i] = (Processor_selected_RFpower_amplitude / 100.0f * TUNE_POWER);
			FPGA_Audio_Buffer_TX_Q_tmp[i] = 0.0f;
		}
	}
	
	//Two-signal tune generator
	if (TRX_Tune && TRX.TWO_SIGNAL_TUNE)
	{
		for (uint_fast16_t i = 0; i < AUDIO_BUFFER_HALF_SIZE; i++)
		{
			float32_t point = generateSin((Processor_selected_RFpower_amplitude / 100.0f * TUNE_POWER) / 2.0f, two_signal_gen_position, TRX_SAMPLERATE, 1000);
			point += generateSin((Processor_selected_RFpower_amplitude / 100.0f * TUNE_POWER) / 2.0f, two_signal_gen_position, TRX_SAMPLERATE, 2000);
			two_signal_gen_position++;
			if (two_signal_gen_position >= TRX_SAMPLERATE)
				two_signal_gen_position = 0;
			FPGA_Audio_Buffer_TX_I_tmp[i] = point;
			FPGA_Audio_Buffer_TX_Q_tmp[i] = point;
		}
		//hilbert fir
		// + 45 deg to Q data
		arm_fir_f32(&FIR_TX_Hilbert_Q, FPGA_Audio_Buffer_TX_I_tmp, FPGA_Audio_Buffer_TX_I_tmp, AUDIO_BUFFER_HALF_SIZE);
		// - 45 deg to I data
		arm_fir_f32(&FIR_TX_Hilbert_I, FPGA_Audio_Buffer_TX_Q_tmp, FPGA_Audio_Buffer_TX_Q_tmp, AUDIO_BUFFER_HALF_SIZE);
	}
	
	//FM tone generator
	if (TRX_Tune && (mode == TRX_MODE_NFM || mode == TRX_MODE_WFM))
	{
		static uint32_t tone_counter = 100;
		tone_counter++;
		if(tone_counter >= 400)
			tone_counter = 0;
		for (uint_fast16_t i = 0; i < AUDIO_BUFFER_HALF_SIZE; i++)
		{
			float32_t point = 0.0f;
			if(tone_counter > 300)
				point = generateSin(1.0f, two_signal_gen_position, TRX_SAMPLERATE, 3500);
			else if(tone_counter > 200)
				point = generateSin(1.0f, two_signal_gen_position, TRX_SAMPLERATE, 2000);
			else if(tone_counter > 100)
				point = generateSin(1.0f, two_signal_gen_position, TRX_SAMPLERATE, 1000);
			
			two_signal_gen_position++;
			if (two_signal_gen_position >= TRX_SAMPLERATE)
				two_signal_gen_position = 0;

			FPGA_Audio_Buffer_TX_I_tmp[i] = point;
			FPGA_Audio_Buffer_TX_Q_tmp[i] = point;
		}
		ModulateFM(AUDIO_BUFFER_HALF_SIZE);
	}

	if (!TRX_Tune)
	{
		//Copy and convert buffer
		for (uint_fast16_t i = 0; i < AUDIO_BUFFER_HALF_SIZE; i++)
		{
			//FPGA_Audio_Buffer_TX_I_tmp[i] = (float32_t)Processor_AudioBuffer_A[i * 2] / 2147483648.0f;
			//FPGA_Audio_Buffer_TX_Q_tmp[i] = (float32_t)Processor_AudioBuffer_A[i * 2 + 1] / 2147483648.0f;
			FPGA_Audio_Buffer_TX_I_tmp[i] = (float32_t)convertToSPIBigEndian(Processor_AudioBuffer_A[i * 2]) / 2147483648.0f;
			FPGA_Audio_Buffer_TX_Q_tmp[i] = (float32_t)convertToSPIBigEndian(Processor_AudioBuffer_A[i * 2 + 1]) / 2147483648.0f;
		}

		//sendToDebug_float32(FPGA_Audio_Buffer_TX_I_tmp[0],false);
		
		if (TRX.InputType_MIC)
		{
			//Mic Gain
			arm_scale_f32(FPGA_Audio_Buffer_TX_I_tmp, TRX.MIC_GAIN, FPGA_Audio_Buffer_TX_I_tmp, AUDIO_BUFFER_HALF_SIZE);
			arm_scale_f32(FPGA_Audio_Buffer_TX_Q_tmp, TRX.MIC_GAIN, FPGA_Audio_Buffer_TX_Q_tmp, AUDIO_BUFFER_HALF_SIZE);
			//Mic Equalizer
			if (mode != TRX_MODE_DIGI_L && mode != TRX_MODE_DIGI_U && mode != TRX_MODE_IQ)
				doMIC_EQ(AUDIO_BUFFER_HALF_SIZE);
		}
		//USB Gain (24bit)
		if (TRX.InputType_USB)
		{
			arm_scale_f32(FPGA_Audio_Buffer_TX_I_tmp, 10.0f, FPGA_Audio_Buffer_TX_I_tmp, AUDIO_BUFFER_HALF_SIZE);
			arm_scale_f32(FPGA_Audio_Buffer_TX_Q_tmp, 10.0f, FPGA_Audio_Buffer_TX_Q_tmp, AUDIO_BUFFER_HALF_SIZE);
		}

		//Process DC corrector filter
		dc_filter(FPGA_Audio_Buffer_TX_I_tmp, AUDIO_BUFFER_HALF_SIZE, DC_FILTER_TX_I);
		dc_filter(FPGA_Audio_Buffer_TX_Q_tmp, AUDIO_BUFFER_HALF_SIZE, DC_FILTER_TX_Q);
	}

	if (mode != TRX_MODE_IQ && !TRX_Tune)
	{
		//IIR HPF
		if (current_vfo->HPF_Filter_Width > 0)
			arm_biquad_cascade_df2T_f32(&IIR_TX_HPF_I, FPGA_Audio_Buffer_TX_I_tmp, FPGA_Audio_Buffer_TX_I_tmp, AUDIO_BUFFER_HALF_SIZE);
		//IIR LPF
		if (current_vfo->TX_LPF_Filter_Width > 0)
			arm_biquad_cascade_df2T_f32(&IIR_TX_LPF_I, FPGA_Audio_Buffer_TX_I_tmp, FPGA_Audio_Buffer_TX_I_tmp, AUDIO_BUFFER_HALF_SIZE);
		memcpy(&FPGA_Audio_Buffer_TX_Q_tmp[0], &FPGA_Audio_Buffer_TX_I_tmp[0], AUDIO_BUFFER_HALF_SIZE * 4); //double left and right channel

		switch (mode)
		{
		case TRX_MODE_CW_L:
		case TRX_MODE_CW_U:
			for (uint_fast16_t i = 0; i < AUDIO_BUFFER_HALF_SIZE; i++)
			{
				FPGA_Audio_Buffer_TX_I_tmp[i] = TRX_GenerateCWSignal(Processor_selected_RFpower_amplitude);
				FPGA_Audio_Buffer_TX_Q_tmp[i] = 0.0f;
			}
			break;
		case TRX_MODE_USB:
		case TRX_MODE_DIGI_U:
			//hilbert fir
			// + 45 deg to Q data
			arm_fir_f32(&FIR_TX_Hilbert_Q, FPGA_Audio_Buffer_TX_I_tmp, FPGA_Audio_Buffer_TX_I_tmp, AUDIO_BUFFER_HALF_SIZE);
			// - 45 deg to I data
			arm_fir_f32(&FIR_TX_Hilbert_I, FPGA_Audio_Buffer_TX_Q_tmp, FPGA_Audio_Buffer_TX_Q_tmp, AUDIO_BUFFER_HALF_SIZE);
			break;
		case TRX_MODE_LSB:
		case TRX_MODE_DIGI_L:
			//hilbert fir
			// + 45 deg to I data
			arm_fir_f32(&FIR_TX_Hilbert_I, FPGA_Audio_Buffer_TX_I_tmp, FPGA_Audio_Buffer_TX_I_tmp, AUDIO_BUFFER_HALF_SIZE);
			// - 45 deg to Q data
			arm_fir_f32(&FIR_TX_Hilbert_Q, FPGA_Audio_Buffer_TX_Q_tmp, FPGA_Audio_Buffer_TX_Q_tmp, AUDIO_BUFFER_HALF_SIZE);
			break;
		case TRX_MODE_AM:
			// + 45 deg to I data
			arm_fir_f32(&FIR_TX_Hilbert_I, FPGA_Audio_Buffer_TX_I_tmp, FPGA_Audio_Buffer_TX_I_tmp, AUDIO_BUFFER_HALF_SIZE);
			// - 45 deg to Q data
			arm_fir_f32(&FIR_TX_Hilbert_Q, FPGA_Audio_Buffer_TX_Q_tmp, FPGA_Audio_Buffer_TX_Q_tmp, AUDIO_BUFFER_HALF_SIZE);
			for (size_t i = 0; i < AUDIO_BUFFER_HALF_SIZE; i++)
			{
				float32_t i_am = ((FPGA_Audio_Buffer_TX_I_tmp[i] - FPGA_Audio_Buffer_TX_Q_tmp[i]) + (Processor_selected_RFpower_amplitude)) / 2.0f;
				float32_t q_am = ((FPGA_Audio_Buffer_TX_Q_tmp[i] - FPGA_Audio_Buffer_TX_I_tmp[i]) - (Processor_selected_RFpower_amplitude)) / 2.0f;
				FPGA_Audio_Buffer_TX_I_tmp[i] = i_am;
				FPGA_Audio_Buffer_TX_Q_tmp[i] = q_am;
			}
			break;
		case TRX_MODE_NFM:
		case TRX_MODE_WFM:
			ModulateFM(AUDIO_BUFFER_HALF_SIZE);
			break;
		case TRX_MODE_LOOPBACK:
			break;
		default:
			break;
		}
	}

	//// RF PowerControl (Audio Level Control) Compressor
	// looking for a maximum in amplitude
	float32_t ampl_max_i = 0.0f;
	float32_t ampl_max_q = 0.0f;
	float32_t ampl_min_i = 0.0f;
	float32_t ampl_min_q = 0.0f;
	uint32_t tmp_index;
	arm_max_f32(FPGA_Audio_Buffer_TX_I_tmp, AUDIO_BUFFER_HALF_SIZE, &ampl_max_i, &tmp_index);
	arm_max_f32(FPGA_Audio_Buffer_TX_Q_tmp, AUDIO_BUFFER_HALF_SIZE, &ampl_max_q, &tmp_index);
	arm_min_f32(FPGA_Audio_Buffer_TX_I_tmp, AUDIO_BUFFER_HALF_SIZE, &ampl_min_i, &tmp_index);
	arm_min_f32(FPGA_Audio_Buffer_TX_Q_tmp, AUDIO_BUFFER_HALF_SIZE, &ampl_min_q, &tmp_index);
	float32_t Processor_TX_MAX_amplitude_IN = ampl_max_i;
	if (ampl_max_q > Processor_TX_MAX_amplitude_IN)
		Processor_TX_MAX_amplitude_IN = ampl_max_q;
	if ((-ampl_min_i) > Processor_TX_MAX_amplitude_IN)
		Processor_TX_MAX_amplitude_IN = -ampl_min_i;
	if ((-ampl_min_q) > Processor_TX_MAX_amplitude_IN)
		Processor_TX_MAX_amplitude_IN = -ampl_min_q;

	// calculate the target gain
	if (Processor_TX_MAX_amplitude_IN > 0.0f)
	{
		ALC_need_gain_target = (Processor_selected_RFpower_amplitude * 0.99f) / Processor_TX_MAX_amplitude_IN;
		// move the gain one step
		if (fabsf(ALC_need_gain_target - ALC_need_gain) > 0.00001f) // hysteresis
		{
			if (ALC_need_gain_target > ALC_need_gain)
			{
				if (mode == TRX_MODE_DIGI_L || mode == TRX_MODE_DIGI_U) // FAST AGC
					ALC_need_gain = (ALC_need_gain * (1.0f - (float32_t)TRX.TX_AGC_speed / 30.0f)) + (ALC_need_gain_target * ((float32_t)TRX.TX_AGC_speed / 30.0f));
				else // SLOW AGC
					ALC_need_gain = (ALC_need_gain * (1.0f - (float32_t)TRX.TX_AGC_speed / 1000.0f)) + (ALC_need_gain_target * ((float32_t)TRX.TX_AGC_speed / 1000.0f));
			}
		}
		//just in case
		if (ALC_need_gain < 0.0f)
			ALC_need_gain = 0.0f;
		// overload (clipping), sharply reduce the gain
		if ((ALC_need_gain * Processor_TX_MAX_amplitude_IN) > (Processor_selected_RFpower_amplitude * 1.0f))
		{
			ALC_need_gain = ALC_need_gain_target;
			// sendToDebug_str ("MIC_CLIP");
		}
		if (ALC_need_gain > TX_AGC_MAXGAIN)
			ALC_need_gain = TX_AGC_MAXGAIN;
		// noise threshold
		if (Processor_TX_MAX_amplitude_IN < TX_AGC_NOISEGATE)
			ALC_need_gain = 0.0f;
	}
	// disable gain for some types of mod
	if ((ALC_need_gain > 1.0f) && (mode == TRX_MODE_LOOPBACK))
		ALC_need_gain = 1.0f;
	if (mode == TRX_MODE_CW_L || mode == TRX_MODE_CW_U || mode == TRX_MODE_NFM || mode == TRX_MODE_WFM)
		ALC_need_gain = 1.0f;
	if (TRX_Tune)
		ALC_need_gain = 1.0f;

	// apply gain
	arm_scale_f32(FPGA_Audio_Buffer_TX_I_tmp, ALC_need_gain, FPGA_Audio_Buffer_TX_I_tmp, AUDIO_BUFFER_HALF_SIZE);
	arm_scale_f32(FPGA_Audio_Buffer_TX_Q_tmp, ALC_need_gain, FPGA_Audio_Buffer_TX_Q_tmp, AUDIO_BUFFER_HALF_SIZE);

	Processor_TX_MAX_amplitude_OUT = Processor_TX_MAX_amplitude_IN * ALC_need_gain;
	if (Processor_selected_RFpower_amplitude > 0.0f)
		TRX_ALC = Processor_TX_MAX_amplitude_OUT / Processor_selected_RFpower_amplitude;
	else
		TRX_ALC = 0.0f;
	//RF PowerControl (Audio Level Control) Compressor END

	//Send TX data to FFT
	float32_t* FFTInput_I_current = FFT_buff_current ? (float32_t*)&FFTInput_I_B : (float32_t*)&FFTInput_I_A;
	float32_t* FFTInput_Q_current = FFT_buff_current ? (float32_t*)&FFTInput_Q_B : (float32_t*)&FFTInput_Q_A;
	for (uint_fast16_t i = 0; i < AUDIO_BUFFER_HALF_SIZE; i++)
	{
		FFTInput_I_current[FFT_buff_index] = FPGA_Audio_Buffer_TX_I_tmp[i];
		FFTInput_Q_current[FFT_buff_index] = FPGA_Audio_Buffer_TX_Q_tmp[i];
		FFT_buff_index++;
		if (FFT_buff_index >= FFT_SIZE)
		{
			FFT_buff_index = 0;
			FFT_new_buffer_ready = true;
			FFT_buff_current = !FFT_buff_current;
		}
	}

	//Loopback mode
	if (mode == TRX_MODE_LOOPBACK && !TRX_Tune)
	{
		//OUT Volume
		float32_t volume_gain_tx = volume2rate((float32_t)TRX.Volume / 100.0f);
		arm_scale_f32(FPGA_Audio_Buffer_TX_I_tmp, volume_gain_tx, FPGA_Audio_Buffer_TX_I_tmp, AUDIO_BUFFER_HALF_SIZE);

		for (uint_fast16_t i = 0; i < AUDIO_BUFFER_HALF_SIZE; i++)
		{
			arm_float_to_q31(&FPGA_Audio_Buffer_TX_I_tmp[i], &Processor_AudioBuffer_A[i * 2], 1);
			Processor_AudioBuffer_A[i * 2] = convertToSPIBigEndian(Processor_AudioBuffer_A[i * 2]); //left channel
			Processor_AudioBuffer_A[i * 2 + 1] = Processor_AudioBuffer_A[i * 2];					//right channel
		}

		if (WM8731_DMA_state) //compleate
		{
			HAL_DMA_Start(&hdma_memtomem_dma2_stream1, (uint32_t)&Processor_AudioBuffer_A[0], (uint32_t)&CODEC_Audio_Buffer_RX[AUDIO_BUFFER_SIZE], AUDIO_BUFFER_SIZE);
			HAL_DMA_PollForTransfer(&hdma_memtomem_dma2_stream1, HAL_DMA_FULL_TRANSFER, HAL_MAX_DELAY);
		}
		else //half
		{
			HAL_DMA_Start(&hdma_memtomem_dma2_stream2, (uint32_t)&Processor_AudioBuffer_A[0], (uint32_t)&CODEC_Audio_Buffer_RX[0], AUDIO_BUFFER_SIZE);
			HAL_DMA_PollForTransfer(&hdma_memtomem_dma2_stream2, HAL_DMA_FULL_TRANSFER, HAL_MAX_DELAY);
		}
	}
	else
	{
		//CW SelfHear
		if (TRX.CW_SelfHear && (TRX.CW_KEYER || TRX_key_serial || TRX_key_dot_hard || TRX_key_dash_hard) && (mode == TRX_MODE_CW_L || mode == TRX_MODE_CW_U) && !TRX_Tune)
		{
			float32_t volume_gain_tx = volume2rate((float32_t)TRX.Volume / 100.0f);
			float32_t amplitude = (db2rateV(TRX.AGC_GAIN_TARGET) * volume_gain_tx * CODEC_BITS_FULL_SCALE / 2.0f);
			for (uint_fast16_t i = 0; i < AUDIO_BUFFER_HALF_SIZE; i++)
			{
				int32_t data = convertToSPIBigEndian((int32_t)(amplitude  * ( FPGA_Audio_Buffer_TX_I_tmp[i] / Processor_selected_RFpower_amplitude) * arm_sin_f32(((float32_t)i / (float32_t)TRX_SAMPLERATE) * PI * 2.0f * (float32_t)TRX.CW_GENERATOR_SHIFT_HZ)));
				if (WM8731_DMA_state)
				{
					CODEC_Audio_Buffer_RX[AUDIO_BUFFER_SIZE + i * 2] = data;
					CODEC_Audio_Buffer_RX[AUDIO_BUFFER_SIZE + i * 2 + 1] = data;
				}
				else
				{
					CODEC_Audio_Buffer_RX[i * 2] = data;
					CODEC_Audio_Buffer_RX[i * 2 + 1] = data;
				}
			}
		}
		else if (TRX.CW_SelfHear)
		{
			memset(CODEC_Audio_Buffer_RX, 0x00, sizeof CODEC_Audio_Buffer_RX);
		}
		//
		if (FPGA_Audio_Buffer_State) //Send to FPGA DMA
		{
			//sendToDebug_float32(FPGA_Audio_SendBuffer_I[0],false);
			HAL_DMA_Start(&hdma_memtomem_dma2_stream1, (uint32_t)&FPGA_Audio_Buffer_TX_I_tmp[0], (uint32_t)&FPGA_Audio_SendBuffer_I[AUDIO_BUFFER_HALF_SIZE], AUDIO_BUFFER_HALF_SIZE);
			HAL_DMA_PollForTransfer(&hdma_memtomem_dma2_stream1, HAL_DMA_FULL_TRANSFER, HAL_MAX_DELAY);
			HAL_DMA_Start(&hdma_memtomem_dma2_stream1, (uint32_t)&FPGA_Audio_Buffer_TX_Q_tmp[0], (uint32_t)&FPGA_Audio_SendBuffer_Q[AUDIO_BUFFER_HALF_SIZE], AUDIO_BUFFER_HALF_SIZE);
			HAL_DMA_PollForTransfer(&hdma_memtomem_dma2_stream1, HAL_DMA_FULL_TRANSFER, HAL_MAX_DELAY);
		}
		else
		{
			HAL_DMA_Start(&hdma_memtomem_dma2_stream2, (uint32_t)&FPGA_Audio_Buffer_TX_I_tmp[0], (uint32_t)&FPGA_Audio_SendBuffer_I[0], AUDIO_BUFFER_HALF_SIZE);
			HAL_DMA_PollForTransfer(&hdma_memtomem_dma2_stream2, HAL_DMA_FULL_TRANSFER, HAL_MAX_DELAY);
			HAL_DMA_Start(&hdma_memtomem_dma2_stream2, (uint32_t)&FPGA_Audio_Buffer_TX_Q_tmp[0], (uint32_t)&FPGA_Audio_SendBuffer_Q[0], AUDIO_BUFFER_HALF_SIZE);
			HAL_DMA_PollForTransfer(&hdma_memtomem_dma2_stream2, HAL_DMA_FULL_TRANSFER, HAL_MAX_DELAY);
		}
	}

	Processor_NeedTXBuffer = false;
	Processor_NeedRXBuffer = false;
	USB_AUDIO_need_rx_buffer = false;
}

// Hilbert filter for phase shift of signals
static void doRX_HILBERT(uint16_t size)
{
	arm_fir_f32(&FIR_RX_Hilbert_I, FPGA_Audio_Buffer_RX_I_tmp, FPGA_Audio_Buffer_RX_I_tmp, size);
	arm_fir_f32(&FIR_RX_Hilbert_Q, FPGA_Audio_Buffer_RX_Q_tmp, FPGA_Audio_Buffer_RX_Q_tmp, size);
}

// Low-pass filter for I and Q
static void doRX_LPF_IQ(uint16_t size)
{
	if (CurrentVFO()->RX_LPF_Filter_Width > 0)
	{
		arm_biquad_cascade_df2T_f32(&IIR_RX_LPF_I, FPGA_Audio_Buffer_RX_I_tmp, FPGA_Audio_Buffer_RX_I_tmp, size);
		arm_biquad_cascade_df2T_f32(&IIR_RX_LPF_Q, FPGA_Audio_Buffer_RX_Q_tmp, FPGA_Audio_Buffer_RX_Q_tmp, size);
	}
}

// LPF filter for I
static void doRX_LPF_I(uint16_t size)
{
	if (CurrentVFO()->RX_LPF_Filter_Width > 0)
	{
		arm_biquad_cascade_df2T_f32(&IIR_RX_LPF_I, FPGA_Audio_Buffer_RX_I_tmp, FPGA_Audio_Buffer_RX_I_tmp, size);
	}
}

// Gauss filter for I
static void doRX_GAUSS_I(uint16_t size)
{
	if (!TRX.CW_GaussFilter)
		return;
	if (CurrentVFO()->Mode == TRX_MODE_CW_L || CurrentVFO()->Mode == TRX_MODE_CW_U)
	{
		arm_biquad_cascade_df2T_f32(&IIR_RX_GAUSS, FPGA_Audio_Buffer_RX_I_tmp, FPGA_Audio_Buffer_RX_I_tmp, size);
	}
}

// HPF filter for I
static void doRX_HPF_I(uint16_t size)
{
	if (CurrentVFO()->HPF_Filter_Width > 0)
	{
		arm_biquad_cascade_df2T_f32(&IIR_RX_HPF_I, FPGA_Audio_Buffer_RX_I_tmp, FPGA_Audio_Buffer_RX_I_tmp, size);
	}
}

// notch filter
static void doRX_NOTCH(uint16_t size)
{
	if (CurrentVFO()->AutoNotchFilter) // automatic filter
	{
		for (uint32_t block = 0; block < (size / AUTO_NOTCH_BLOCK_SIZE); block++)
			processAutoNotchReduction(FPGA_Audio_Buffer_RX_I_tmp + (block * AUTO_NOTCH_BLOCK_SIZE));
	}
}

// RX Equalizer
static void doRX_EQ(uint16_t size)
{
	if (TRX.RX_EQ_LOW != 0)
		arm_biquad_cascade_df2T_f32(&EQ_RX_LOW_FILTER, FPGA_Audio_Buffer_RX_I_tmp, FPGA_Audio_Buffer_RX_I_tmp, size);
	if (TRX.RX_EQ_MID != 0)
		arm_biquad_cascade_df2T_f32(&EQ_RX_MID_FILTER, FPGA_Audio_Buffer_RX_I_tmp, FPGA_Audio_Buffer_RX_I_tmp, size);
	if (TRX.RX_EQ_HIG != 0)
		arm_biquad_cascade_df2T_f32(&EQ_RX_HIG_FILTER, FPGA_Audio_Buffer_RX_I_tmp, FPGA_Audio_Buffer_RX_I_tmp, size);
}

// Equalizer microphone
static void doMIC_EQ(uint16_t size)
{
	if (TRX.MIC_EQ_LOW != 0)
		arm_biquad_cascade_df2T_f32(&EQ_MIC_LOW_FILTER, FPGA_Audio_Buffer_TX_I_tmp, FPGA_Audio_Buffer_TX_I_tmp, size);
	if (TRX.MIC_EQ_MID != 0)
		arm_biquad_cascade_df2T_f32(&EQ_MIC_MID_FILTER, FPGA_Audio_Buffer_TX_I_tmp, FPGA_Audio_Buffer_TX_I_tmp, size);
	if (TRX.MIC_EQ_HIG != 0)
		arm_biquad_cascade_df2T_f32(&EQ_MIC_HIG_FILTER, FPGA_Audio_Buffer_TX_I_tmp, FPGA_Audio_Buffer_TX_I_tmp, size);
}

// automatic gain control
static void doRX_AGC(uint16_t size, uint_fast8_t mode)
{
	DoRxAGC(FPGA_Audio_Buffer_RX_I_tmp, size, mode);
}

// s-meter
static void doRX_SMETER(uint16_t size)
{
	if(Processor_RX_Power_value != 0)
		return;
	// Prepare data to calculate s-meter
	float32_t i = 0;
	arm_rms_f32(FPGA_Audio_Buffer_RX_I_tmp, size, &i);
	if(current_if_gain > 0)
		i *= 1.0f / current_if_gain;
	Processor_RX_Power_value = i;
}

// copy I to Q channel
static void doRX_COPYCHANNEL(uint16_t size)
{
	// Double channel I-> Q
	dma_memcpy32((uint32_t *)&FPGA_Audio_Buffer_RX_Q_tmp[0], (uint32_t *)&FPGA_Audio_Buffer_RX_I_tmp[0], size);
}

// FM demodulator
static void DemodulateFM(uint16_t size)
{
	float32_t *lpf_prev = &DFM_RX_lpf_prev;
	float32_t *hpf_prev_a = &DFM_RX_hpf_prev_a;
	float32_t *hpf_prev_b = &DFM_RX_hpf_prev_b;
	float32_t *i_prev = &DFM_RX_i_prev;
	float32_t *q_prev = &DFM_RX_q_prev;
	uint_fast8_t *fm_sql_count = &DFM_RX_fm_sql_count;
	float32_t *FPGA_Audio_Buffer_I_tmp = &FPGA_Audio_Buffer_RX_I_tmp[0];
	float32_t *FPGA_Audio_Buffer_Q_tmp = &FPGA_Audio_Buffer_RX_Q_tmp[0];
	float32_t *fm_sql_avg = &DFM_RX_fm_sql_avg;
	arm_biquad_cascade_df2T_instance_f32 *iir_filter_inst = &IIR_RX_Squelch_HPF;
	bool *squelched = &DFM_RX_Squelched;

	float32_t angle, x, y, a, b;
	static float32_t squelch_buf[FPGA_RX_IQ_BUFFER_HALF_SIZE];

	for (uint_fast16_t i = 0; i < size; i++)
	{
		// first, calculate "x" and "y" for the arctan2, comparing the vectors of present data with previous data
		y = (FPGA_Audio_Buffer_Q_tmp[i] * *i_prev) - (FPGA_Audio_Buffer_I_tmp[i] * *q_prev);
		x = (FPGA_Audio_Buffer_I_tmp[i] * *i_prev) + (FPGA_Audio_Buffer_Q_tmp[i] * *q_prev);
		angle = atan2f(y, x);

		// we now have our audio in "angle"
		squelch_buf[i] = angle; // save audio in "d" buffer for squelch noise filtering/detection - done later

		a = *lpf_prev + (FM_RX_LPF_ALPHA * (angle - *lpf_prev)); //
		*lpf_prev = a;											 // save "[n-1]" sample for next iteration

		*q_prev = FPGA_Audio_Buffer_Q_tmp[i]; // save "previous" value of each channel to allow detection of the change of angle in next go-around
		*i_prev = FPGA_Audio_Buffer_I_tmp[i];

		if ((!*squelched) || (!TRX.FM_SQL_threshold)) // high-pass audio only if we are un-squelched (to save processor time)
		{
			if (CurrentVFO()->Mode == TRX_MODE_WFM)
			{
				FPGA_Audio_Buffer_I_tmp[i] = (float32_t)(angle / PI) * 0.1f; //second way
			}
			else
			{
				b = FM_RX_HPF_ALPHA * (*hpf_prev_b + a - *hpf_prev_a); // do differentiation
				*hpf_prev_a = a;									   // save "[n-1]" samples for next iteration
				*hpf_prev_b = b;
				FPGA_Audio_Buffer_I_tmp[i] = b * 0.1f; // save demodulated and filtered audio in main audio processing buffer
			}
		}
		else if (*squelched)				// were we squelched or tone NOT detected?
			FPGA_Audio_Buffer_I_tmp[i] = 0; // do not filter receive audio - fill buffer with zeroes to mute it
	}

	// *** Squelch Processing ***
	arm_biquad_cascade_df2T_f32(iir_filter_inst, squelch_buf, squelch_buf, size);									   // Do IIR high-pass filter on audio so we may detect squelch noise energy
	*fm_sql_avg = ((1.0f - FM_RX_SQL_SMOOTHING) * *fm_sql_avg) + (FM_RX_SQL_SMOOTHING * sqrtf(fabsf(squelch_buf[0]))); // IIR filter squelch energy magnitude:  We need look at only one representative sample

	*fm_sql_count = *fm_sql_count + 1; // bump count that controls how often the squelch threshold is checked
	if (*fm_sql_count >= FM_SQUELCH_PROC_DECIMATION)
		*fm_sql_count = 0; // enforce the count limit

	// Determine if the (averaged) energy in "ads.fm_sql_avg" is above or below the squelch threshold
	if (*fm_sql_count == 0) // do the squelch threshold calculation much less often than we are called to process this audio
	{
		if (*fm_sql_avg > 0.7f) // limit maximum noise value in averaging to keep it from going out into the weeds under no-signal conditions (higher = noisier)
			*fm_sql_avg = 0.7f;
		b = *fm_sql_avg * 10.0f; // scale noise amplitude to range of squelch setting

		// Now evaluate noise power with respect to squelch setting
		if (!TRX.FM_SQL_threshold) // is squelch set to zero?
			*squelched = false;	   // yes, the we are un-squelched
		else if (*squelched)	   // are we squelched?
		{
			if (b <= (float)((10 - TRX.FM_SQL_threshold) - FM_SQUELCH_HYSTERESIS)) // yes - is average above threshold plus hysteresis?
				*squelched = false;												   //  yes, open the squelch
		}
		else // is the squelch open (e.g. passing audio)?
		{
			if ((10.0f - TRX.FM_SQL_threshold) > FM_SQUELCH_HYSTERESIS) // is setting higher than hysteresis?
			{
				if (b > (float)((10 - TRX.FM_SQL_threshold) + FM_SQUELCH_HYSTERESIS)) // yes - is average below threshold minus hysteresis?
					*squelched = true;												  // yes, close the squelch
			}
			else // setting is lower than hysteresis so we can't use it!
			{
				if (b > (10.0f - (float)TRX.FM_SQL_threshold)) // yes - is average below threshold?
					*squelched = true;						   // yes, close the squelch
			}
		}
	}
}

// FM modulator
static void ModulateFM(uint16_t size)
{
	static float32_t modulation = (float32_t)TRX_SAMPLERATE;
	static float32_t hpf_prev_a = 0;
	static float32_t hpf_prev_b = 0;
	static float32_t sin_data = 0;
	static float32_t fm_mod_accum = 0;
	static float32_t modulation_index = 15000.0f;
	if (CurrentVFO()->TX_LPF_Filter_Width == 5000)
		modulation_index = 4000.0f;
	if (CurrentVFO()->TX_LPF_Filter_Width == 6000)
		modulation_index = 6000.0f;
	if (CurrentVFO()->TX_LPF_Filter_Width == 7000)
		modulation_index = 8000.0f;
	if (CurrentVFO()->TX_LPF_Filter_Width == 8000)
		modulation_index = 11000.0f;
	if (CurrentVFO()->TX_LPF_Filter_Width == 9000)
		modulation_index = 13000.0f;
	if (CurrentVFO()->TX_LPF_Filter_Width == 10000)
		modulation_index = 15000.0f;
	if (CurrentVFO()->TX_LPF_Filter_Width == 15000)
		modulation_index = 30000.0f;
	if (CurrentVFO()->TX_LPF_Filter_Width == 20000)
		modulation_index = 40000.0f;
	if (CurrentVFO()->TX_LPF_Filter_Width == 0)
		modulation_index = 45000.0f;
	
	// Do differentiating high-pass filter to provide 6dB/octave pre-emphasis - which also removes any DC component!
	float32_t ampl = (Processor_selected_RFpower_amplitude / 100.0f * TUNE_POWER);
	for (uint_fast16_t i = 0; i < size; i++)
	{
		hpf_prev_b = FM_TX_HPF_ALPHA * (hpf_prev_b +FPGA_Audio_Buffer_TX_I_tmp[i] - hpf_prev_a); // do differentiation
		hpf_prev_a = FPGA_Audio_Buffer_TX_I_tmp[i];												  // save "[n-1] samples for next iteration
		fm_mod_accum = (1.0f - 0.999f) * fm_mod_accum + 0.999f * hpf_prev_b;						  // save differentiated data in audio buffer // change frequency using scaled audio
		while(fm_mod_accum > modulation) fm_mod_accum -= modulation; // limit range
		while(fm_mod_accum < -modulation) fm_mod_accum += modulation; // limit range
		sin_data = ((fm_mod_accum * modulation_index) / modulation) * PI;
		FPGA_Audio_Buffer_TX_I_tmp[i] = ampl * arm_sin_f32(sin_data);
		FPGA_Audio_Buffer_TX_Q_tmp[i] = ampl * arm_cos_f32(sin_data);
	}
}

// Apply IF Gain IF Gain
static void doRX_IFGain(uint16_t size)
{
	float32_t if_gain = db2rateV(TRX.IF_Gain);

	//apply gain
	arm_scale_f32(FPGA_Audio_Buffer_RX_I_tmp, if_gain, FPGA_Audio_Buffer_RX_I_tmp, size);
	arm_scale_f32(FPGA_Audio_Buffer_RX_Q_tmp, if_gain, FPGA_Audio_Buffer_RX_Q_tmp, size);
	current_if_gain = if_gain;
}
