#ifndef _LCDDRIVER_ILI9481_H_
#include "settings.h"
#if (defined(LCD_ILI9481) || defined(LCD_HX8357B) || defined(LCD_HX8357C) || defined(LCD_ILI9486) || defined(LCD_R61581))
#define _LCDDRIVER_ILI9481_H_

//LCD dimensions defines
#define LCD_WIDTH 480
#define LCD_HEIGHT 320

//ILI9481 LCD commands
#define LCD_COMMAND_COLUMN_ADDR 0x2A
#define LCD_COMMAND_PAGE_ADDR 0x2B
#define LCD_COMMAND_GRAM 0x2C
#define LCD_COMMAND_SOFT_RESET 0x01
#define LCD_COMMAND_NORMAL_MODE_ON 0x13
#define LCD_COMMAND_VCOM 0xD1
#define LCD_COMMAND_NORMAL_PWR_WR 0xD2
#define LCD_COMMAND_PANEL_DRV_CTL 0xC0
#define LCD_COMMAND_FR_SET 0xC5
#define LCD_COMMAND_GAMMAWR 0xC8
#define LCD_COMMAND_PIXEL_FORMAT 0x3A
#define LCD_COMMAND_DISPLAY_ON 0x29
#define LCD_COMMAND_EXIT_SLEEP_MODE 0x11
#define LCD_COMMAND_POWER_SETTING 0xD0
#define LCD_COMMAND_COLOR_INVERSION_OFF 0x20
#define LCD_COMMAND_COLOR_INVERSION_ON 0x21
#define LCD_COMMAND_TEARING_OFF 0x34
#define LCD_COMMAND_TEARING_ON 0x35
#define LCD_COMMAND_MADCTL 0x36
#define LCD_COMMAND_IDLE_OFF 0x38
#define LCD_COMMAND_TIMING_NORMAL 0xC1
#define LCD_PARAM_MADCTL_MY 0x80
#define LCD_PARAM_MADCTL_MX 0x40
#define LCD_PARAM_MADCTL_MV 0x20
#define LCD_PARAM_MADCTL_ML 0x10
#define LCD_PARAM_MADCTL_RGB 0x00
#define LCD_PARAM_MADCTL_BGR 0x08
#define LCD_PARAM_MADCTL_SS 0x02
#define LCD_PARAM_MADCTL_GS 0x01

#ifndef _swap_int16_t
#define _swap_int16_t(a, b) \
  {                         \
    int16_t t = a;          \
    a = b;                  \
    b = t;                  \
  }
#endif

#endif
#endif
