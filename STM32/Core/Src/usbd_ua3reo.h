#ifndef __USB_CDC_H
#define __USB_CDC_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "usbd_ioreq.h"
#include "audio_processor.h"

#define DEBUG_INTERFACE_IDX 0x0 // Index of DEBUG interface
#define CAT_INTERFACE_IDX 0x2	// Index of CAT interface
#define AUDIO_INTERFACE_IDX 0x4 // Index of AUDIO interface

#define DEBUG_EP_IDX 0x01
#define CAT_EP_IDX 0x02
#define AUDIO_EP_IDX 0x03
#define DEBUG_CMD_IDX 0x04
#define CAT_CMD_IDX 0x04

#define IN_EP_DIR 0x80 // Adds a direction bit

#define DEBUG_OUT_EP DEBUG_EP_IDX
#define DEBUG_IN_EP (DEBUG_EP_IDX | IN_EP_DIR)
#define DEBUG_CMD_EP (DEBUG_CMD_IDX | IN_EP_DIR)

#define CAT_OUT_EP CAT_EP_IDX
#define CAT_IN_EP (CAT_EP_IDX | IN_EP_DIR)
#define CAT_CMD_EP (CAT_CMD_IDX | IN_EP_DIR)

#define AUDIO_OUT_EP AUDIO_EP_IDX
#define AUDIO_IN_EP (AUDIO_EP_IDX | IN_EP_DIR)

#ifndef CDC_HS_BINTERVAL
#define CDC_HS_BINTERVAL 0x10U
#endif /* CDC_HS_BINTERVAL */

#ifndef CDC_FS_BINTERVAL
#define CDC_FS_BINTERVAL 0x10U
#endif /* CDC_FS_BINTERVAL */

/* CDC Endpoints parameters: you can fine tune these values depending on the needed baudrates and performance. */
#define CDC_DATA_HS_MAX_PACKET_SIZE 16U /* Endpoint IN & OUT Packet size */
#define CDC_DATA_FS_MAX_PACKET_SIZE 16U /* Endpoint IN & OUT Packet size */
#define CDC_CMD_PACKET_SIZE 16U			/* Control Endpoint Packet size */

#define USB_CDC_CONFIG_DESC_SIZ 314U

#define CDC_DATA_HS_IN_PACKET_SIZE CDC_DATA_HS_MAX_PACKET_SIZE
#define CDC_DATA_HS_OUT_PACKET_SIZE CDC_DATA_HS_MAX_PACKET_SIZE

#define CDC_DATA_FS_IN_PACKET_SIZE CDC_DATA_FS_MAX_PACKET_SIZE
#define CDC_DATA_FS_OUT_PACKET_SIZE CDC_DATA_FS_MAX_PACKET_SIZE

/*---------------------------------------------------------------------*/
/*  CDC definitions                                                    */
/*---------------------------------------------------------------------*/
#define CDC_SEND_ENCAPSULATED_COMMAND 0x00U
#define CDC_GET_ENCAPSULATED_RESPONSE 0x01U
#define CDC_SET_COMM_FEATURE 0x02U
#define CDC_GET_COMM_FEATURE 0x03U
#define CDC_CLEAR_COMM_FEATURE 0x04U
#define CDC_SET_LINE_CODING 0x20U
#define CDC_GET_LINE_CODING 0x21U
#define CDC_SET_CONTROL_LINE_STATE 0x22U
#define CDC_SEND_BREAK 0x23U

//AUDIO
#define USBD_AUDIO_FREQ 48000U
#define BYTES_IN_SAMPLE_AUDIO_OUT_PACKET 2U													//16bit
#define AUDIO_OUT_PACKET (BYTES_IN_SAMPLE_AUDIO_OUT_PACKET * 2 * (USBD_AUDIO_FREQ / 1000))	//2bytes (16bit) * 2 channel * 48 packet per second
#define USB_AUDIO_RX_BUFFER_SIZE (AUDIO_BUFFER_SIZE * BYTES_IN_SAMPLE_AUDIO_OUT_PACKET)		//16 bit
#define USB_AUDIO_TX_BUFFER_SIZE (AUDIO_BUFFER_SIZE * BYTES_IN_SAMPLE_AUDIO_OUT_PACKET * 2) //16 bit x2 size

#define AUDIO_REQ_GET_CUR 0x81U
#define AUDIO_REQ_SET_CUR 0x01U
#define AUDIO_OUT_STREAMING_CTRL 0x02U
#define USB_AUDIO_DESC_SIZ 0x09U
#define AUDIO_DESCRIPTOR_TYPE 0x21U

	extern volatile uint32_t RX_USB_AUDIO_SAMPLES;
	extern volatile uint32_t TX_USB_AUDIO_SAMPLES;
	extern volatile bool RX_USB_AUDIO_underrun;
	extern volatile uint32_t USB_LastActiveTime;

	typedef struct
	{
		uint32_t bitrate;
		uint8_t format;
		uint8_t paritytype;
		uint8_t datatype;
	} USBD_CDC_LineCodingTypeDef;

	typedef enum
	{
		AUDIO_CMD_START = 1,
		AUDIO_CMD_PLAY,
		AUDIO_CMD_STOP,
	} AUDIO_CMD_TypeDef;

	typedef enum
	{
		AUDIO_OFFSET_NONE = 0,
		AUDIO_OFFSET_HALF,
		AUDIO_OFFSET_FULL,
		AUDIO_OFFSET_UNKNOWN,
	} AUDIO_OffsetTypeDef;

	typedef struct
	{
		uint8_t cmd;
		uint8_t data[USB_MAX_EP0_SIZE];
		uint8_t len;
		uint8_t unit;
	} USBD_AUDIO_ControlTypeDef;

	typedef struct _USBD_DEBUG_Itf
	{
		int8_t (*Init)(void);
		int8_t (*DeInit)(void);
		int8_t (*Control)(uint8_t cmd, uint8_t *pbuf, uint32_t len);
		int8_t (*Receive)(uint8_t *Buf);

	} USBD_DEBUG_ItfTypeDef;

	typedef struct _USBD_CAT_Itf
	{
		int8_t (*Init)(void);
		int8_t (*DeInit)(void);
		int8_t (*Control)(uint8_t cmd, uint8_t *pbuf);
		int8_t (*Receive)(uint8_t *Buf, uint32_t *Len);

	} USBD_CAT_ItfTypeDef;

	typedef struct
	{
		int8_t (*Init)(void);
		int8_t (*DeInit)(void);
	} USBD_AUDIO_ItfTypeDef;

	typedef struct
	{
		uint32_t alt_setting;
		USBD_AUDIO_ControlTypeDef control;
		uint8_t *RxBuffer;
		uint8_t *TxBuffer;
		uint32_t TxBufferIndex;
	} USBD_AUDIO_HandleTypeDef;

	typedef struct
	{
		uint32_t data[CDC_DATA_HS_MAX_PACKET_SIZE / 4U]; /* Force 32bits alignment */
		uint8_t CmdOpCode;
		uint8_t CmdLength;
		uint8_t *RxBuffer;
		uint8_t *TxBuffer;
		uint32_t RxLength;
		uint32_t TxLength;

		__IO uint32_t TxState;
		__IO uint32_t RxState;
	} USBD_DEBUG_HandleTypeDef;

	typedef struct
	{
		uint32_t data[CDC_DATA_HS_MAX_PACKET_SIZE / 4U]; /* Force 32bits alignment */
		uint8_t CmdOpCode;
		uint8_t CmdLength;
		uint8_t *RxBuffer;
		uint8_t *TxBuffer;
		uint32_t RxLength;
		uint32_t TxLength;

		__IO uint32_t TxState;
		__IO uint32_t RxState;
	} USBD_CAT_HandleTypeDef;

	extern USBD_ClassTypeDef USBD_UA3REO;
	extern USBD_HandleTypeDef hUsbDeviceFS;

#define USBD_UA3REO_CLASS &USBD_UA3REO

	extern uint8_t USBD_DEBUG_RegisterInterface(USBD_HandleTypeDef *pdev, USBD_DEBUG_ItfTypeDef *fops);
	extern uint8_t USBD_DEBUG_SetTxBuffer(USBD_HandleTypeDef *pdev, uint8_t *pbuff, uint16_t length);
	extern uint8_t USBD_DEBUG_SetRxBuffer(USBD_HandleTypeDef *pdev, uint8_t *pbuff);
	extern uint8_t USBD_DEBUG_ReceivePacket(USBD_HandleTypeDef *pdev);
	extern uint8_t USBD_DEBUG_TransmitPacket(USBD_HandleTypeDef *pdev);

	extern uint8_t USBD_CAT_RegisterInterface(USBD_HandleTypeDef *pdev, USBD_CAT_ItfTypeDef *fops);
	extern uint8_t USBD_CAT_SetTxBuffer(USBD_HandleTypeDef *pdev, uint8_t *pbuff, uint16_t length);
	extern uint8_t USBD_CAT_SetRxBuffer(USBD_HandleTypeDef *pdev, uint8_t *pbuff);
	extern uint8_t USBD_CAT_ReceivePacket(USBD_HandleTypeDef *pdev);
	extern uint8_t USBD_CAT_TransmitPacket(USBD_HandleTypeDef *pdev);

	extern uint8_t USBD_AUDIO_RegisterInterface(USBD_HandleTypeDef *pdev, USBD_AUDIO_ItfTypeDef *fops);
	extern uint8_t USBD_AUDIO_StartTransmit(USBD_HandleTypeDef *pdev);
	extern uint8_t USBD_AUDIO_StartReceive(USBD_HandleTypeDef *pdev);
	extern void USBD_Restart(void);

	/**
	  * @}
	  */

#ifdef __cplusplus
}
#endif

#endif
