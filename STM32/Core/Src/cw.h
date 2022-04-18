#ifndef CW_H
#define CW_H

#include "stm32f4xx_hal.h"
#include <stdbool.h>
#include "settings.h"

extern void CW_key_change(void);
extern float32_t CW_GenerateSignal(float32_t power);

volatile extern bool CW_key_serial;
volatile extern bool CW_old_key_serial;
volatile extern bool CW_key_dot_hard;
volatile extern bool CW_key_dash_hard;
volatile extern uint_fast16_t CW_Key_Timeout_est;
volatile extern uint_fast8_t KEYER_symbol_status;

#endif
