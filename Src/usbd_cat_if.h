#ifndef __USBD_CDC_CAT_IF_H__
#define __USBD_CDC_CAT_IF_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "usbd_ua3reo.h"

	extern USBD_CAT_ItfTypeDef USBD_CAT_fops_FS;
	extern void ua3reo_dev_cat_parseCommand(void);

#ifdef __cplusplus
}
#endif

#endif
