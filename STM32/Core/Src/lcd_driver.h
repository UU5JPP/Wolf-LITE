#ifndef _LCDDRIVER_H_
#define _LCDDRIVER_H_

//List of includes
#include "settings.h"
#include <stdbool.h>
#include "stm32f4xx_hal.h"
#include <stdlib.h>
#include <stdio.h>
#include "images.h"

#define LCD_FSMC_COMM_ADDR 0x60000000
#define LCD_FSMC_DATA_ADDR 0x60080000

//LCD dimensions defines
#if (defined(LCD_ILI9481) || defined(LCD_HX8357B) || defined(LCD_HX8357C) || defined(LCD_ILI9486) || defined (LCD_SSD1963) || defined(LCD_R61581))
	#include "lcd_driver_ILI9481.h"
#endif

#if (LCD_WIDTH == 480 && LCD_HEIGHT == 320)
	#define LAY_480x320
#endif
#define LCD_PIXEL_COUNT (LCD_WIDTH * LCD_HEIGHT)

//List of colors
#define COLOR_BLACK 0x0000
#define COLOR_NAVY 0x000F
#define COLOR_DGREEN 0x03E0
#define COLOR_DCYAN 0x03EF
#define COLOR_MAROON 0x7800
#define COLOR_PURPLE 0x780F
#define COLOR_OLIVE 0x7BE0
#define COLOR_LGRAY 0xC618
#define COLOR_DGRAY 0x7BEF
#define COLOR_BLUE 0x001F
#define COLOR_BLUE2 0x051D
#define COLOR_GREEN 0x07E0
#define COLOR_GREEN2 0xB723
#define COLOR_GREEN3 0x8000
#define COLOR_CYAN 0x07FF
#define COLOR_RED 0xF800
#define COLOR_MAGENTA 0xF81F
#define COLOR_YELLOW 0xFFE0
#define COLOR_WHITE 0xFFFF
#define COLOR_ORANGE 0xFD20
#define COLOR_GREENYELLOW 0xAFE5
#define COLOR_BROWN 0XBC40

/// Font data stored PER GLYPH
typedef struct
{
	uint16_t bitmapOffset; ///< Pointer into GFXfont->bitmap
	uint8_t width;		   ///< Bitmap dimensions in pixels
	uint8_t height;		   ///< Bitmap dimensions in pixels
	uint8_t xAdvance;	   ///< Distance to advance cursor (x axis)
	int8_t xOffset;		   ///< X dist from cursor pos to UL corner
	int8_t yOffset;		   ///< Y dist from cursor pos to UL corner
} GFXglyph;

/// Data stored for FONT AS A WHOLE
typedef struct
{
	const uint8_t *bitmap;	///< Glyph bitmaps, concatenated
	const GFXglyph *glyph;	///< Glyph array
	const uint8_t first;	///< ASCII extents (first char)
	const uint8_t last;		///< ASCII extents (last char)
	const uint8_t yAdvance; ///< Newline distance (y axis)
} GFXfont;

//Functions defines Macros
#define uswap(a, b)     \
	{                   \
		uint16_t t = a; \
		a = b;          \
		b = t;          \
	}
#define rgb888torgb565(r, g, b) ((uint16_t)(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xFF) >> 3)))
	
extern void LCDDriver_SendData(uint16_t data);
extern uint16_t LCDDriver_readReg(uint16_t reg);
extern void LCDDriver_SetCursorAreaPosition(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
extern uint16_t LCDDriver_GetCurrentXOffset(void);
extern void LCDDriver_Init(void);
extern void LCDDriver_Fill(uint16_t color);
extern void LCDDriver_Fill_RectXY(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);
extern void LCDDriver_Fill_RectWH(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
extern void LCDDriver_Fill_Triangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
extern void LCDDriver_drawPixel(uint16_t x, uint16_t y, uint16_t color);
extern void LCDDriver_drawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);
extern void LCDDriver_drawFastHLine(uint16_t x, uint16_t y, int16_t w, uint16_t color);
extern void LCDDriver_drawFastVLine(uint16_t x, uint16_t y, int16_t h, uint16_t color);
extern void LCDDriver_drawRectXY(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);
extern void LCDDriver_drawChar(uint16_t x, uint16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size);
extern void LCDDriver_drawCharFont(uint16_t x, uint16_t y, unsigned char c, uint16_t color, uint16_t bg, const GFXfont *gfxFont);
extern void LCDDriver_printText(char text[], uint16_t x, uint16_t y, uint16_t color, uint16_t bg, uint8_t size);
extern void LCDDriver_printTextFont(char text[], uint16_t x, uint16_t y, uint16_t color, uint16_t bg, const GFXfont *gfxFont);
extern void LCDDriver_getTextBounds(char text[], uint16_t x, uint16_t y, uint16_t *x1, uint16_t *y1, uint16_t *w, uint16_t *h, const GFXfont *gfxFont);
extern void LCDDriver_printImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t *data);
extern void LCDDriver_printImage_RLECompressed(uint16_t x, uint16_t y, const tIMAGE *image, uint16_t transparent_color, uint16_t bg_color);
extern void LCDDriver_setRotation(uint8_t rotate);
extern void LCDDriver_setBrightness(uint8_t percent);
extern void LCDDriver_printImage_RLECompressed_StartStream(uint16_t x, uint16_t y, uint16_t width, uint16_t height);
extern void LCDDriver_printImage_RLECompressed_ContinueStream(int16_t *data, uint16_t len);
extern uint16_t addColor(uint16_t color, uint8_t add_r, uint8_t add_g, uint8_t add_b); //add opacity or mix colors
extern uint16_t mixColors(uint16_t color1, uint16_t color2, float32_t opacity);		  //mix two colors with opacity

#endif
