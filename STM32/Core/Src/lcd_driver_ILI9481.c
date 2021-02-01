#include "settings.h"
#if (defined(LCD_ILI9481) || defined(LCD_HX8357B) || defined(LCD_HX8357C) || defined(LCD_ILI9486) || defined(LCD_R61581))

//Header files
#include "lcd_driver.h"
#include "main.h"
#include "fonts.h"
#include "functions.h"
#include "lcd_driver_ILI9481.h"

//***** Functions prototypes *****//
static inline void LCDDriver_SetCursorPosition(uint16_t x, uint16_t y);
inline void LCDDriver_SetCursorAreaPosition(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

//Write command to LCD
inline void LCDDriver_SendCommand(uint16_t com)
{
	*(__IO uint16_t *)((uint32_t)(LCD_FSMC_COMM_ADDR)) = com;
}

//Write data to LCD
inline void LCDDriver_SendData(uint16_t data)
{
	*(__IO uint16_t *)((uint32_t)(LCD_FSMC_DATA_ADDR)) = data;
}

//Write pair command-data
inline void LCDDriver_writeReg(uint16_t reg, uint16_t val) {
  LCDDriver_SendCommand(reg);
  LCDDriver_SendData(val);
}

//Read command from LCD
inline uint16_t LCDDriver_ReadStatus(void)
{
	return *(__IO uint16_t *)((uint32_t)(LCD_FSMC_COMM_ADDR));
}
//Read data from LCD
inline uint16_t LCDDriver_ReadData(void)
{
	return *(__IO uint16_t *)((uint32_t)(LCD_FSMC_DATA_ADDR));
}


//Read Register
inline uint16_t LCDDriver_readReg(uint16_t reg)
{
  LCDDriver_SendCommand(reg);
  return LCDDriver_ReadData();
}

//Initialise function
void LCDDriver_Init(void)
{
#if (defined(LCD_ILI9481) || defined(LCD_HX8357B))
	#define ILI9481_COMM_DELAY 20
	
	LCDDriver_SendCommand(LCD_COMMAND_SOFT_RESET); //0x01
	HAL_Delay(ILI9481_COMM_DELAY);

	LCDDriver_SendCommand(LCD_COMMAND_EXIT_SLEEP_MODE); //0x11
	HAL_Delay(ILI9481_COMM_DELAY);

	LCDDriver_SendCommand(LCD_COMMAND_NORMAL_MODE_ON); //0x13
	
	LCDDriver_SendCommand(LCD_COMMAND_POWER_SETTING); //(0xD0);
	LCDDriver_SendData(0x07);
	LCDDriver_SendData(0x42);
	LCDDriver_SendData(0x18);

	LCDDriver_SendCommand(LCD_COMMAND_VCOM); //(0xD1);
	LCDDriver_SendData(0x00);
	LCDDriver_SendData(0x07);
	LCDDriver_SendData(0x10);
	
	LCDDriver_SendCommand(LCD_COMMAND_NORMAL_PWR_WR); //(0xD2);
	LCDDriver_SendData(0x01);
	LCDDriver_SendData(0x02);
	HAL_Delay(ILI9481_COMM_DELAY);

#if defined(LCD_HX8357B)	
	LCDDriver_SendCommand(LCD_COMMAND_PANEL_DRV_CTL); //(0xC0);
	LCDDriver_SendData(0x10);
	LCDDriver_SendData(0x3B);
	LCDDriver_SendData(0x00);
	LCDDriver_SendData(0x02);
	LCDDriver_SendData(0x11);
	HAL_Delay(ILI9481_COMM_DELAY);
#endif
	
	LCDDriver_SendCommand(LCD_COMMAND_FR_SET); //(0xC5);
	LCDDriver_SendData(0x03);
	HAL_Delay(ILI9481_COMM_DELAY);
	
	LCDDriver_SendCommand(LCD_COMMAND_GAMMAWR); //(0xC8);
	LCDDriver_SendData(0x00);
	LCDDriver_SendData(0x32);
	LCDDriver_SendData(0x36);
	LCDDriver_SendData(0x45);
	LCDDriver_SendData(0x06);
	LCDDriver_SendData(0x16);
	LCDDriver_SendData(0x37);
	LCDDriver_SendData(0x75);
	LCDDriver_SendData(0x77);
	LCDDriver_SendData(0x54);
	LCDDriver_SendData(0x0C);
	LCDDriver_SendData(0x00);
	HAL_Delay(ILI9481_COMM_DELAY);
	
	LCDDriver_SendCommand(LCD_COMMAND_MADCTL); //(0x36);
	LCDDriver_SendData(LCD_PARAM_MADCTL_BGR);
	HAL_Delay(ILI9481_COMM_DELAY);
	
	LCDDriver_SendCommand(LCD_COMMAND_PIXEL_FORMAT); //(0x3A);
	LCDDriver_SendData(0x55);
	HAL_Delay(ILI9481_COMM_DELAY);
	
	LCDDriver_SendCommand(LCD_COMMAND_COLUMN_ADDR); //(0x2A);
	LCDDriver_SendData(0x00);
	LCDDriver_SendData(0x00);
	LCDDriver_SendData(0x01);
	LCDDriver_SendData(0x3F);
	HAL_Delay(ILI9481_COMM_DELAY);
	
	LCDDriver_SendCommand(LCD_COMMAND_PAGE_ADDR); //(0x2B);
	LCDDriver_SendData(0x00);
	LCDDriver_SendData(0x00);
	LCDDriver_SendData(0x01);
	LCDDriver_SendData(0xDF);
	HAL_Delay(ILI9481_COMM_DELAY);
	
#if defined(LCD_HX8357B)	
	LCDDriver_SendCommand(LCD_COMMAND_COLOR_INVERSION_ON); //(0x21);
	HAL_Delay(ILI9481_COMM_DELAY);
#endif

	LCDDriver_SendCommand(LCD_COMMAND_IDLE_OFF);		   //(0x38);
	HAL_Delay(ILI9481_COMM_DELAY);
	
	LCDDriver_SendCommand(LCD_COMMAND_DISPLAY_ON);		   //(0x29);
	HAL_Delay(ILI9481_COMM_DELAY);
#endif 

#if defined(LCD_HX8357C)
	LCDDriver_SendCommand(0x11);
	HAL_Delay(20);
	LCDDriver_SendCommand(0xD0);
	LCDDriver_SendData(0x07);
	LCDDriver_SendData(0x42);
	LCDDriver_SendData(0x18);

	LCDDriver_SendCommand(0xD1);
	LCDDriver_SendData(0x00);
	LCDDriver_SendData(0x07);
	LCDDriver_SendData(0x10);

	LCDDriver_SendCommand(0xD2);
	LCDDriver_SendData(0x01);
	LCDDriver_SendData(0x02);

	LCDDriver_SendCommand(0xC0);
	LCDDriver_SendData(0x10);
	LCDDriver_SendData(0x3B);
	LCDDriver_SendData(0x00);
	LCDDriver_SendData(0x02);
	LCDDriver_SendData(0x11);

	LCDDriver_SendCommand(0xC5);
	LCDDriver_SendData(0x03);

	LCDDriver_SendCommand(0xC8);
	LCDDriver_SendData(0x00);
	LCDDriver_SendData(0x32);
	LCDDriver_SendData(0x36);
	LCDDriver_SendData(0x45);
	LCDDriver_SendData(0x06);
	LCDDriver_SendData(0x16);
	LCDDriver_SendData(0x37);
	LCDDriver_SendData(0x75);
	LCDDriver_SendData(0x77);
	LCDDriver_SendData(0x54);
	LCDDriver_SendData(0x0C);
	LCDDriver_SendData(0x00);

	LCDDriver_SendCommand(0x36);
	LCDDriver_SendData(0x8A);
	
	LCDDriver_SendCommand(0x35); // Tearing effect on
  LCDDriver_SendData(0x00);    // Added parameter

	LCDDriver_SendCommand(0x3A);
	LCDDriver_SendData(0x55);

	LCDDriver_SendCommand(0x2A);
	LCDDriver_SendData(0x00);
	LCDDriver_SendData(0x00);
	LCDDriver_SendData(0x01);
	LCDDriver_SendData(0x3F);

	LCDDriver_SendCommand(0x2B);
	LCDDriver_SendData(0x00);
	LCDDriver_SendData(0x00);
	LCDDriver_SendData(0x01);
	LCDDriver_SendData(0xE0);
	HAL_Delay(120);
	LCDDriver_SendCommand(0x29);
#endif

#if	defined(LCD_ILI9486) 
	#define ILI9481_COMM_DELAY 100
	
	HAL_Delay(ILI9481_COMM_DELAY);
	
	LCDDriver_SendCommand(0XF1);
	LCDDriver_SendData(0x36);
	LCDDriver_SendData(0x04);
	LCDDriver_SendData(0x00);
	LCDDriver_SendData(0x3C);
	LCDDriver_SendData(0X0F);
	LCDDriver_SendData(0x8F);
	
	LCDDriver_SendCommand(0XF2);
	LCDDriver_SendData(0x18);
	LCDDriver_SendData(0xA3);
	LCDDriver_SendData(0x12);
	LCDDriver_SendData(0x02);
	LCDDriver_SendData(0XB2);
	LCDDriver_SendData(0x12);
	LCDDriver_SendData(0xFF);
	LCDDriver_SendData(0x10);
	LCDDriver_SendData(0x00);
	
	LCDDriver_SendCommand(0XF8);
	LCDDriver_SendData(0x21);
	LCDDriver_SendData(0x04);
	
	LCDDriver_SendCommand(0XF9);
	LCDDriver_SendData(0x00);
	LCDDriver_SendData(0x08);
	
	LCDDriver_SendCommand(0x36);
	LCDDriver_SendData(0x08);
	
	LCDDriver_SendCommand(0xB4);
	LCDDriver_SendData(0x00);
	
	LCDDriver_SendCommand(0xC1);
	LCDDriver_SendData(0x47); //0x41
	
	LCDDriver_SendCommand(0xC5);
	LCDDriver_SendData(0x00);
	LCDDriver_SendData(0xAF); //0x91
	LCDDriver_SendData(0x80);
	LCDDriver_SendData(0x00);
	
	LCDDriver_SendCommand(0xE0);
	LCDDriver_SendData(0x0F);
	LCDDriver_SendData(0x1F);
	LCDDriver_SendData(0x1C);
	LCDDriver_SendData(0x0C);
	LCDDriver_SendData(0x0F);
	LCDDriver_SendData(0x08);
	LCDDriver_SendData(0x48);
	LCDDriver_SendData(0x98);
	LCDDriver_SendData(0x37);
	LCDDriver_SendData(0x0A);
	LCDDriver_SendData(0x13);
	LCDDriver_SendData(0x04);
	LCDDriver_SendData(0x11);
	LCDDriver_SendData(0x0D);
	LCDDriver_SendData(0x00);
	
	LCDDriver_SendCommand(0xE1);
	LCDDriver_SendData(0x0F);
	LCDDriver_SendData(0x32);
	LCDDriver_SendData(0x2E);
	LCDDriver_SendData(0x0B);
	LCDDriver_SendData(0x0D);
	LCDDriver_SendData(0x05);
	LCDDriver_SendData(0x47);
	LCDDriver_SendData(0x75);
	LCDDriver_SendData(0x37);
	LCDDriver_SendData(0x06);
	LCDDriver_SendData(0x10);
	LCDDriver_SendData(0x03);
	LCDDriver_SendData(0x24);
	LCDDriver_SendData(0x20);
	LCDDriver_SendData(0x00);
	
	LCDDriver_SendCommand(0x3A);
	LCDDriver_SendData(0x55);
	
	LCDDriver_SendCommand(0x11);
	
	LCDDriver_SendCommand(0x36);
	
	LCDDriver_SendData(0x28);
	HAL_Delay(ILI9481_COMM_DELAY);
	
	LCDDriver_SendCommand(0x29);
#endif

#if defined(LCD_R61581)
	LCDDriver_SendCommand(0xB0);		
	LCDDriver_SendData(0x1E);	    

	LCDDriver_SendCommand(0xB0);
	LCDDriver_SendData(0x00);

	LCDDriver_SendCommand(0xB3);
	LCDDriver_SendData(0x02);
	LCDDriver_SendData(0x00);
	LCDDriver_SendData(0x00);
	LCDDriver_SendData(0x10);

	LCDDriver_SendCommand(0xB4);
	LCDDriver_SendData(0x00);//0X10

// 		LCDDriver_SendCommand(0xB9); //PWM Settings for Brightness Control
// 		LCDDriver_SendData(0x01);// Disabled by default. 
// 		LCDDriver_SendData(0xFF); //0xFF = Max brightness
// 		LCDDriver_SendData(0xFF);
// 		LCDDriver_SendData(0x18);

	LCDDriver_SendCommand(0xC0);
	LCDDriver_SendData(0x03);
	LCDDriver_SendData(0x3B);//
	LCDDriver_SendData(0x00);
	LCDDriver_SendData(0x00);
	LCDDriver_SendData(0x00);
	LCDDriver_SendData(0x01);
	LCDDriver_SendData(0x00);//NW
	LCDDriver_SendData(0x43);

	LCDDriver_SendCommand(0xC1);
	LCDDriver_SendData(0x08);
	LCDDriver_SendData(0x15);//CLOCK
	LCDDriver_SendData(0x08);
	LCDDriver_SendData(0x08);

	LCDDriver_SendCommand(0xC4);
	LCDDriver_SendData(0x15);
	LCDDriver_SendData(0x03);
	LCDDriver_SendData(0x03);
	LCDDriver_SendData(0x01);

	LCDDriver_SendCommand(0xC6);
	LCDDriver_SendData(0x02);

	LCDDriver_SendCommand(0xC8);
	LCDDriver_SendData(0x0c);
	LCDDriver_SendData(0x05);
	LCDDriver_SendData(0x0A);//0X12
	LCDDriver_SendData(0x6B);//0x7D
	LCDDriver_SendData(0x04);
	LCDDriver_SendData(0x06);//0x08
	LCDDriver_SendData(0x15);//0x0A
	LCDDriver_SendData(0x10);
	LCDDriver_SendData(0x00);
	LCDDriver_SendData(0x60);//0x23

	LCDDriver_SendCommand(0x36);
	LCDDriver_SendData(0x0A);

	LCDDriver_SendCommand(0x0C);
	LCDDriver_SendData(0x55);

	LCDDriver_SendCommand(0x3A);
	LCDDriver_SendData(0x55);

	LCDDriver_SendCommand(0x38);

	LCDDriver_SendCommand(0xD0);
	LCDDriver_SendData(0x07);
	LCDDriver_SendData(0x07);//VCI1
	LCDDriver_SendData(0x14);//VRH 0x1D
	LCDDriver_SendData(0xA2);//BT 0x06

	LCDDriver_SendCommand(0xD1);
	LCDDriver_SendData(0x03);
	LCDDriver_SendData(0x5A);//VCM  0x5A
	LCDDriver_SendData(0x10);//VDV

	LCDDriver_SendCommand(0xD2);
	LCDDriver_SendData(0x03);
	LCDDriver_SendData(0x04);//0x24
	LCDDriver_SendData(0x04);

	LCDDriver_SendCommand(0x11);
	HAL_Delay(150);

	LCDDriver_SendCommand(0x2A);
	LCDDriver_SendData(0x00);
	LCDDriver_SendData(0x00);
	LCDDriver_SendData(0x01);
	LCDDriver_SendData(0xDF);//320

	LCDDriver_SendCommand(0x2B);
	LCDDriver_SendData(0x00);
	LCDDriver_SendData(0x00);
	LCDDriver_SendData(0x01);
	LCDDriver_SendData(0x3F);//480


	HAL_Delay(100);

	LCDDriver_SendCommand(0x29);
	HAL_Delay(30);

	LCDDriver_SendCommand(0x2C);
	HAL_Delay(30);
#endif
}

//Set screen rotation
void LCDDriver_setRotation(uint8_t rotate)
{
	#if defined(LCD_ILI9481) || defined(LCD_HX8357B)  || defined(LCD_ILI9486)
		LCDDriver_SendCommand(LCD_COMMAND_MADCTL);
		switch (rotate)
		{
		case 1: // Portrait
			LCDDriver_SendData(LCD_PARAM_MADCTL_BGR | LCD_PARAM_MADCTL_MX);
			break;
		case 2: // Landscape (Portrait + 90)
			LCDDriver_SendData(LCD_PARAM_MADCTL_BGR | LCD_PARAM_MADCTL_MV);
			break;
		case 3: // Inverter portrait
			LCDDriver_SendData(LCD_PARAM_MADCTL_BGR | LCD_PARAM_MADCTL_MY);
			break;
		case 4: // Inverted landscape
			LCDDriver_SendData(LCD_PARAM_MADCTL_BGR | LCD_PARAM_MADCTL_MV | LCD_PARAM_MADCTL_MX | LCD_PARAM_MADCTL_MY);
			//LCDDriver_SendData(LCD_PARAM_MADCTL_BGR | LCD_PARAM_MADCTL_MV | LCD_PARAM_MADCTL_SS | LCD_PARAM_MADCTL_GS);
			break;
		}
		HAL_Delay(120);
	#endif
	#if defined(LCD_HX8357C)
		LCDDriver_SendCommand(LCD_COMMAND_MADCTL);
		switch (rotate)
		{
		case 1: // Portrait
			LCDDriver_SendData(LCD_PARAM_MADCTL_BGR | LCD_PARAM_MADCTL_MX);
			break;
		case 2: // Landscape (Portrait + 90)
			LCDDriver_SendData(LCD_PARAM_MADCTL_BGR | LCD_PARAM_MADCTL_MV);
			break;
		case 3: // Inverter portrait
			LCDDriver_SendData(LCD_PARAM_MADCTL_BGR | LCD_PARAM_MADCTL_MY);
			break;
		case 4: // Inverted landscape
			LCDDriver_SendData(LCD_PARAM_MADCTL_BGR | LCD_PARAM_MADCTL_MV | LCD_PARAM_MADCTL_GS | LCD_PARAM_MADCTL_ML | LCD_PARAM_MADCTL_SS);
			break;
		}
		HAL_Delay(120);
	#endif
}

//Set cursor position
inline void LCDDriver_SetCursorAreaPosition(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	LCDDriver_SendCommand(LCD_COMMAND_COLUMN_ADDR);
	LCDDriver_SendData(x1 >> 8);
	LCDDriver_SendData(x1 & 0xFF);
	LCDDriver_SendData(x2 >> 8);
	LCDDriver_SendData(x2 & 0xFF);
	LCDDriver_SendCommand(LCD_COMMAND_PAGE_ADDR);
	LCDDriver_SendData(y1 >> 8);
	LCDDriver_SendData(y1 & 0xFF);
	LCDDriver_SendData(y2 >> 8);
	LCDDriver_SendData(y2 & 0xFF);
	LCDDriver_SendCommand(LCD_COMMAND_GRAM);
}

static inline void LCDDriver_SetCursorPosition(uint16_t x, uint16_t y)
{
	LCDDriver_SendCommand(LCD_COMMAND_COLUMN_ADDR);
	LCDDriver_SendData(x >> 8); //-V760
	LCDDriver_SendData(x & 0xFF);
	LCDDriver_SendData(x >> 8);
	LCDDriver_SendData(x & 0xFF);
	LCDDriver_SendCommand(LCD_COMMAND_PAGE_ADDR);
	LCDDriver_SendData(y >> 8); //-V760
	LCDDriver_SendData(y & 0xFF);
	LCDDriver_SendData(y >> 8);
	LCDDriver_SendData(y & 0xFF);
	LCDDriver_SendCommand(LCD_COMMAND_GRAM);
}

//Write data to a single pixel
void LCDDriver_drawPixel(uint16_t x, uint16_t y, uint16_t color)
{
	LCDDriver_SetCursorPosition(x, y);
	LCDDriver_SendData(color);
}

//Fill the entire screen with a background color
void LCDDriver_Fill(uint16_t color)
{
	LCDDriver_Fill_RectXY(0, 0, LCD_WIDTH, LCD_HEIGHT, color);
}

//Rectangle drawing functions
IRAM1 static uint16_t fillxy_color = 0;
void LCDDriver_Fill_RectXY(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color)
{
	if (x1 > (LCD_WIDTH - 1))
		x1 = LCD_WIDTH - 1;
	if (y1 > (LCD_HEIGHT - 1))
		y1 = LCD_HEIGHT - 1;
	uint32_t n = ((x1 + 1) - x0) * ((y1 + 1) - y0);
	if (n > LCD_PIXEL_COUNT)
		n = LCD_PIXEL_COUNT;
	LCDDriver_SetCursorAreaPosition(x0, y0, x1, y1);
	fillxy_color = color;
	
	if (n > 50)
	{
		const uint32_t part_size = 30000;
		uint32_t estamated = n;
		while (estamated > 0)
		{
			if (estamated >= part_size)
			{
				HAL_DMA_Start(&hdma_memtomem_dma2_stream3, (uint32_t)&fillxy_color, LCD_FSMC_DATA_ADDR, part_size);
				HAL_DMA_PollForTransfer(&hdma_memtomem_dma2_stream3, HAL_DMA_FULL_TRANSFER, HAL_MAX_DELAY);
				estamated -= part_size;
			}
			else
			{
				HAL_DMA_Start(&hdma_memtomem_dma2_stream3, (uint32_t)&fillxy_color, LCD_FSMC_DATA_ADDR, estamated);
				HAL_DMA_PollForTransfer(&hdma_memtomem_dma2_stream3, HAL_DMA_FULL_TRANSFER, HAL_MAX_DELAY);
				estamated = 0;
			}
		}
	}
	else
	{
		while (n--)
		{
			LCDDriver_SendData(color);
		}
	}
}

void LCDDriver_Fill_RectWH(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
	LCDDriver_Fill_RectXY(x, y, x + w, y + h, color);
}

//Line drawing functions
void LCDDriver_drawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color)
{
	int16_t steep = abs(y1 - y0) > abs(x1 - x0);
	if (steep)
	{
		uswap(x0, y0)
		uswap(x1, y1)
	}

	if (x0 > x1)
	{
		uswap(x0, x1)
		uswap(y0, y1)
	}

	int16_t dx, dy;
	dx = (int16_t)(x1 - x0);
	dy = (int16_t)abs(y1 - y0);

	int16_t err = dx / 2;
	int16_t ystep;

	if (y0 < y1)
	{
		ystep = 1;
	}
	else
	{
		ystep = -1;
	}

	for (; x0 <= x1; x0++)
	{
		if (steep)
		{
			LCDDriver_drawPixel(y0, x0, color);
		}
		else
		{
			LCDDriver_drawPixel(x0, y0, color);
		}
		err -= dy;
		if (err < 0)
		{
			y0 += ystep;
			err += dx;
		}
	}
}

void LCDDriver_drawFastHLine(uint16_t x, uint16_t y, int16_t w, uint16_t color)
{
	int16_t x2 = x + w;
	if (x2 < 0)
		x2 = 0;
	if (x2 > (LCD_WIDTH - 1))
		x2 = LCD_WIDTH - 1;

	if (x2 < x)
		LCDDriver_Fill_RectXY((uint16_t)x2, y, x, y, color);
	else
		LCDDriver_Fill_RectXY(x, y, (uint16_t)x2, y, color);
}

void LCDDriver_drawFastVLine(uint16_t x, uint16_t y, int16_t h, uint16_t color)
{
	int16_t y2 = y + h - 1;
	if (y2 < 0)
		y2 = 0;
	if (y2 > (LCD_HEIGHT - 1))
		y2 = LCD_HEIGHT - 1;

	if (y2 < y)
		LCDDriver_Fill_RectXY(x, (uint16_t)y2, x, y, color);
	else
		LCDDriver_Fill_RectXY(x, y, x, (uint16_t)y2, color);
}

void LCDDriver_drawRectXY(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color)
{
	LCDDriver_drawFastHLine(x0, y0, (int16_t)(x1 - x0), color);
	LCDDriver_drawFastHLine(x0, y1, (int16_t)(x1 - x0), color);
	LCDDriver_drawFastVLine(x0, y0, (int16_t)(y1 - y0), color);
	LCDDriver_drawFastVLine(x1, y0, (int16_t)(y1 - y0), color);
}

void LCDDriver_Fill_Triangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
	int16_t a, b, y, last;

	// Sort coordinates by Y order (y2 >= y1 >= y0)
	if (y0 > y1)
	{
		_swap_int16_t(y0, y1)
		_swap_int16_t(x0, x1)
	}
	if (y1 > y2)
	{
		_swap_int16_t(y2, y1)
		_swap_int16_t(x2, x1)
	}
	if (y0 > y1)
	{
		_swap_int16_t(y0, y1)
		_swap_int16_t(x0, x1)
	}

	if (y0 == y2)
	{ // Handle awkward all-on-same-line case as its own thing
		a = b = x0;
		if (x1 < a)
			a = x1;
		else if (x1 > b)
			b = x1;
		if (x2 < a)
			a = x2;
		else if (x2 > b)
			b = x2;
		LCDDriver_drawFastHLine(a, y0, b - a + 1, color);
		return;
	}

	int16_t dx01 = x1 - x0, dy01 = y1 - y0, dx02 = x2 - x0, dy02 = y2 - y0,
			dx12 = x2 - x1, dy12 = y2 - y1;
	int32_t sa = 0, sb = 0;

	// For upper part of triangle, find scanline crossings for segments
	// 0-1 and 0-2.  If y1=y2 (flat-bottomed triangle), the scanline y1
	// is included here (and second loop will be skipped, avoiding a /0
	// error there), otherwise scanline y1 is skipped here and handled
	// in the second loop...which also avoids a /0 error here if y0=y1
	// (flat-topped triangle).
	if (y1 == y2)
		last = y1; // Include y1 scanline
	else
		last = y1 - 1; // Skip it

	for (y = y0; y <= last; y++)
	{
		a = x0 + sa / dy01;
		b = x0 + sb / dy02;
		sa += dx01;
		sb += dx02;
		/* longhand:
    a = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
    b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
    */
		if (a > b)
			_swap_int16_t(a, b)
		LCDDriver_drawFastHLine(a, y, b - a + 1, color);
	}

	// For lower part of triangle, find scanline crossings for segments
	// 0-2 and 1-2.  This loop is skipped if y1=y2.
	sa = (int32_t)dx12 * (y - y1);
	sb = (int32_t)dx02 * (y - y0);
	for (; y <= y2; y++)
	{
		a = x1 + sa / dy12;
		b = x0 + sb / dy02;
		sa += dx12;
		sb += dx02;
		/* longhand:
    a = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
    b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
    */
		if (a > b)
			_swap_int16_t(a, b)
		LCDDriver_drawFastHLine(a, y, b - a + 1, color);
	}
}

#endif
