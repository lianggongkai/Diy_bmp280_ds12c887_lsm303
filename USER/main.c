
/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "FreeRTOS.h"
#include "task.h"
#include "lcd_driver.h"
#include "gui.h"
#include "math.h"
#include "stdlib.h"
#include "i2c_ds3231.h"
#include "ds3231.h"
#include "string.h"
#include "bme280.h"
#include "bme280_defs.h"
#include "lsm303.h"
#include "stdio.h"
#include "precall.h"
#include "i2c_soft.h"

#define SwingLedStkSize 		configMINIMAL_STACK_SIZE
#define SwingLedPriority		(tskIDLE_PRIORITY+1)

#define DrawLCDStkSize			512
#define DrawLCDPriority			(tskIDLE_PRIORITY+2)

#define ReadI2CDeviceStkSize	 256
#define ReadI2CDevicePriority	 (tskIDLE_PRIORITY+3)

#define SetCalenderStkSize		256
#define SetCalenderPriority		(tskIDLE_PRIORITY+4)

int main(void)
{
	Lcd_Init();
	I2C_GPIO_Init();
	RunLedInit();
	
	xTaskCreate(vtaskSwingLed,"SwingLed",SwingLedStkSize,NULL,SwingLedPriority,NULL);
	xTaskCreate(vtaskDrawLCD,"vDrawLCD",DrawLCDStkSize,NULL,DrawLCDPriority,NULL);
	xTaskCreate(vtaskReadI2CDevice,"vtaskReadI2CDevice",ReadI2CDeviceStkSize,NULL,ReadI2CDevicePriority,NULL);
	xTaskCreate(vtaskSetCalender,"SetCalender",SetCalenderStkSize,NULL,SetCalenderPriority,NULL);
	
	vTaskStartScheduler();
	
	return 0;
}

