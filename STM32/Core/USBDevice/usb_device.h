#ifndef __USB_DEVICE__H__
#define __USB_DEVICE__H__

#ifdef __cplusplus
 extern "C" {
#endif

#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"
#include "usbd_def.h"


extern USBD_HandleTypeDef hUsbDeviceFS;
extern void MX_USB_DEVICE_Init(void);
extern void MX_USB_DevDisconnect(void);

#ifdef __cplusplus
}
#endif

#endif /* __USB_DEVICE__H__ */
