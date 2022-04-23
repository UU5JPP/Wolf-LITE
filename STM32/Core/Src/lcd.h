#ifndef LCD_h
#define LCD_h

#include "stm32f4xx_hal.h"
#include "trx_manager.h"
#include "lcd_driver.h"
#include "screen_layout.h"

typedef struct
{
	bool Background;
	bool TopButtons;
	bool TopButtonsRedraw;
	bool BottomButtons;
	bool BottomButtonsRedraw;
	bool FreqInfo;
	bool FreqInfoRedraw;
	bool StatusInfoGUI;
	bool StatusInfoGUIRedraw;
	bool StatusInfoBar;
	bool StatusInfoBarRedraw;
	bool SystemMenu;
	bool SystemMenuRedraw;
	bool SystemMenuCurrent;
	bool Tooltip;
} DEF_LCD_UpdateQuery;

extern void LCD_Init(void);
extern void LCD_doEvents(void);
extern void LCD_showError(char text[], bool redraw);
extern void LCD_showInfo(char text[], bool autohide);
extern void LCD_redraw(bool do_now);
extern void LCD_showTooltip(char text[]);

volatile extern DEF_LCD_UpdateQuery LCD_UpdateQuery;
volatile extern bool LCD_busy;
volatile extern bool LCD_systemMenuOpened;
extern uint16_t LCD_bw_trapez_stripe_pos;
extern STRUCT_COLOR_THEME* COLOR;

#endif
