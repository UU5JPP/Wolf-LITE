#include "usbd_debug_if.h"
#include "usbd_cat_if.h"
#include "trx_manager.h"
#include "lcd_driver.h"

#define DEBUG_APP_RX_DATA_SIZE 8
#define DEBUG_APP_TX_DATA_SIZE 64
#define DEBUG_RX_FIFO_BUFFER_SIZE 128
#define DEBUG_TX_FIFO_BUFFER_SIZE 256

static uint8_t DEBUG_UserRxBufferFS[DEBUG_APP_RX_DATA_SIZE];
static uint8_t DEBUG_UserTxBufferFS[DEBUG_APP_TX_DATA_SIZE];
static uint8_t debug_tx_fifo[DEBUG_TX_FIFO_BUFFER_SIZE] = {0};
static uint16_t debug_tx_fifo_head = 0;
static uint16_t debug_tx_fifo_tail = 0;
static uint8_t lineCoding[7] = {0x00, 0xC2, 0x01, 0x00, 0x00, 0x00, 0x08}; // 115200bps, 1stop, no parity, 8bit

extern USBD_HandleTypeDef hUsbDeviceFS;

static int8_t DEBUG_Init_FS(void);
static int8_t DEBUG_DeInit_FS(void);
static int8_t DEBUG_Control_FS(uint8_t cmd, uint8_t *pbuf, uint32_t len);
static int8_t DEBUG_Receive_FS(uint8_t *pbuf);

USBD_DEBUG_ItfTypeDef USBD_DEBUG_fops_FS =
	{
		DEBUG_Init_FS,
		DEBUG_DeInit_FS,
		DEBUG_Control_FS,
		DEBUG_Receive_FS};

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Initializes the CDC media low layer over the FS USB IP
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t DEBUG_Init_FS(void)
{
	/* USER CODE BEGIN 3 */
	/* Set Application Buffers */
	USBD_DEBUG_SetTxBuffer(&hUsbDeviceFS, DEBUG_UserTxBufferFS, 0);
	USBD_DEBUG_SetRxBuffer(&hUsbDeviceFS, DEBUG_UserRxBufferFS);
	return (USBD_OK);
	/* USER CODE END 3 */
}

/**
  * @brief  DeInitializes the CDC media low layer
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t DEBUG_DeInit_FS(void)
{
	/* USER CODE BEGIN 4 */
	return (USBD_OK);
	/* USER CODE END 4 */
}

/**
  * @brief  Manage the CDC class requests
  * @param  cmd: Command code
  * @param  pbuf: Buffer containing command data (request parameters)
  * @param  length: Number of data to be sent (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t DEBUG_Control_FS(uint8_t cmd, uint8_t *pbuf, uint32_t len)
{
	/* USER CODE BEGIN 5 */

	switch (cmd)
	{
	case CDC_SEND_ENCAPSULATED_COMMAND:
		break;

	case CDC_GET_ENCAPSULATED_RESPONSE:
		break;

	case CDC_SET_COMM_FEATURE:
		break;

	case CDC_GET_COMM_FEATURE:
		break;

	case CDC_CLEAR_COMM_FEATURE:
		break;

		/*******************************************************************************/
		/* Line Coding Structure                                                       */
		/*-----------------------------------------------------------------------------*/
		/* Offset | Field       | Size | Value  | Description                          */
		/* 0      | dwDTERate   |   4  | Number |Data terminal rate, in bits per second*/
		/* 4      | bCharFormat |   1  | Number | Stop bits                            */
		/*                                        0 - 1 Stop bit                       */
		/*                                        1 - 1.5 Stop bits                    */
		/*                                        2 - 2 Stop bits                      */
		/* 5      | bParityType |  1   | Number | Parity                               */
		/*                                        0 - None                             */
		/*                                        1 - Odd                              */
		/*                                        2 - Even                             */
		/*                                        3 - Mark                             */
		/*                                        4 - Space                            */
		/* 6      | bDataBits  |   1   | Number Data bits (5, 6, 7, 8 or 16).          */
		/*******************************************************************************/
	case CDC_SET_LINE_CODING:
		memcpy(lineCoding, pbuf, sizeof(lineCoding));
		break;

	case CDC_GET_LINE_CODING:
		memcpy(pbuf, lineCoding, sizeof(lineCoding));
		break;

	case CDC_SET_CONTROL_LINE_STATE:
		if (pbuf[2] == 1)
			TRX_key_serial = true;
		else
			TRX_key_serial = false;
		break;

	case CDC_SEND_BREAK:
		break;

	default:
		break;
	}

	return (USBD_OK);
	/* USER CODE END 5 */
}

/**
  * @brief  Data received over USB OUT endpoint are sent over CDC interface
  *         through this function.
  *
  *         @note
  *         This function will block any OUT packet reception on USB endpoint
  *         untill exiting this function. If you exit this function before transfer
  *         is complete on CDC interface (ie. using DMA controller) it will result
  *         in receiving more data while previous ones are still not sent.
  *
  * @param  Buf: Buffer of data to be received
  * @param  Len: Number of data received (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t DEBUG_Receive_FS(uint8_t *Buf)
{
	/* USER CODE BEGIN 6 */
	USBD_DEBUG_SetRxBuffer(&hUsbDeviceFS, &Buf[0]);
	USBD_DEBUG_ReceivePacket(&hUsbDeviceFS);
	return (USBD_OK);
	/* USER CODE END 6 */
}

/**
  * @brief  CDC_Transmit_FS
  *         Data to send over USB IN endpoint are sent over CDC interface
  *         through this function.
  *         @note
  *
  *
  * @param  Buf: Buffer of data to be sent
  * @param  Len: Number of data to be sent (in bytes)
  * @retval USBD_OK if all operations are OK else USBD_FAIL or USBD_BUSY
  */
uint8_t DEBUG_Transmit_FS(uint8_t *Buf, uint16_t Len)
{
	uint8_t result = USBD_OK;
	/* USER CODE BEGIN 7 */
	USBD_DEBUG_HandleTypeDef *hcdc = (USBD_DEBUG_HandleTypeDef *)hUsbDeviceFS.pClassDataDEBUG;
	if (hcdc->TxState != 0)
	{
		return USBD_BUSY;
	}
	USBD_DEBUG_SetTxBuffer(&hUsbDeviceFS, Buf, Len);
	result = USBD_DEBUG_TransmitPacket(&hUsbDeviceFS);
	/* USER CODE END 7 */
	return result;
}

void DEBUG_Transmit_FIFO(uint8_t *data, uint16_t length)
{
	if (length <= DEBUG_TX_FIFO_BUFFER_SIZE)
		for (uint16_t i = 0; i < length; i++)
		{
			debug_tx_fifo[debug_tx_fifo_head] = data[i];
			debug_tx_fifo_head++;
			if (debug_tx_fifo_head == debug_tx_fifo_tail)
			{
				uint_fast16_t tryes = 0;
				while (DEBUG_Transmit_FIFO_Events() == USBD_BUSY && tryes < 512)
					tryes++;
				if(DEBUG_Transmit_FIFO_Events() == USBD_BUSY)
					break;
			}
			if (debug_tx_fifo_head >= DEBUG_TX_FIFO_BUFFER_SIZE)
				debug_tx_fifo_head = 0;
		}
}

static uint8_t temp_buff[DEBUG_TX_FIFO_BUFFER_SIZE] = {0};
static bool FIFO_Events_busy = false;
uint8_t DEBUG_Transmit_FIFO_Events(void)
{
	if (FIFO_Events_busy)
		return USBD_FAIL;
	if (debug_tx_fifo_head == debug_tx_fifo_tail)
		return USBD_OK;
	USBD_DEBUG_HandleTypeDef *hcdc = (USBD_DEBUG_HandleTypeDef *)hUsbDeviceFS.pClassDataDEBUG;
	if (hcdc->TxState != 0)
		return USBD_FAIL;
	FIFO_Events_busy = true;
	uint16_t indx = 0;
	memset(temp_buff, 0x00, DEBUG_TX_FIFO_BUFFER_SIZE);

	if (debug_tx_fifo_tail > debug_tx_fifo_head)
	{
		for (uint16_t i = debug_tx_fifo_tail; i < DEBUG_TX_FIFO_BUFFER_SIZE; i++)
		{
			temp_buff[indx] = debug_tx_fifo[i];
			indx++;
			debug_tx_fifo_tail++;
			if (indx == DEBUG_APP_TX_DATA_SIZE)
				break;
		}
		if (debug_tx_fifo_tail == DEBUG_TX_FIFO_BUFFER_SIZE)
			debug_tx_fifo_tail = 0;
	}
	else if (debug_tx_fifo_tail < debug_tx_fifo_head)
	{
		for (uint16_t i = debug_tx_fifo_tail; i < debug_tx_fifo_head; i++)
		{
			temp_buff[indx] = debug_tx_fifo[i];
			indx++;
			debug_tx_fifo_tail++;
			if (indx == DEBUG_APP_TX_DATA_SIZE)
				break;
		}
	}

	DEBUG_Transmit_FS(temp_buff, indx);
	FIFO_Events_busy = false;
	return USBD_BUSY;
}
