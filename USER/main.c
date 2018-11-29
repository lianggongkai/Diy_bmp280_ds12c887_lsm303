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

#define HardwareInitStackSize	configMINIMAL_STACK_SIZE
#define HardwareInitPriority	(tskIDLE_PRIORITY+1)

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
	//int a = 1;
	char Buff[20] = {0};
	Lcd_Clear(WHITE);
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
		
		
		Gui_DrawFont_GBK16(3,0,BLACK,WHITE,(u8 *)(Buff+9));
		Gui_DrawFont_GBK1632(0,16,BLACK,GREEN,(u8 *)Buff);
		//Gui_DrawFont_Num32(0,32,RED,WHITE,9);
		//Gui_DrawFont_Num32(26,32,RED,WHITE,8);
		//Gui_DrawFont_Num32(32,32,RED,WHITE,2);
		//Gui_DrawFont_Num32(48,32,RED,WHITE,3);
		//Gui_DrawFont_Num32(0,32,RED,WHITE,0);
		//Gui_DrawLine(0,0,0,130,BLUE);
		//Gui_DrawLine(1,1,1,129,BLUE);
		//Gui_DrawLine(2,2,2,128,BLUE);
		//ShowSin((0.17f)*(a-1),WHITE);
		//ShowSin((0.17f)*(a++),BLUE);
		xTaskResumeAll();
		vTaskDelay(100);
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

