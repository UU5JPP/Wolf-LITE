#ifndef LCD_COLOR_THEMES_h
#define LCD_COLOR_THEMES_h

#include "lcd_driver.h"

typedef const struct
{
	const uint16_t BACKGROUND;
	const uint16_t FOREGROUND;
	const uint16_t BUTTON_TEXT;
	const uint16_t BUTTON_INACTIVE_TEXT;
	const uint16_t BUTTON_BACKGROUND;
	const uint16_t BUTTON_EDGING;
	const uint16_t FREQ_MHZ;
	const uint16_t FREQ_KHZ;
	const uint16_t FREQ_HZ;
	const uint16_t STATUS_BAR_VOL;
	const uint16_t STATUS_VFO;
	const uint16_t STATUS_SWR;
	const uint16_t STATUS_VFO_BG;
	const uint16_t STATUS_BAR_LEFT;
	const uint16_t STATUS_BAR_RIGHT;
	const uint16_t STATUS_BAR_LABELS;
	const uint16_t STATUS_RX;
	const uint16_t STATUS_TX;
	const uint16_t STATUS_LABELS_BW;
	const uint16_t STATUS_LABELS_TX;
	const uint16_t STATUS_LABEL_S_VAL;
	const uint16_t STATUS_LABEL_DBM;
	const uint16_t STATUS_LABEL_BW;
	const uint16_t STATUS_LABEL_RIT;
	const uint16_t STATUS_LABEL_VLT;
	const uint16_t STATUS_LABEL_PTT_SW1;
	const uint16_t STATUS_LABEL_CPU;
	const uint16_t STATUS_LABEL_PWM;
	const uint16_t STATUS_LABEL_THERM;
	const uint16_t STATUS_LABEL_NOTCH;
	const uint16_t STATUS_SMETER;
	const uint16_t STATUS_SMETER_STRIPE;
	const uint16_t STATUS_SMETER_PEAK;
	const uint16_t STATUS_MODE;
	const uint16_t STATUS_ERR;
	const uint16_t BANDMAP_CW;
	const uint16_t BANDMAP_SSB;
	const uint16_t BANDMAP_DIGI;
	const uint16_t BANDMAP_AM;
	const uint16_t BANDMAP_FM;
	const uint16_t GREETINGS;
	const uint16_t TOOLTIP_FORE;
	const uint16_t TOOLTIP_BACK;
	const uint16_t TOOLTIP_BORD;
	const uint16_t CLOCK;
	const uint16_t FFT_GRADIENT_START_R;
	const uint16_t FFT_GRADIENT_START_G;
	const uint16_t FFT_GRADIENT_START_B;
	const uint16_t FFT_GRADIENT_END_R;
	const uint16_t FFT_GRADIENT_END_G;
	const uint16_t FFT_GRADIENT_END_B;
	const uint16_t BOTTOM_BUTTONS_COLOR;
	const bool WTF_BG_WHITE;
	const uint16_t BW_TRAPEZ_BORDER;
	const uint16_t BW_TRAPEZ_STRIPE;
	const uint16_t BW_TRAPEZ_FILL;
} STRUCT_COLOR_THEME;

static const STRUCT_COLOR_THEME COLOR_THEMES[2] = {
	//0 - black
	{
		//3'2 inch
		.BACKGROUND 					= COLOR_BLACK,
		.FOREGROUND 					= COLOR_WHITE,
		.BUTTON_TEXT 					= rgb888torgb565(32, 191, 17),
		.BUTTON_INACTIVE_TEXT = COLOR_WHITE,//rgb888torgb565(130, 130, 130),
		.BUTTON_BACKGROUND  	= rgb888torgb565(15,15,15),
		.BUTTON_EDGING		 	  = COLOR_RED,
		.FREQ_MHZ 					  = COLOR_WHITE,
		.FREQ_KHZ 						= COLOR_WHITE,
		.FREQ_HZ					 	  = rgb888torgb565(150, 150, 150),
		.STATUS_VFO				    = COLOR_BLACK,
		.STATUS_SWR 				  = COLOR_RED,
		.STATUS_VFO_BG 		  	= COLOR_WHITE,
		.STATUS_BAR_VOL 		  = rgb888torgb565(0, 255, 255),
		.STATUS_BAR_LEFT 		  = rgb888torgb565(100, 100, 255),
		.STATUS_BAR_RIGHT 	  = rgb888torgb565(248, 13, 13),
		.STATUS_BAR_LABELS 	  = rgb888torgb565(32, 171, 17),
		.STATUS_RX 					  = COLOR_WHITE,
		.STATUS_TX				   	= COLOR_RED,
		.STATUS_LABELS_BW	 	  = COLOR_WHITE,
		.STATUS_LABELS_TX		  = rgb888torgb565(32, 171, 17),
		.STATUS_LABEL_S_VAL   = rgb888torgb565(24, 205, 46),
		.STATUS_LABEL_DBM 		= rgb888torgb565(32, 191, 17),
		.STATUS_LABEL_BW 			= rgb888torgb565(0, 200, 255),
		.STATUS_LABEL_RIT 		= COLOR_WHITE,
		.STATUS_LABEL_VLT 		= rgb888torgb565(249, 205, 46),
		.STATUS_LABEL_PTT_SW1 = rgb888torgb565(249, 205, 46),
		.STATUS_LABEL_CPU 		= rgb888torgb565(255, 0, 0),
		.STATUS_LABEL_PWM 		= rgb888torgb565(255, 0, 0),
		.STATUS_LABEL_THERM	  = rgb888torgb565(249, 205, 46),
		.STATUS_LABEL_NOTCH   = rgb888torgb565(0, 200, 255),
		.STATUS_SMETER			  = rgb888torgb565(249, 205, 46),
		.STATUS_SMETER_STRIPE = COLOR_RED,
		.STATUS_SMETER_PEAK   = rgb888torgb565(249, 151, 46),
		.STATUS_MODE 					= rgb888torgb565(249, 205, 46),
		.STATUS_ERR 					= COLOR_RED,
		.BANDMAP_CW 					= rgb888torgb565(50, 50, 255),
		.BANDMAP_SSB 					= rgb888torgb565(50, 237, 255),
		.BANDMAP_DIGI 				= rgb888torgb565(255, 50, 50),
		.BANDMAP_AM 					= rgb888torgb565(219, 255, 50),
		.BANDMAP_FM 					= rgb888torgb565(255, 50, 208),
		.GREETINGS 						= rgb888torgb565(0, 92, 86),
		.TOOLTIP_FORE				  = COLOR_WHITE,
		.TOOLTIP_BACK 				= COLOR_BLACK,
		.TOOLTIP_BORD 				= COLOR_WHITE,
		.CLOCK 								= COLOR_WHITE,
		.FFT_GRADIENT_START_R = 0,
		.FFT_GRADIENT_START_G = 11,
		.FFT_GRADIENT_START_B = 40,
		.FFT_GRADIENT_END_R 	= 46,
		.FFT_GRADIENT_END_G		= 77,
		.FFT_GRADIENT_END_B	  = 158,
		.WTF_BG_WHITE				  = false,
		.BOTTOM_BUTTONS_COLOR = rgb888torgb565(249, 205, 46),
		.BW_TRAPEZ_BORDER     = rgb888torgb565(120, 120, 120),
		.BW_TRAPEZ_STRIPE 		= rgb888torgb565(0, 200, 255),
		.BW_TRAPEZ_FILL 			= rgb888torgb565(140, 140, 140),
	},
	//1 - white
	{
		//3'2 inch
		.BACKGROUND 					= COLOR_BLACK,
		.FOREGROUND 					= COLOR_WHITE,
		.BUTTON_TEXT 					= COLOR_GREEN, //rgb888torgb565(50, 200, 0),//green
		.BUTTON_INACTIVE_TEXT = COLOR_WHITE,//rgb888torgb565(130, 130, 130),
		.BUTTON_BACKGROUND 		= rgb888torgb565(15,15,15),//(54, 54, 54),//dark grey
		.BUTTON_EDGING		 	  = COLOR_RED,
		.FREQ_MHZ						  = rgb888torgb565(10,253,0),//COLOR_WHITE,
		.FREQ_KHZ 						= rgb888torgb565(10,253,0),//COLOR_WHITE,
		.FREQ_HZ 							= rgb888torgb565(0,255,168),//rgb888torgb565(150, 150, 150),
		.STATUS_VFO					  = COLOR_BLACK,
		.STATUS_VFO_BG 				= COLOR_WHITE,
		.STATUS_BAR_LEFT 			= rgb888torgb565(100, 100, 255),
		.STATUS_BAR_RIGHT		  = rgb888torgb565(248, 13, 13),
		.STATUS_BAR_LABELS 		= rgb888torgb565(32, 171, 17),
		.STATUS_RX					  = COLOR_WHITE,
		.STATUS_TX 						= COLOR_RED,
		.STATUS_LABELS_BW 		= COLOR_WHITE,
		.STATUS_LABELS_TX 		= rgb888torgb565(32, 171, 17),
		.STATUS_LABEL_S_VAL	  = rgb888torgb565(249, 205, 46),
		.STATUS_LABEL_DBM 		= rgb888torgb565(32, 191, 17),
		.STATUS_LABEL_BW 			= rgb888torgb565(0, 200, 255),
		.STATUS_LABEL_RIT 		= COLOR_WHITE,
		.STATUS_LABEL_VLT 		= rgb888torgb565(249, 205, 46),
		.STATUS_LABEL_CPU 		= rgb888torgb565(255, 0, 0),
		.STATUS_LABEL_PWM 		= rgb888torgb565(255, 0, 0),
		.STATUS_LABEL_THERM 	= rgb888torgb565(249, 205, 46),
		.STATUS_LABEL_NOTCH 	= rgb888torgb565(0, 200, 255),
		.STATUS_SMETER 				= rgb888torgb565(249, 205, 46),
		.STATUS_SMETER_STRIPE = COLOR_RED,
		.STATUS_SMETER_PEAK	  = rgb888torgb565(249, 151, 46),
		.STATUS_MODE 					= rgb888torgb565(249, 205, 46),
		.STATUS_ERR 					= COLOR_RED,
		.BANDMAP_CW 					= rgb888torgb565(50, 50, 255),
		.BANDMAP_SSB 					= rgb888torgb565(50, 237, 255),
		.BANDMAP_DIGI 				= rgb888torgb565(255, 50, 50),
		.BANDMAP_AM 					= rgb888torgb565(219, 255, 50),
		.BANDMAP_FM 					= rgb888torgb565(255, 50, 208),
		.GREETINGS 						= rgb888torgb565(0, 92, 86),
		.TOOLTIP_FORE				  = COLOR_WHITE,
		.TOOLTIP_BACK 				= COLOR_BLACK,
		.TOOLTIP_BORD 				= COLOR_WHITE,
		.CLOCK 								= COLOR_WHITE,
		.FFT_GRADIENT_START_R = 0,
		.FFT_GRADIENT_START_G = 40,//11,
		.FFT_GRADIENT_START_B = 0,//40,
		.FFT_GRADIENT_END_R 	= 250,//46,
		.FFT_GRADIENT_END_G 	= 22,//77,
		.FFT_GRADIENT_END_B 	= 7,//158,
		.WTF_BG_WHITE 	 			= false,
		.BOTTOM_BUTTONS_COLOR = rgb888torgb565(249, 205, 46),
		.BW_TRAPEZ_BORDER 		= rgb888torgb565(120, 120, 120),
		.BW_TRAPEZ_STRIPE 		= rgb888torgb565(0, 200, 255),
		.BW_TRAPEZ_FILL 			= rgb888torgb565(140, 140, 140),
	},
};

#define COLOR_THEMES_COUNT 2
#define BG_COLOR COLOR->BACKGROUND
#define FG_COLOR COLOR->FOREGROUND

//LCD dimensions defines
#include "screen_layout.h"

#endif
