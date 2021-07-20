#include "fpga.h"
#include "main.h"
#include "trx_manager.h"
#include "rf_unit.h"

// Public variables
volatile uint32_t FPGA_samples = 0;										  // counter of the number of samples when exchanging with FPGA
volatile bool FPGA_NeedSendParams = false;								  // flag of the need to send parameters to FPGA
volatile bool FPGA_NeedGetParams = false;								  // flag of the need to get parameters from FPGA
volatile bool FPGA_NeedRestart = true;									  // flag of necessity to restart FPGA modules
volatile bool FPGA_Buffer_underrun = false;								  // flag of lack of data from FPGA
uint_fast16_t FPGA_Audio_RXBuffer_Index = 0;							  // current index in FPGA buffers
uint_fast16_t FPGA_Audio_TXBuffer_Index = 0;							  // current index in FPGA buffers
bool FPGA_Audio_Buffer_State = true;									  // buffer state, half or full full true - compleate; false - half
IRAM1 volatile float32_t FPGA_Audio_Buffer_RX_Q[FPGA_RX_IQ_BUFFER_SIZE] = {0}; // FPGA buffers
IRAM1 volatile float32_t FPGA_Audio_Buffer_RX_I[FPGA_RX_IQ_BUFFER_SIZE] = {0};
IRAM1 volatile float32_t FPGA_Audio_SendBuffer_Q[FPGA_TX_IQ_BUFFER_SIZE] = {0};
IRAM1 volatile float32_t FPGA_Audio_SendBuffer_I[FPGA_TX_IQ_BUFFER_SIZE] = {0};

// Private variables
static GPIO_InitTypeDef FPGA_GPIO_InitStruct; // structure of GPIO ports
static bool FPGA_bus_stop = false;			  // suspend the FPGA bus

// Prototypes
static inline void FPGA_clockFall(void);			// remove CLK signal
static inline void FPGA_clockRise(void);			// raise the CLK signal
static inline void FPGA_syncAndClockRiseFall(void); // raise CLK and SYNC signals, then release
static void FPGA_fpgadata_sendiq(void);				// send IQ data
static void FPGA_fpgadata_getiq(void);				// get IQ data
static void FPGA_fpgadata_getparam(void);			// get parameters
static void FPGA_fpgadata_sendparam(void);			// send parameters
static void FPGA_setBusInput(void);					// switch the bus to input
static void FPGA_setBusOutput(void);				// switch bus to pin

// initialize exchange with FPGA
void FPGA_Init(void)
{
	FPGA_GPIO_InitStruct.Pin = FPGA_BUS_D0_Pin | FPGA_BUS_D1_Pin | FPGA_BUS_D2_Pin | FPGA_BUS_D3_Pin | FPGA_BUS_D4_Pin | FPGA_BUS_D5_Pin | FPGA_BUS_D6_Pin | FPGA_BUS_D7_Pin;
	FPGA_GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	FPGA_GPIO_InitStruct.Pull = GPIO_PULLUP;
	FPGA_GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(FPGA_BUS_D0_GPIO_Port, &FPGA_GPIO_InitStruct);

	FPGA_GPIO_InitStruct.Pin = FPGA_CLK_Pin | FPGA_SYNC_Pin;
	FPGA_GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	FPGA_GPIO_InitStruct.Pull = GPIO_PULLUP;
	FPGA_GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(FPGA_CLK_GPIO_Port, &FPGA_GPIO_InitStruct);
	
	FPGA_bus_stop = true;
	FPGA_setBusOutput();
	FPGA_writePacket(5); // RESET ON
	FPGA_syncAndClockRiseFall();
	HAL_Delay(100);
	FPGA_writePacket(6); // RESET OFF
	FPGA_syncAndClockRiseFall();
	FPGA_NeedRestart = false;
	FPGA_bus_stop = false;
	
	//test bus
	/*FPGA_setBusOutput();
	FPGA_writePacket(0);
	FPGA_syncAndClockRiseFall();
	
	for(uint8_t i = 0; i<8; i++)
	{
		FPGA_writePacket(1 << i);
		FPGA_clockRise();
		FPGA_writePacket(0);
		FPGA_clockFall();
		sendToDebug_uint8(FPGA_readPacket,false);
	}*/
}

// restart FPGA modules
void FPGA_restart(void) // restart FPGA modules
{
	static bool FPGA_restart_state = false;
	if(!FPGA_restart_state)
	{
		FPGA_setBusOutput();
		FPGA_writePacket(5); // RESET ON
		FPGA_syncAndClockRiseFall();
	}
	else
	{
		FPGA_writePacket(6); // RESET OFF
		FPGA_syncAndClockRiseFall();
		FPGA_NeedRestart = false;
	}
	FPGA_restart_state = !FPGA_restart_state;
}

// exchange parameters with FPGA
void FPGA_fpgadata_stuffclock(void)
{
	if (!FPGA_NeedSendParams && !FPGA_NeedGetParams && !FPGA_NeedRestart)
		return;
	if (FPGA_bus_stop)
		return;
	uint8_t FPGA_fpgadata_out_tmp8 = 0;
	//data exchange

	//STAGE 1
	//out
	if (FPGA_NeedSendParams) //send params
		FPGA_fpgadata_out_tmp8 = 1;
	else //get params
		FPGA_fpgadata_out_tmp8 = 2;

	FPGA_setBusOutput();
	FPGA_writePacket(FPGA_fpgadata_out_tmp8);
	FPGA_syncAndClockRiseFall();

	if (FPGA_NeedSendParams)
	{
		FPGA_fpgadata_sendparam();
		FPGA_NeedSendParams = false;
	}
	else if (FPGA_NeedGetParams)
	{
		FPGA_fpgadata_getparam();
		FPGA_NeedGetParams = false;
	}
	else if (FPGA_NeedRestart)
	{
		FPGA_restart();
	}
}

// exchange IQ data with FPGA
void FPGA_fpgadata_iqclock(void)
{
	if (FPGA_bus_stop)
		return;
	uint8_t FPGA_fpgadata_out_tmp8 = 4; //RX
	VFO *current_vfo = CurrentVFO();
	if (current_vfo->Mode == TRX_MODE_LOOPBACK)
		return;
	//data exchange

	//STAGE 1
	//out
	if (TRX_on_TX())
		FPGA_fpgadata_out_tmp8 = 3; //TX

	FPGA_setBusOutput();
	FPGA_writePacket(FPGA_fpgadata_out_tmp8);
	FPGA_syncAndClockRiseFall();

	if (TRX_on_TX())
		FPGA_fpgadata_sendiq();
	else
		FPGA_fpgadata_getiq();
}

// send parameters
static inline void FPGA_fpgadata_sendparam(void)
{
	uint8_t FPGA_fpgadata_out_tmp8 = 0;
	VFO *current_vfo = CurrentVFO();

	//STAGE 2
	//out PTT+PREAMP
	uint8_t att_val = (uint8_t)TRX.ATT_DB;
	bitWrite(FPGA_fpgadata_out_tmp8, 0, (!TRX_on_TX() && current_vfo->Mode != TRX_MODE_LOOPBACK)); //RX
	bitWrite(FPGA_fpgadata_out_tmp8, 1, TRX.ADC_Driver);
	if(TRX.ATT)
	{
		bitWrite(FPGA_fpgadata_out_tmp8, 2, 0); //att0.5
		bitWrite(FPGA_fpgadata_out_tmp8, 3, (att_val >> 0) & 0x1); //att1
		bitWrite(FPGA_fpgadata_out_tmp8, 4, (att_val >> 1) & 0x1); //att2
		bitWrite(FPGA_fpgadata_out_tmp8, 5, (att_val >> 2) & 0x1); //att4
		bitWrite(FPGA_fpgadata_out_tmp8, 6, (att_val >> 3) & 0x1); //att8
		bitWrite(FPGA_fpgadata_out_tmp8, 7, (att_val >> 4) & 0x1); //att16
	}
	FPGA_writePacket(FPGA_fpgadata_out_tmp8);
	FPGA_clockRise();
	FPGA_clockFall();

	//STAGE 3
	//out RX1-FREQ
	FPGA_writePacket(((TRX_freq_phrase & (0XFF << 16)) >> 16));
	FPGA_clockRise();
	FPGA_clockFall();

	//STAGE 4
	//OUT RX1-FREQ
	FPGA_writePacket(((TRX_freq_phrase & (0XFF << 8)) >> 8));
	FPGA_clockRise();
	FPGA_clockFall();

	//STAGE 5
	//OUT RX1-FREQ
	FPGA_writePacket(TRX_freq_phrase & 0XFF);
	FPGA_clockRise();
	FPGA_clockFall();

	//STAGE 9
	//OUT CIC-GAIN
	FPGA_writePacket(0);
	FPGA_clockRise();
	FPGA_clockFall();

	//STAGE 10
	//OUT CICCOMP-GAIN
	FPGA_writePacket(CALIBRATE.CICFIR_GAINER_val);
	FPGA_clockRise();
	FPGA_clockFall();

	//STAGE 11
	//OUT TX-CICCOMP-GAIN
	FPGA_writePacket(CALIBRATE.TXCICFIR_GAINER_val);
	FPGA_clockRise();
	FPGA_clockFall();

	//STAGE 12
	//OUT DAC-GAIN
	FPGA_writePacket(CALIBRATE.DAC_GAINER_val);
	FPGA_clockRise();
	FPGA_clockFall();

	//STAGE 13
	//out TX-FREQ
	FPGA_writePacket(((TRX_freq_phrase_tx & (0XFF << 16)) >> 16));
	FPGA_clockRise();
	FPGA_clockFall();

	//STAGE 14
	//OUT TX-FREQ
	FPGA_writePacket(((TRX_freq_phrase_tx & (0XFF << 8)) >> 8));
	FPGA_clockRise();
	FPGA_clockFall();

	//STAGE 15
	//OUT TX-FREQ
	FPGA_writePacket(TRX_freq_phrase_tx & 0XFF);
	FPGA_clockRise();
	FPGA_clockFall();
	
	//STAGE 16
	//out BPF
	FPGA_fpgadata_out_tmp8 = 0;
	if(CurrentVFO()->Freq >= 1500000 && CurrentVFO()->Freq <= 2400000) //160m
	{
		bitWrite(FPGA_fpgadata_out_tmp8, 4, 0); //LPF1
		bitWrite(FPGA_fpgadata_out_tmp8, 5, 0); //LPF2
		bitWrite(FPGA_fpgadata_out_tmp8, 6, 0); //LPF3
	}
	else if(CurrentVFO()->Freq >= 2400000 && CurrentVFO()->Freq <= 4500000) //80m
	{
		bitWrite(FPGA_fpgadata_out_tmp8, 4, 1); //LPF1
		bitWrite(FPGA_fpgadata_out_tmp8, 5, 0); //LPF2
		bitWrite(FPGA_fpgadata_out_tmp8, 6, 0); //LPF3
	}
	else if(CurrentVFO()->Freq >= 4500000 && CurrentVFO()->Freq <= 7500000) //40m
	{
		bitWrite(FPGA_fpgadata_out_tmp8, 4, 0); //LPF1
		bitWrite(FPGA_fpgadata_out_tmp8, 5, 1); //LPF2
		bitWrite(FPGA_fpgadata_out_tmp8, 6, 0); //LPF3
	}
	else if(CurrentVFO()->Freq >= 7500000 && CurrentVFO()->Freq <= 14800000) //30m,20m
	{
		bitWrite(FPGA_fpgadata_out_tmp8, 4, 1); //LPF1
		bitWrite(FPGA_fpgadata_out_tmp8, 5, 1); //LPF2
		bitWrite(FPGA_fpgadata_out_tmp8, 6, 0); //LPF3
	}
	else if(CurrentVFO()->Freq >= 12000000 && CurrentVFO()->Freq <= 32000000) //17,15m,12,10m
	{
		bitWrite(FPGA_fpgadata_out_tmp8, 4, 0); //LPF1
		bitWrite(FPGA_fpgadata_out_tmp8, 5, 0); //LPF2
		bitWrite(FPGA_fpgadata_out_tmp8, 6, 1); //LPF3
	}
	
	
	//out LPF
	if(CurrentVFO()->Freq >= 1500000 && CurrentVFO()->Freq <= 2400000) //160m
	{
		bitWrite(FPGA_fpgadata_out_tmp8, 0, 0); //BPF_A
		bitWrite(FPGA_fpgadata_out_tmp8, 1, 0); //BPF_B
		bitWrite(FPGA_fpgadata_out_tmp8, 3, 1); //BPF_OE1
		bitWrite(FPGA_fpgadata_out_tmp8, 2, 0); //BPF_OE2
	}
	else if(CurrentVFO()->Freq >= 2400000 && CurrentVFO()->Freq <= 4500000) //80m
	{
		bitWrite(FPGA_fpgadata_out_tmp8, 0, 1); //BPF_A
		bitWrite(FPGA_fpgadata_out_tmp8, 1, 0); //BPF_B
		bitWrite(FPGA_fpgadata_out_tmp8, 3, 1); //BPF_OE1
		bitWrite(FPGA_fpgadata_out_tmp8, 2, 0); //BPF_OE2
	}
	else if(CurrentVFO()->Freq >= 4500000 && CurrentVFO()->Freq <= 7500000) //40m
	{
		bitWrite(FPGA_fpgadata_out_tmp8, 0, 0); //BPF_A
		bitWrite(FPGA_fpgadata_out_tmp8, 1, 1); //BPF_B
		bitWrite(FPGA_fpgadata_out_tmp8, 3, 1); //BPF_OE1
		bitWrite(FPGA_fpgadata_out_tmp8, 2, 0); //BPF_OE2
	}
	else if(CurrentVFO()->Freq >= 7500000 && CurrentVFO()->Freq <= 12000000) //30m
	{
		bitWrite(FPGA_fpgadata_out_tmp8, 0, 1); //BPF_A
		bitWrite(FPGA_fpgadata_out_tmp8, 1, 1); //BPF_B
		bitWrite(FPGA_fpgadata_out_tmp8, 3, 1); //BPF_OE1
		bitWrite(FPGA_fpgadata_out_tmp8, 2, 0); //BPF_OE2
	}
	else if(CurrentVFO()->Freq >= 12000000 && CurrentVFO()->Freq <= 14800000) //20m
	{
		bitWrite(FPGA_fpgadata_out_tmp8, 0, 0); //BPF_A
		bitWrite(FPGA_fpgadata_out_tmp8, 1, 0); //BPF_B
		bitWrite(FPGA_fpgadata_out_tmp8, 3, 0); //BPF_OE1
		bitWrite(FPGA_fpgadata_out_tmp8, 2, 1); //BPF_OE2
	}
	else if(CurrentVFO()->Freq >= 14800000 && CurrentVFO()->Freq <= 22000000) //17,15m
	{
		bitWrite(FPGA_fpgadata_out_tmp8, 0, 1); //BPF_A
		bitWrite(FPGA_fpgadata_out_tmp8, 1, 0); //BPF_B
		bitWrite(FPGA_fpgadata_out_tmp8, 3, 0); //BPF_OE1
		bitWrite(FPGA_fpgadata_out_tmp8, 2, 1); //BPF_OE2

	}
	else if(CurrentVFO()->Freq >= 22000000 && CurrentVFO()->Freq <= 32000000) //12,10m
	{
		bitWrite(FPGA_fpgadata_out_tmp8, 0, 0); //BPF_A
		bitWrite(FPGA_fpgadata_out_tmp8, 1, 1); //BPF_B
		bitWrite(FPGA_fpgadata_out_tmp8, 3, 0); //BPF_OE1
		bitWrite(FPGA_fpgadata_out_tmp8, 2, 1); //BPF_OE2
	}
	else //if(CurrentVFO()->Freq >= 0 && CurrentVFO()->Freq <= 53000000) //6m
	{
		bitWrite(FPGA_fpgadata_out_tmp8, 0, 1); //BPF_A
		bitWrite(FPGA_fpgadata_out_tmp8, 1, 1); //BPF_B
		bitWrite(FPGA_fpgadata_out_tmp8, 3, 0); //BPF_OE1
		bitWrite(FPGA_fpgadata_out_tmp8, 2, 1); //BPF_OE2
	}
	//bitWrite(FPGA_fpgadata_out_tmp8, 7, 0); //unused
	FPGA_writePacket(FPGA_fpgadata_out_tmp8);
	FPGA_clockRise();
	FPGA_clockFall();
}

// get parameters
static inline void FPGA_fpgadata_getparam(void)
{
	register uint8_t FPGA_fpgadata_in_tmp8 = 0;
	register int32_t FPGA_fpgadata_in_tmp32 = 0;
	FPGA_setBusInput();

	//STAGE 2
	FPGA_clockRise();
	FPGA_fpgadata_in_tmp8 = FPGA_readPacket;
	TRX_ADC_OTR = bitRead(FPGA_fpgadata_in_tmp8, 0);
	TRX_DAC_OTR = bitRead(FPGA_fpgadata_in_tmp8, 1);
	FPGA_clockFall();

	//STAGE 3
	FPGA_clockRise();
	FPGA_fpgadata_in_tmp8 = FPGA_readPacket;
	FPGA_clockFall();
	//STAGE 4
	FPGA_clockRise();
	TRX_ADC_MINAMPLITUDE = (int16_t)(((FPGA_fpgadata_in_tmp8 << 8) & 0xFF00) | FPGA_readPacket);
	bitWrite(TRX_ADC_MINAMPLITUDE, 15, bitRead(TRX_ADC_MINAMPLITUDE, 11)); //EXTEND 12 To 16 BITS
	bitWrite(TRX_ADC_MINAMPLITUDE, 14, bitRead(TRX_ADC_MINAMPLITUDE, 11));
	bitWrite(TRX_ADC_MINAMPLITUDE, 13, bitRead(TRX_ADC_MINAMPLITUDE, 11));
	bitWrite(TRX_ADC_MINAMPLITUDE, 12, bitRead(TRX_ADC_MINAMPLITUDE, 11));
	FPGA_clockFall();

	//STAGE 5
	FPGA_clockRise();
	FPGA_fpgadata_in_tmp8 = FPGA_readPacket;
	FPGA_clockFall();
	//STAGE 6
	FPGA_clockRise();
	TRX_ADC_MAXAMPLITUDE = (int16_t)(((FPGA_fpgadata_in_tmp8 << 8) & 0xFF00) | FPGA_readPacket);
	bitWrite(TRX_ADC_MAXAMPLITUDE, 15, bitRead(TRX_ADC_MAXAMPLITUDE, 11)); //EXTEND 12 To 16 BITS
	bitWrite(TRX_ADC_MAXAMPLITUDE, 14, bitRead(TRX_ADC_MAXAMPLITUDE, 11));
	bitWrite(TRX_ADC_MAXAMPLITUDE, 13, bitRead(TRX_ADC_MAXAMPLITUDE, 11));
	bitWrite(TRX_ADC_MAXAMPLITUDE, 12, bitRead(TRX_ADC_MAXAMPLITUDE, 11));
	FPGA_clockFall();
}

// get IQ data
static inline void FPGA_fpgadata_getiq(void)
{
	register int16_t FPGA_fpgadata_in_tmp16 = 0;
	float32_t* FFTInput_I_current = FFT_buff_current ? (float32_t*)&FFTInput_I_B : (float32_t*)&FFTInput_I_A;
  float32_t* FFTInput_Q_current = FFT_buff_current ? (float32_t*)&FFTInput_Q_B : (float32_t*)&FFTInput_Q_A;
	
	float32_t FPGA_fpgadata_in_float32 = 0;
	FPGA_samples++;
	FPGA_setBusInput();

	//Q RX
	//STAGE 2
	FPGA_clockRise();
	FPGA_fpgadata_in_tmp16 = (FPGA_readPacket << 8);
	FPGA_clockFall();

	//STAGE 3
	FPGA_clockRise();
	FPGA_fpgadata_in_tmp16 |= (FPGA_readPacket);

	/*static float32_t min = 0;
	static float32_t max = 0;
	if((float32_t)FPGA_fpgadata_in_tmp16 < min)
		min = (float32_t)FPGA_fpgadata_in_tmp16;
	if((float32_t)FPGA_fpgadata_in_tmp16 > max)
		max = (float32_t)FPGA_fpgadata_in_tmp16;
	if(FPGA_samples == 100)
	{
		sendToDebug_float32(min, false);
		sendToDebug_float32(max, false);
		sendToDebug_newline();
		min = 0;
		max = 0;
	}*/
	
	FPGA_fpgadata_in_float32 = (float32_t)FPGA_fpgadata_in_tmp16 / 32768.0f;
	if (TRX_RX_IQ_swap)
	{
		FFTInput_I_current[FFT_buff_index] = FPGA_fpgadata_in_float32;
		FPGA_Audio_Buffer_RX_I[FPGA_Audio_RXBuffer_Index] = FPGA_fpgadata_in_float32;
	}
	else
	{
		FFTInput_Q_current[FFT_buff_index] = FPGA_fpgadata_in_float32;
		FPGA_Audio_Buffer_RX_Q[FPGA_Audio_RXBuffer_Index] = FPGA_fpgadata_in_float32;
	}
	FPGA_clockFall();

	//I RX1
	//STAGE 4
	FPGA_clockRise();
	FPGA_fpgadata_in_tmp16 = (FPGA_readPacket << 8);
	FPGA_clockFall();

	//STAGE 5
	FPGA_clockRise();
	FPGA_fpgadata_in_tmp16 |= (FPGA_readPacket);

	FPGA_fpgadata_in_float32 = (float32_t)FPGA_fpgadata_in_tmp16 / 32768.0f;
	if (TRX_RX_IQ_swap)
	{
		FFTInput_Q_current[FFT_buff_index] = FPGA_fpgadata_in_float32;
		FPGA_Audio_Buffer_RX_Q[FPGA_Audio_RXBuffer_Index] = FPGA_fpgadata_in_float32;
	}
	else
	{
		FFTInput_I_current[FFT_buff_index] = FPGA_fpgadata_in_float32;
		FPGA_Audio_Buffer_RX_I[FPGA_Audio_RXBuffer_Index] = FPGA_fpgadata_in_float32;
	}
	FPGA_clockFall();

	FPGA_Audio_RXBuffer_Index++;
	if (FPGA_Audio_RXBuffer_Index == FPGA_RX_IQ_BUFFER_SIZE)
		FPGA_Audio_RXBuffer_Index = 0;

	FFT_buff_index++;
	if (FFT_buff_index >= FFT_SIZE)
	{
		FFT_buff_index = 0;
		FFT_new_buffer_ready = true;
		FFT_buff_current = !FFT_buff_current;
	}
	FPGA_clockFall();
}

// send IQ data
static inline void FPGA_fpgadata_sendiq(void)
{
	q15_t FPGA_fpgadata_out_q_tmp16 = 0;
	q15_t FPGA_fpgadata_out_i_tmp16 = 0;
	q15_t FPGA_fpgadata_out_tmp_tmp16 = 0;
	arm_float_to_q15((float32_t *)&FPGA_Audio_SendBuffer_Q[FPGA_Audio_TXBuffer_Index], &FPGA_fpgadata_out_q_tmp16, 1);
	arm_float_to_q15((float32_t *)&FPGA_Audio_SendBuffer_I[FPGA_Audio_TXBuffer_Index], &FPGA_fpgadata_out_i_tmp16, 1);
	FPGA_samples++;

	if (TRX_TX_IQ_swap)
	{
		FPGA_fpgadata_out_tmp_tmp16 = FPGA_fpgadata_out_i_tmp16;
		FPGA_fpgadata_out_i_tmp16 = FPGA_fpgadata_out_q_tmp16;
		FPGA_fpgadata_out_q_tmp16 = FPGA_fpgadata_out_tmp_tmp16;
	}

	//out Q
	//STAGE 2
	FPGA_writePacket((FPGA_fpgadata_out_q_tmp16 >> 8) & 0xFF);
	//clock
	FPGA_clockRise();
	//clock
	FPGA_clockFall();

	//STAGE 3
	FPGA_writePacket((FPGA_fpgadata_out_q_tmp16 >> 0) & 0xFF);
	//clock
	FPGA_clockRise();
	//clock
	FPGA_clockFall();

	//out I
	//STAGE 4
	FPGA_writePacket((FPGA_fpgadata_out_i_tmp16 >> 8) & 0xFF);
	//clock
	FPGA_clockRise();
	//clock
	FPGA_clockFall();

	//STAGE 5
	FPGA_writePacket((FPGA_fpgadata_out_i_tmp16 >> 0) & 0xFF);
	//clock
	FPGA_clockRise();
	//clock
	FPGA_clockFall();

	FPGA_Audio_TXBuffer_Index++;
	if (FPGA_Audio_TXBuffer_Index == FPGA_TX_IQ_BUFFER_SIZE)
	{
		if (Processor_NeedTXBuffer)
		{
			FPGA_Buffer_underrun = true;
			FPGA_Audio_TXBuffer_Index--;
		}
		else
		{
			FPGA_Audio_TXBuffer_Index = 0;
			FPGA_Audio_Buffer_State = true;
			Processor_NeedTXBuffer = true;
		}
	}
	else if (FPGA_Audio_TXBuffer_Index == FPGA_TX_IQ_BUFFER_HALF_SIZE)
	{
		if (Processor_NeedTXBuffer)
		{
			FPGA_Buffer_underrun = true;
			FPGA_Audio_TXBuffer_Index--;
		}
		else
		{
			FPGA_Audio_Buffer_State = false;
			Processor_NeedTXBuffer = true;
		}
	}
}

// switch the bus to input
static inline void FPGA_setBusInput(void)
{
	// Configure IO Direction mode (Input)
	/*register uint32_t temp = GPIOA->MODER;
	temp &= ~(GPIO_MODER_MODE0 << (0 * 2U));
	temp |= ((GPIO_MODE_INPUT & 0x00000003U) << (0 * 2U));
	temp &= ~(GPIO_MODER_MODE0 << (1 * 2U));
	temp |= ((GPIO_MODE_INPUT & 0x00000003U) << (1 * 2U));
	temp &= ~(GPIO_MODER_MODE0 << (2 * 2U));
	temp |= ((GPIO_MODE_INPUT & 0x00000003U) << (2 * 2U));
	temp &= ~(GPIO_MODER_MODE0 << (3 * 2U));
	temp |= ((GPIO_MODE_INPUT & 0x00000003U) << (3 * 2U));
	temp &= ~(GPIO_MODER_MODE0 << (4 * 2U));
	temp |= ((GPIO_MODE_INPUT & 0x00000003U) << (4 * 2U));
	temp &= ~(GPIO_MODER_MODE0 << (5 * 2U));
	temp |= ((GPIO_MODE_INPUT & 0x00000003U) << (5 * 2U));
	temp &= ~(GPIO_MODER_MODE0 << (6 * 2U));
	temp |= ((GPIO_MODE_INPUT & 0x00000003U) << (6 * 2U));
	temp &= ~(GPIO_MODER_MODE0 << (7 * 2U));
	temp |= ((GPIO_MODE_INPUT & 0x00000003U) << (7 * 2U));
	sendToDebug_uint32(temp,false);
	GPIOA->MODER = temp;*/
	GPIOA->MODER = -1431764992;
	__asm("nop");__asm("nop");__asm("nop");
}

// switch bus to pin
static inline void FPGA_setBusOutput(void)
{
	// Configure IO Direction mode (Output)
	/*uint32_t temp = GPIOA->MODER;
	temp &= ~(GPIO_MODER_MODE0 << (0 * 2U));
	temp |= ((GPIO_MODE_OUTPUT_PP & 0x00000003U) << (0 * 2U));
	temp &= ~(GPIO_MODER_MODE0 << (1 * 2U));
	temp |= ((GPIO_MODE_OUTPUT_PP & 0x00000003U) << (1 * 2U));
	temp &= ~(GPIO_MODER_MODE0 << (2 * 2U));
	temp |= ((GPIO_MODE_OUTPUT_PP & 0x00000003U) << (2 * 2U));
	temp &= ~(GPIO_MODER_MODE0 << (3 * 2U));
	temp |= ((GPIO_MODE_OUTPUT_PP & 0x00000003U) << (3 * 2U));
	temp &= ~(GPIO_MODER_MODE0 << (4 * 2U));
	temp |= ((GPIO_MODE_OUTPUT_PP & 0x00000003U) << (4 * 2U));
	temp &= ~(GPIO_MODER_MODE0 << (5 * 2U));
	temp |= ((GPIO_MODE_OUTPUT_PP & 0x00000003U) << (5 * 2U));
	temp &= ~(GPIO_MODER_MODE0 << (6 * 2U));
	temp |= ((GPIO_MODE_OUTPUT_PP & 0x00000003U) << (6 * 2U));
	temp &= ~(GPIO_MODER_MODE0 << (7 * 2U));
	temp |= ((GPIO_MODE_OUTPUT_PP & 0x00000003U) << (7 * 2U));
	sendToDebug_uint32(temp,false);
	GPIOA->MODER = temp;*/
	GPIOA->MODER = -1431743147;
	__asm("nop");__asm("nop");__asm("nop");
}

// raise the CLK signal
static inline void FPGA_clockRise(void)
{
	FPGA_CLK_GPIO_Port->BSRR = FPGA_CLK_Pin;
	__asm("nop");__asm("nop");__asm("nop");
}

// remove CLK signal
static inline void FPGA_clockFall(void)
{
	FPGA_CLK_GPIO_Port->BSRR = (FPGA_CLK_Pin << 16U);
	__asm("nop");__asm("nop");__asm("nop");
}

// raise CLK and SYNC signal, then lower
static inline void FPGA_syncAndClockRiseFall(void)
{
	FPGA_CLK_GPIO_Port->BSRR = FPGA_SYNC_Pin;
	FPGA_CLK_GPIO_Port->BSRR = FPGA_CLK_Pin;
	FPGA_CLK_GPIO_Port->BSRR = (FPGA_SYNC_Pin << 16U) | (FPGA_CLK_Pin << 16U);
	__asm("nop");__asm("nop");__asm("nop");
}
