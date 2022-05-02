#ifndef FPGA_h
#define FPGA_h

#include "stm32f4xx_hal.h"
#include <stdbool.h>
#include "fft.h"
#include "audio_processor.h"
#include "settings.h"

#define FPGA_flash_size 0x200000
#define FPGA_flash_file_offset (0xA0 - 1)
#define FPGA_sector_size (64 * 1024)
#define FPGA_page_size 256
#define FPGA_FLASH_COMMAND_DELAY               \
    for (uint32_t wait = 0; wait < 50; wait++) \
        __asm("nop"); //50
#define FPGA_FLASH_WRITE_DELAY                  \
    for (uint32_t wait = 0; wait < 500; wait++) \
        __asm("nop"); //500
#define FPGA_FLASH_READ_DELAY                  \
    for (uint32_t wait = 0; wait < 50; wait++) \
        __asm("nop"); //50

#define FPGA_writePacket(value) (FPGA_BUS_D0_GPIO_Port->BSRR = (value) | 0xFF0000)
#define FPGA_readPacket (FPGA_BUS_D0_GPIO_Port->IDR & 0xFF)

//Micron M25P80 Serial Flash COMMANDS:
#define M25P80_WRITE_ENABLE 0x06
#define M25P80_WRITE_DISABLE 0x04
#define M25P80_READ_IDENTIFICATION 0x9F
#define M25P80_READ_IDENTIFICATION2 0x9E
#define M25P80_READ_STATUS_REGISTER 0x05
#define M25P80_WRITE_STATUS_REGISTER 0x01
#define M25P80_READ_DATA_BYTES 0x03
#define M25P80_READ_DATA_BYTES_at_HIGHER_SPEED 0x0B
#define M25P80_PAGE_PROGRAM 0x02
#define M25P80_SECTOR_ERASE 0xD8
#define M25P80_BULK_ERASE 0xC7
#define M25P80_DEEP_POWER_DOWN 0xB9
#define M25P80_RELEASE_from_DEEP_POWER_DOWN 0xAB

//Public variables
extern volatile uint32_t FPGA_samples;                                     // counter of the number of samples when exchanging with FPGA
extern volatile bool FPGA_Buffer_underrun;                                 // flag of lack of data from FPGA
extern volatile bool FPGA_NeedSendParams;                                  // flag of the need to send parameters to FPGA
extern volatile bool FPGA_NeedGetParams;                                   // flag of the need to get parameters from FPGA
extern volatile bool FPGA_NeedRestart;                                     // flag of necessity to restart FPGA modules
extern volatile float32_t FPGA_Audio_Buffer_RX_Q[FPGA_RX_IQ_BUFFER_SIZE]; // FPGA buffers
extern volatile float32_t FPGA_Audio_Buffer_RX_I[FPGA_RX_IQ_BUFFER_SIZE];
extern volatile float32_t FPGA_Audio_SendBuffer_Q[FPGA_TX_IQ_BUFFER_SIZE];
extern volatile float32_t FPGA_Audio_SendBuffer_I[FPGA_TX_IQ_BUFFER_SIZE];
extern uint_fast16_t FPGA_Audio_RXBuffer_Index; // current index in FPGA buffers
extern uint_fast16_t FPGA_Audio_TXBuffer_Index; // current index in FPGA buffers
extern bool FPGA_Audio_Buffer_State;            // buffer state, half or full full true - compleate; false - half

//Public methods
extern void FPGA_Init(void);                // initialize exchange with FPGA
extern void FPGA_fpgadata_iqclock(void);    // exchange IQ data with FPGA
extern void FPGA_fpgadata_stuffclock(void); // exchange parameters with FPGA
extern void FPGA_restart(void);             // restart FPGA modules
extern uint8_t ADCDAC_OVR_StatusLatency;

#endif
