#ifndef SYSTEM_MENU_H
#define SYSTEM_MENU_H

#include "stm32f4xx.h"
#include <stdbool.h>

typedef enum
{
	SYSMENU_BOOLEAN = 0x01,
	SYSMENU_RUN = 0x02,
	SYSMENU_UINT8 = 0x03,
	SYSMENU_UINT16 = 0x04,
	SYSMENU_UINT32 = 0x05,
	SYSMENU_UINT32R = 0x06,
	SYSMENU_INT8 = 0x07,
	SYSMENU_INT16 = 0x08,
	SYSMENU_INT32 = 0x09,
	SYSMENU_FLOAT32 = 0x0A,
	SYSMENU_MENU = 0x0B,
	SYSMENU_HIDDEN_MENU = 0x0C,
	SYSMENU_INFOLINE = 0x0D,
} SystemMenuType;

struct sysmenu_item_handler
{
	char *title;
	SystemMenuType type;
	uint32_t *value;
	void (*menuHandler)(int8_t direction);
};

extern void SYSMENU_drawSystemMenu(bool draw_background);
extern void SYSMENU_redrawCurrentItem(void);
extern void eventRotateSystemMenu(int8_t direction);
extern void eventSecRotateSystemMenu(int8_t direction);
extern void eventCloseSystemMenu(void);
extern void eventCloseAllSystemMenu(void);
extern bool sysmenu_hiddenmenu_enabled;
extern void SYSMENU_TRX_RFPOWER_HOTKEY(void);
extern void SYSMENU_TRX_STEP_HOTKEY(void);
extern void SYSMENU_CW_WPM_HOTKEY(void);
extern void SYSMENU_CW_KEYER_HOTKEY(void);
extern void SYSMENU_AUDIO_VOLUME_HOTKEY(void);
extern void SYSMENU_AUDIO_BW_SSB_HOTKEY(void);
extern void SYSMENU_AUDIO_BW_CW_HOTKEY(void);
extern void SYSMENU_AUDIO_BW_AM_HOTKEY(void);
extern void SYSMENU_AUDIO_BW_FM_HOTKEY(void);
extern void SYSMENU_AUDIO_HPF_SSB_HOTKEY(void);
extern void SYSMENU_AUDIO_HPF_CW_HOTKEY(void);
extern void SYSMENU_AUDIO_SQUELCH_HOTKEY(void);
extern void SYSMENU_AUDIO_AGC_HOTKEY(void);

#endif
