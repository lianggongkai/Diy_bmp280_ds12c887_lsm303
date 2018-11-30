/**
  ******************************************************************************
  * @file    Project/STM32F10x_StdPeriph_Template/main.c 
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */  

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
#include "stdio.h"

#define HardwareInitStackSize	configMINIMAL_STACK_SIZE
#define HardwareInitPriority	(tskIDLE_PRIORITY+1)
struct bme280_calib_data cal;
struct bme280_uncomp_data uncomp_data;
struct bme280_data	comp_data;

void vHardwareInit(void *p)
{
	GPIO_InitTypeDef a;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
	
	a.GPIO_Mode = GPIO_Mode_Out_PP;
	a.GPIO_Pin  = GPIO_Pin_13;
	a.GPIO_Speed =  GPIO_Speed_10MHz;
	
	GPIO_Init(GPIOC,&a);
	//Lcd_Init();
	vTaskDelete(NULL);
}

#define SwingLedStackSize configMINIMAL_STACK_SIZE
#define SwingLedPriority	(tskIDLE_PRIORITY+1)
void vSwingLed(void *p)
{
	unsigned char a = 0;
	while(1){
		a = 1-a;
		if(a){
			GPIOC->BSRR |= GPIO_Pin_13;
			//GPIO_SetBits(GPIOC,GPIO_Pin_13);
		}
		else {
			GPIOC->BRR |= GPIO_Pin_13;
			//GPIO_ResetBits(GPIOC,GPIO_Pin_13);
			}
		vTaskDelay(500);
	}
}

void ShowSin(float angle,u16 color)
{
	int x = 0;//x == (0,120);
	int y = 0;
	int lastx = 0,lasty = 60;
	for( x = 0 ; x <= 120; x++){
		y = (int)(30 * sin( (double)((x / 10.0f)+angle)));
		y = y - 30;
		y = abs(y)+30;
		Gui_DrawLine(lastx,lasty,x,y,color);
		Gui_DrawPoint(x,y,color);
		lastx = x;
		lasty = y;
	}
}

#define DrawLCDStackSize		512
#define DrawLCDPriority			(tskIDLE_PRIORITY+2)
void vDrawLCD(void *p)
{
	int a = 1;
	u8 id = 0;
	char Buff[20] = {0};
	char Pressure[15] = {0};
	char Temp[15] = {0};
	char Huminity[15] = {0};
	char Altitude[15] = {0};
	char RTT[15] = {0};
	float temp0 = 0.0f;
	Lcd_Clear(WHITE);
	ReadCalibrate(&cal);
	while(1){
		vTaskSuspendAll();
		//ReadDS3231(0,&sec);
		Get_DS3231_Time();
		//DecSec = (sec & 0x0f) + ((sec & 0x70)>>4)*10;
		Buff[0] = calendar.hour/10 + '0';
		Buff[1] = calendar.hour%10 + '0';
		Buff[2] = ':';
		Buff[3] = calendar.min /10 + '0';
		Buff[4] = calendar.min %10 + '0';
		Buff[5] = ':';
		Buff[6] = calendar.sec / 10 + '0';
		Buff[7] = calendar.sec % 10 + '0';
		Buff[8] = '\0';
		Buff[9] = calendar.w_year/10+'0';
		Buff[10] = calendar.w_year%10+'0';
		Buff[11] = '/';
		Buff[12] = calendar.w_month/10 + '0';
		Buff[13] = calendar.w_month%10+ '0';
		Buff[14] = '/';
		Buff[15] = calendar.w_date/10 + '0';
		Buff[16] = calendar.w_date%10+ '0';
		Buff[17] = ' ';
		switch(calendar.week){
			case 1:
				strcpy(Buff+18,"星期一");break;
			case 2:
				strcpy(Buff+18,"星期二");break;
			case 3:
				strcpy(Buff+18,"星期三");break;
			case 4:
				strcpy(Buff+18,"星期四");break;
			case 5:
				strcpy(Buff+18,"星期五");break;
			case 6:
				strcpy(Buff+18,"星期六");break;
			case 7:
				strcpy(Buff+18,"星期日");break;
			default:break;
		}
		//sprintf(ChipId,"%d",);
		id = BMP280_RD_Byte(BME280_CHIP_ID_ADDR);
		ReadCalibrate(&cal);
		ReadMeasureResult(&uncomp_data);
		comp_data.humidity = compensate_humidity(&uncomp_data,&cal);
		comp_data.pressure = compensate_pressure(&uncomp_data,&cal);
		temp0 = compensate_temperature_f32(&uncomp_data,&cal);
		BME280_SetOperationMode();
		sprintf(Pressure,"P:%5.2f(Pa)",(float)comp_data.pressure/100);
		//Pressure[strlen(Pressure)] = '\0';
		
		sprintf(Temp,"T:%5.2f(C)",temp0/2);
		//Temp[strlen(Temp)] = '\0';
		
		sprintf(Huminity,"H:%5.2f(RH)",(float)comp_data.humidity/1024);
		//Huminity[strlen(Huminity)] = '\0';
		
		sprintf(Altitude,"HB:%5.2f(m)\n",((1013.25-((float)comp_data.pressure/100))*9.0f));
		
		sprintf(RTT,"RTT:%2.2f(C)\n",(calendar.temper_H+calendar.temper_L/100.0f));
		
		Gui_DrawFont_GBK16(3,0,BLACK,WHITE,(u8 *)(Buff+9));
		Gui_DrawFont_GBK1632(0,16,BLACK,GREEN,(u8 *)Buff);
		Gui_DrawFont_GBK16(0,48,BLACK,WHITE,(u8 *)Pressure);
		Gui_DrawFont_GBK16(0,64,BLACK,WHITE,(u8 *)Temp);
		Gui_DrawFont_GBK16(0,80,BLACK,WHITE,(u8 *)Huminity);
		Gui_DrawFont_GBK16(0,96,BLACK,WHITE,(u8 *)Altitude);
		Gui_DrawFont_GBK16(0,112,BLACK,WHITE,(u8 *)RTT);
		
		xTaskResumeAll();
		if(a++ > 10)GPIO_ResetBits(GPIOB,GPIO_Pin_12);
		vTaskDelay(500);
	}
}

#define SetCalenderStkSize		256
#define SetCalenderPriority		(tskIDLE_PRIORITY+3)
Calendar_OBJ SetCalenderPara;
u16 Confirmset;
void SetCalender(void *p)
{
	while(1)
	{
		if(Confirmset != 5566){
			vTaskDelay(20);
		}else{
			Set_DS3231_Time(SetCalenderPara.w_year,SetCalenderPara.w_month,SetCalenderPara.w_date,SetCalenderPara.hour,SetCalenderPara.min,SetCalenderPara.sec,\
			SetCalenderPara.week);
			vTaskDelay(200);
			Confirmset = 0;
		}
	}
}

int main(void)
{
	Lcd_Init();
	DS3231_Init();
	
	//u8 ReadDS3231(u8 ADDR,u8 *val)
	xTaskCreate(vHardwareInit,"hardwareInit",HardwareInitStackSize,NULL,HardwareInitPriority,NULL);
	xTaskCreate(vSwingLed,"SwingLed",SwingLedStackSize,NULL,SwingLedPriority,NULL);
	xTaskCreate(vDrawLCD,"vDrawLCD",DrawLCDStackSize,NULL,DrawLCDPriority,NULL);
	xTaskCreate(SetCalender,"SetCalender",SetCalenderStkSize,NULL,SetCalenderPriority,NULL);
	vTaskStartScheduler();
	
/*	while(1){
		delay_ms(10);
		Get_DS3231_Time();
	}*/
	//for(;;);
	return 0;
}

