#ifndef TRX_MANAGER_H
#define TRX_MANAGER_H

#include "stm32f4xx_hal.h"
#include <stdbool.h>
#include "settings.h"

extern void TRX_Init(void);
extern void TRX_setFrequency(uint32_t _freq, VFO *vfo);
extern void TRX_setMode(uint_fast8_t _mode, VFO *vfo);
extern void TRX_ptt_change(void);
extern void TRX_key_change(void);
extern bool TRX_on_TX(void);
extern void TRX_DoAutoGain(void);
extern void TRX_Restart_Mode(void);
extern void TRX_DBMCalculate(void);
extern float32_t TRX_GenerateCWSignal(float32_t power);
extern void TRX_TemporaryMute(void);

volatile extern bool TRX_ptt_hard;
volatile extern bool TRX_ptt_soft;
volatile extern bool TRX_old_ptt_soft;
volatile extern bool TRX_RX_IQ_swap;
volatile extern bool TRX_TX_IQ_swap;
volatile extern bool TRX_Tune;
volatile extern bool TRX_Inited;
volatile extern int_fast16_t TRX_RX_dBm;
volatile extern bool TRX_ADC_OTR;
volatile extern bool TRX_DAC_OTR;
volatile extern int16_t TRX_ADC_MINAMPLITUDE;
volatile extern int16_t TRX_ADC_MAXAMPLITUDE;
volatile extern int_fast16_t TRX_SHIFT;
volatile extern float32_t TRX_MAX_TX_Amplitude;
volatile extern float32_t TRX_PWR_Forward;
volatile extern float32_t TRX_PWR_Backward;
volatile extern float32_t TRX_SWR;
volatile extern float32_t TRX_ALC;
volatile extern bool TRX_Mute;
volatile extern bool TRX_IF_Gain;
volatile extern float32_t TRX_IQ_phase_error;
volatile extern bool TRX_NeedGoToBootloader;
volatile extern bool TRX_Temporary_Stop_BandMap;
//volatile extern uint8_t TRX_AutoGain_Stage;
extern const char *MODE_DESCR[];
extern uint32_t TRX_freq_phrase;
extern uint32_t TRX_freq_phrase_tx;
volatile extern uint32_t TRX_Temporary_Mute_StartTime;
extern float32_t TRX_InVoltage;
extern float32_t TRX_SW1_Voltage;
extern float32_t TRX_SW2_Voltage;
extern float32_t TRX_CPU_temperature;
extern float32_t TRX_CPU_VRef;
extern float32_t TRX_CPU_VBat;

#endif
