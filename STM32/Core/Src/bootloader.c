#include "bootloader.h"
#include "usb_device.h"
#include "main.h"
#include "lcd.h"
#include "functions.h"

// switch to DFU-mode buloder
void JumpToBootloader(void)
{
	uint32_t i = 0;
	void (*SysMemBootJump)(void);

	volatile uint32_t BootAddr = 0x1FFF0000;
	LCD_busy = true;
	TRX_Inited = false;
	LCD_showError("Flash DFU mode", false);
	MX_USB_DevDisconnect();
	HAL_Delay(1000);
	//prepare cpu
	HAL_MPU_Disable();
	HAL_SuspendTick();
	__disable_irq();   //Disable all interrupts
	SysTick->CTRL = 0; //Disable Systick timer
	SysTick->VAL = 0;
	SysTick->LOAD = 0;
	HAL_RCC_DeInit();		//Set the clock to the default state
	for (i = 0; i < 5; i++) //Clear Interrupt Enable Register & Interrupt Pending Register
	{
		NVIC->ICER[i] = 0xFFFFFFFF;
		NVIC->ICPR[i] = 0xFFFFFFFF;
	}
	__enable_irq(); //Re-enable all interrupts
	//go to bootloader
	SysMemBootJump = (void (*)(void))(*((uint32_t *)((BootAddr + 4)))); //-V566
	__set_MSP(*(uint32_t *)BootAddr);									//-V566
	SysMemBootJump();
	while (true)
	{
	}
}
