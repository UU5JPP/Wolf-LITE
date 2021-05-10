#ifndef Functions_h
#define Functions_h

#include "stm32f4xx_hal.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "profiler.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-int-conversion"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#include "arm_math.h"
#pragma GCC diagnostic pop

#define TRX_MODE_LSB 0
#define TRX_MODE_USB 1
#define TRX_MODE_CW_L 2
#define TRX_MODE_CW_U 3
#define TRX_MODE_NFM 4
#define TRX_MODE_WFM 5
#define TRX_MODE_AM 6
#define TRX_MODE_DIGI_L 7
#define TRX_MODE_DIGI_U 8
#define TRX_MODE_IQ 9
#define TRX_MODE_LOOPBACK 10
#define TRX_MODE_NO_TX 11
#define TRX_MODE_COUNT 12

#define IRAM1 __attribute__((section(".IRAM1"))) // 128kb IRAM1
#define IRAM2 __attribute__((section(".IRAM2"))) // 64kb IRAM2
#define BACKUP_SRAM_BANK1_ADDR (uint32_t *)(BKPSRAM_BASE)
#define BACKUP_SRAM_BANK2_ADDR (uint32_t *)(BKPSRAM_BASE+0x800) // 4kb Backup SRAM

//UINT from BINARY STRING
#define HEX__(n) 0x##n##LU
#define B8__(x) ((x & 0x0000000FLU) ? 1 : 0) + ((x & 0x000000F0LU) ? 2 : 0) + ((x & 0x00000F00LU) ? 4 : 0) + ((x & 0x0000F000LU) ? 8 : 0) + ((x & 0x000F0000LU) ? 16 : 0) + ((x & 0x00F00000LU) ? 32 : 0) + ((x & 0x0F000000LU) ? 64 : 0) + ((x & 0xF0000000LU) ? 128 : 0)
#define B8(d) ((unsigned char)B8__(HEX__(d)))

#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define bitWrite(value, bit, bitvalue) (bitvalue ? bitSet(value, bit) : bitClear(value, bit))

#ifdef __clang__
 #define isnanf __ARM_isnanf
 #define isinff __ARM_isinff
#endif

#define F_PI 3.141592653589793238463f
#define SQRT2 1.41421356237f
#define ARRLENTH(x) (sizeof(x) / sizeof((x)[0]))
#define MINI_DELAY                                       \
  for (uint_fast16_t wait_i = 0; wait_i < 100; wait_i++) \
    __asm("nop");

// Example of __DATE__ string: "Jul 27 2012"
//                              01234567890
#define BUILD_YEAR_CH0 (__DATE__[7])
#define BUILD_YEAR_CH1 (__DATE__[8])
#define BUILD_YEAR_CH2 (__DATE__[9])
#define BUILD_YEAR_CH3 (__DATE__[10])
#define BUILD_MONTH_IS_JAN (__DATE__[0] == 'J' && __DATE__[1] == 'a' && __DATE__[2] == 'n')
#define BUILD_MONTH_IS_FEB (__DATE__[0] == 'F')
#define BUILD_MONTH_IS_MAR (__DATE__[0] == 'M' && __DATE__[1] == 'a' && __DATE__[2] == 'r')
#define BUILD_MONTH_IS_APR (__DATE__[0] == 'A' && __DATE__[1] == 'p')
#define BUILD_MONTH_IS_MAY (__DATE__[0] == 'M' && __DATE__[1] == 'a' && __DATE__[2] == 'y')
#define BUILD_MONTH_IS_JUN (__DATE__[0] == 'J' && __DATE__[1] == 'u' && __DATE__[2] == 'n')
#define BUILD_MONTH_IS_JUL (__DATE__[0] == 'J' && __DATE__[1] == 'u' && __DATE__[2] == 'l')
#define BUILD_MONTH_IS_AUG (__DATE__[0] == 'A' && __DATE__[1] == 'u')
#define BUILD_MONTH_IS_SEP (__DATE__[0] == 'S')
#define BUILD_MONTH_IS_OCT (__DATE__[0] == 'O')
#define BUILD_MONTH_IS_NOV (__DATE__[0] == 'N')
#define BUILD_MONTH_IS_DEC (__DATE__[0] == 'D')
#define BUILD_MONTH_CH0 \
  ((BUILD_MONTH_IS_OCT || BUILD_MONTH_IS_NOV || BUILD_MONTH_IS_DEC) ? '1' : '0')
#define BUILD_MONTH_CH1 \
  (                     \
      (BUILD_MONTH_IS_JAN) ? '1' : (BUILD_MONTH_IS_FEB) ? '2' : (BUILD_MONTH_IS_MAR) ? '3' : (BUILD_MONTH_IS_APR) ? '4' : (BUILD_MONTH_IS_MAY) ? '5' : (BUILD_MONTH_IS_JUN) ? '6' : (BUILD_MONTH_IS_JUL) ? '7' : (BUILD_MONTH_IS_AUG) ? '8' : (BUILD_MONTH_IS_SEP) ? '9' : (BUILD_MONTH_IS_OCT) ? '0' : (BUILD_MONTH_IS_NOV) ? '1' : (BUILD_MONTH_IS_DEC) ? '2' : /* error default */ '?')
#define BUILD_DAY_CH0 (__DATE__[4] == ' ' ? '0' : __DATE__[4] )
#define BUILD_DAY_CH1 (__DATE__[5])

// Example of __TIME__ string: "21:06:19"
//                              01234567
#define BUILD_HOUR_CH0 (__TIME__[0])
#define BUILD_HOUR_CH1 (__TIME__[1])
#define BUILD_MIN_CH0 (__TIME__[3])
#define BUILD_MIN_CH1 (__TIME__[4])
#define BUILD_SEC_CH0 (__TIME__[6])
#define BUILD_SEC_CH1 (__TIME__[7])

typedef struct
{
  float32_t Load; /*!< CPU load percentage */
  uint32_t WCNT;  /*!< Number of working cycles in one period. Meant for private use */
  uint32_t SCNT;  /*!< Number of sleeping cycles in one period. Meant for private use */
  uint32_t SINC;
} CPULOAD_t;

extern CPULOAD_t CPU_LOAD;
volatile extern bool SPI_process;

extern void CPULOAD_Init(void);
extern void CPULOAD_GoToSleepMode(void);
extern void CPULOAD_WakeUp(void);
extern void CPULOAD_Calc(void);
extern uint32_t getRXPhraseFromFrequency(int32_t freq);
extern uint32_t getTXPhraseFromFrequency(int32_t freq);
extern void addSymbols(char *dest, char *str, uint_fast8_t length, char *symbol, bool toEnd);
extern void sendToDebug_str(char *str);
extern void sendToDebug_strln(char *data);
extern void sendToDebug_str2(char *data1, char *data2);
extern void sendToDebug_str3(char *data1, char *data2, char *data3);
extern void sendToDebug_newline(void);
extern void sendToDebug_flush(void);
extern void sendToDebug_uint8(uint8_t data, bool _inline);
extern void sendToDebug_uint16(uint16_t data, bool _inline);
extern void sendToDebug_uint32(uint32_t data, bool _inline);
extern void sendToDebug_int8(int8_t data, bool _inline);
extern void sendToDebug_int16(int16_t data, bool _inline);
extern void sendToDebug_int32(int32_t data, bool _inline);
extern void sendToDebug_float32(float32_t data, bool _inline);
extern void sendToDebug_hex(uint8_t data, bool _inline);
//extern void delay_us(uint32_t us);
extern float32_t log10f_fast(float32_t X);
extern void readFromCircleBuffer32(uint32_t *source, uint32_t *dest, uint32_t index, uint32_t length, uint32_t words_to_read);
extern void readHalfFromCircleUSBBuffer24Bit(uint8_t *source, int32_t *dest, uint32_t index, uint32_t length);
extern void readHalfFromCircleUSBBuffer16Bit(uint8_t *source, int32_t *dest, uint32_t index, uint32_t length);
extern void dma_memcpy32(uint32_t *dest, uint32_t *src, uint32_t len);
extern float32_t db2rateV(float32_t i);
extern float32_t db2rateP(float32_t i);
extern float32_t rate2dbV(float32_t i);
extern float32_t rate2dbP(float32_t i);
extern float32_t volume2rate(float32_t i);
extern void shiftTextLeft(char *string, uint_fast16_t shiftLength);
extern float32_t getMaxTXAmplitudeOnFreq(uint32_t freq);
//extern uint16_t getf_calibrate(uint16_t freq);
extern float32_t generateSin(float32_t amplitude, uint32_t index, uint32_t samplerate, uint32_t freq);
extern int32_t convertToSPIBigEndian(int32_t in);
extern uint8_t rev8(uint8_t data);
extern bool SPI_Transmit(uint8_t *out_data, uint8_t *in_data, uint16_t count, GPIO_TypeDef *CS_PORT, uint16_t CS_PIN, bool hold_cs, uint32_t prescaler);

#endif
