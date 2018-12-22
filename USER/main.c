
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

#define HardwareInitStackSize	configMINIMAL_STACK_SIZE
#define HardwareInitPriority	(tskIDLE_PRIORITY+1)
struct bme280_calib_data cal;
struct bme280_uncomp_data uncomp_data;
struct bme280_data	comp_data;
u8 PageNum = 1,ClearPage = 0;
vector running_min = {2047, 2047, 2047}, running_max = {-2048, -2048, -2048};

#define PageMax 4

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
		if(PageNum == 4){
			GPIOC->BSRR |= GPIO_Pin_13;
			vTaskDelay(1000);
			continue;
		}
		if(a ){
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
void CopyBlank(char *des,u8 strlen);

//flush calender to screen
void DispCalender(void)
{
	char DispCalender[35] = {0};
	Get_DS3231_Time();
	sprintf(DispCalender,"%02d/%02d/%02d",calendar.w_year,calendar.w_month,calendar.w_date);
	switch(calendar.week){
		case 1:strcat(DispCalender," 星期一");break;
		case 2:strcat(DispCalender," 星期二");break;
		case 3:strcat(DispCalender," 星期三");break;
		case 4:strcat(DispCalender," 星期四");break;
		case 5:strcat(DispCalender," 星期五");break;
		case 6:strcat(DispCalender," 星期六");break;
		case 7:strcat(DispCalender," 星期日");break;
		default:strcat(DispCalender," ???");break;
	}
	Gui_DrawFont_GBK16(3,0,BLACK,WHITE,(u8 *)DispCalender);
	
	sprintf(DispCalender,"%02d:%02d:%02d",calendar.hour,calendar.min,calendar.sec);
	Gui_DrawFont_GBK1632(0,16,BLACK,GREEN,(u8 *)DispCalender);
}

//flush Pressure humidity and temperature to screen
void DispPressSensor(void)
{
	#define PTHLen 17
	float temp = 0.0f;
	char DispPTH[PTHLen] = {0};
	BME280_SetOperationMode();
	ReadCalibrate(&cal);
	ReadMeasureResult(&uncomp_data);
	comp_data.humidity = compensate_humidity(&uncomp_data,&cal);
	comp_data.pressure = compensate_pressure(&uncomp_data,&cal);
	temp = compensate_temperature_f32(&uncomp_data,&cal);
	sprintf(DispPTH,"气压:%5.2f(Pa)",(float)comp_data.pressure/100);
	CopyBlank(DispPTH+strlen(DispPTH),PTHLen-strlen(DispPTH));
	Gui_DrawFont_GBK16(0,48,BLACK,WHITE,(u8 *)DispPTH);
	
	sprintf(DispPTH,"温度:%3.2f(C)",(float)temp);
	CopyBlank(DispPTH+strlen(DispPTH),PTHLen-strlen(DispPTH));
	Gui_DrawFont_GBK16(0,64,BLACK,WHITE,(u8 *)DispPTH);
	
	sprintf(DispPTH,"湿度:%4.2f(RH)",(float)comp_data.humidity/1024);
	CopyBlank(DispPTH+strlen(DispPTH),PTHLen-strlen(DispPTH));
	Gui_DrawFont_GBK16(0,80,BLACK,WHITE,(u8 *)DispPTH);
	
	sprintf(DispPTH,"RTC温度:%4.1f(C)",(float)(calendar.temper_H+calendar.temper_L/100));
	CopyBlank(DispPTH+strlen(DispPTH),PTHLen-strlen(DispPTH));
	Gui_DrawFont_GBK16(0,96,BLACK,WHITE,(u8 *)DispPTH);
	
	float Altitude = (1013.25f - (float)comp_data.pressure/100)*9;
	sprintf(DispPTH,"海拔:%4.1f(m)",Altitude);
	CopyBlank(DispPTH+strlen(DispPTH),PTHLen-strlen(DispPTH));
	Gui_DrawFont_GBK16(0,112,BLACK,WHITE,(u8 *)DispPTH);
	//calendar.temper_H
}


void CheckDispNum(void)
{
	static u8 gestureOk = 0;
	vector acc_degree ;
	LSM303Read();
	acc_degree.x = acc.x/4096 * 360;
	acc_degree.y = acc.y/4096 * 360;
	acc_degree.z = acc.z/4096 * 360;
	if(acc_degree.x > 60 && acc_degree.x < 160 && gestureOk == 0)
	{
		if(++PageNum>PageMax)PageNum = 1;
		ClearPage = 1;
		gestureOk = 1;
	}else if(acc_degree.x > 0 && acc_degree.x <20 )gestureOk = 0;
		
}

void CopyBlank(char *des,u8 strlen)
{
	u8 cnt = strlen-1;
	if(des == NULL||strlen < 1)return;
	while(cnt--){
		*(des++) = ' ';
	}
	*(des++) = '\0';
}
//flush magenetic and accelarator screen in next page
void DispMagAccSensor(void)
{
	#define MagAccLen 17
	vector acc_degree ;
	char DispMagAcc[MagAccLen] = {0};
	vector running_min = {2047, 2047, 2047}, running_max = {-2048, -2048, -2048};
	
	LSM303Read();
	
	running_min.x = min((s16)running_min.x, (s16)mag.x);
	running_min.y = min((s16)running_min.y, (s16)mag.y);
	running_min.z = min((s16)running_min.z, (s16)mag.z);

	running_max.x = max((s16)running_max.x, (s16)mag.x);
	running_max.y = max((s16)running_max.y, (s16)mag.y);
	running_max.z = max((s16)running_max.z, (s16)mag.z);
	
	acc_degree.x = acc.x/4096 * 360;
	acc_degree.y = acc.y/4096 * 360;
	acc_degree.z = acc.z/4096 * 360;
	
	Gui_DrawFont_GBK16(0,0,BLACK,WHITE,(u8 *)"三轴加速度角度:");
	sprintf(DispMagAcc,"x轴:%4.1f",acc_degree.x);
	CopyBlank(DispMagAcc+strlen(DispMagAcc),MagAccLen-strlen(DispMagAcc));
	Gui_DrawFont_GBK16(0,16,BLACK,WHITE,(u8 *)DispMagAcc);
	
	sprintf(DispMagAcc,"y轴:%4.1f",acc_degree.y);
	CopyBlank(DispMagAcc+strlen(DispMagAcc),MagAccLen-strlen(DispMagAcc));
	Gui_DrawFont_GBK16(0,32,BLACK,WHITE,(u8 *)DispMagAcc);
	
	sprintf(DispMagAcc,"z轴:%4.1f",acc_degree.z);
	CopyBlank(DispMagAcc+strlen(DispMagAcc),MagAccLen-strlen(DispMagAcc));
	Gui_DrawFont_GBK16(0,48,BLACK,WHITE,(u8 *)DispMagAcc);
	
	Gui_DrawFont_GBK16(0,64,BLACK,WHITE,(u8 *)"三轴磁场:");memset(DispMagAcc,0,MagAccLen);
	sprintf(DispMagAcc,"x轴:%5.1f",mag.x);
	CopyBlank(DispMagAcc+strlen(DispMagAcc),MagAccLen-strlen(DispMagAcc));
	Gui_DrawFont_GBK16(0,80,BLACK,WHITE,(u8 *)DispMagAcc);memset(DispMagAcc,0,MagAccLen);
	
	sprintf(DispMagAcc,"y轴:%5.1f",mag.y);
	CopyBlank(DispMagAcc+strlen(DispMagAcc),MagAccLen-strlen(DispMagAcc));
	Gui_DrawFont_GBK16(0,96,BLACK,WHITE,(u8 *)DispMagAcc);memset(DispMagAcc,0,MagAccLen);
	
	sprintf(DispMagAcc,"z轴:%5.1f",mag.z);
	CopyBlank(DispMagAcc+strlen(DispMagAcc),MagAccLen-strlen(DispMagAcc));
	Gui_DrawFont_GBK16(0,112,BLACK,WHITE,(u8 *)DispMagAcc);memset(DispMagAcc,0,MagAccLen);
	
	float yaw_angle = atan((float)mag.y / mag.x) * 180 / PI;
	//sprintf(Max,"yaw angle : %5.2f",yaw_angle);
}

void DispThreeAxis(void)
{
	#define MagAccLen 17
	vector acc_degree ;
	char DispMagAcc[MagAccLen] = {0};
	
	float Mhx = 0.0f,Mhy = 0.0f;
	
	vector from = {0,-1,0};
	m_max.x = 516;
	m_max.y = 592;
	m_max.z = 441;
	m_min.x = -445;
	m_min.y = -378;
	m_min.z = -445;
	acc_degree.x = acc.x/4096 * 360;
	acc_degree.y = acc.y/4096 * 360;
	acc_degree.z = acc.z/4096 * 360;
	
	sprintf(DispMagAcc,"横滚角:%5.1f",acc_degree.y);
	CopyBlank(DispMagAcc+strlen(DispMagAcc),MagAccLen-strlen(DispMagAcc));
	Gui_DrawFont_GBK16(0,0,RED,WHITE,(u8 *)DispMagAcc);
	
	sprintf(DispMagAcc,"俯仰角:%5.1f",acc_degree.x);
	CopyBlank(DispMagAcc+strlen(DispMagAcc),MagAccLen-strlen(DispMagAcc));
	Gui_DrawFont_GBK16(0,16,GREEN,WHITE,(u8 *)DispMagAcc);
	
	sprintf(DispMagAcc,"0航向角:%5.1f",(atan((float)mag.y / mag.x) * 180 / PI));
	CopyBlank(DispMagAcc+strlen(DispMagAcc),MagAccLen-strlen(DispMagAcc));
	Gui_DrawFont_GBK16(0,32,BLACK,WHITE,(u8 *)DispMagAcc);
	
	float Theta = acc.x/ 4096 * 2 *PI;
	float Gama = acc.y / 4096 * 2 *PI;
	
	float RealYaw = 0.0f;
	Mhx = mag.x * cos(Theta) + mag.z * sin(Theta);
	Mhy = mag.x * sin(Gama)*sin(Theta) + mag.y * cos(Gama) - mag.z * sin(Gama)*cos(Theta);
	//RealYaw = atan(Mhy/Mhx) * 180 /PI;
	RealYaw = Heading(from);
	sprintf(DispMagAcc,"1航向角:%5.1f",RealYaw);
	CopyBlank(DispMagAcc+strlen(DispMagAcc),MagAccLen-strlen(DispMagAcc));
	Gui_DrawFont_GBK16(0,48,BLACK,WHITE,(u8 *)DispMagAcc);
	
	sprintf(DispMagAcc,"2航向角:%5.1f",360-RealYaw);
	CopyBlank(DispMagAcc+strlen(DispMagAcc),MagAccLen-strlen(DispMagAcc));
	Gui_DrawFont_GBK16(0,64,BLACK,WHITE,(u8 *)DispMagAcc);
}

void DispMaxMinMag(void)
{
	#define MagAccLen 17
	
	char DispMagAcc[MagAccLen] = {0};
	
	
	LSM303Read();
	
	running_min.x = min((s16)running_min.x, (s16)mag.x);
	running_min.y = min((s16)running_min.y, (s16)mag.y);
	running_min.z = min((s16)running_min.z, (s16)mag.z);

	running_max.x = max((s16)running_max.x, (s16)mag.x);
	running_max.y = max((s16)running_max.y, (s16)mag.y);
	running_max.z = max((s16)running_max.z, (s16)mag.z);
	
	Gui_DrawFont_GBK16(0,0,RED,WHITE,(u8 *)"三轴磁场Min:");
	sprintf(DispMagAcc,"x轴:%4.1f",running_min.x);
	CopyBlank(DispMagAcc+strlen(DispMagAcc),MagAccLen-strlen(DispMagAcc));
	Gui_DrawFont_GBK16(0,16,BLACK,WHITE,(u8 *)DispMagAcc);
	
	sprintf(DispMagAcc,"y轴:%4.1f",running_min.y);
	CopyBlank(DispMagAcc+strlen(DispMagAcc),MagAccLen-strlen(DispMagAcc));
	Gui_DrawFont_GBK16(0,32,BLACK,WHITE,(u8 *)DispMagAcc);
	
	sprintf(DispMagAcc,"z轴:%4.1f",running_min.x);
	CopyBlank(DispMagAcc+strlen(DispMagAcc),MagAccLen-strlen(DispMagAcc));
	Gui_DrawFont_GBK16(0,48,BLACK,WHITE,(u8 *)DispMagAcc);
	
	Gui_DrawFont_GBK16(0,64,RED,WHITE,(u8 *)"三轴磁场Max:");
	sprintf(DispMagAcc,"x轴:%4.1f",running_max.x);
	CopyBlank(DispMagAcc+strlen(DispMagAcc),MagAccLen-strlen(DispMagAcc));
	Gui_DrawFont_GBK16(0,80,BLACK,WHITE,(u8 *)DispMagAcc);
	
	sprintf(DispMagAcc,"y轴:%4.1f",running_max.y);
	CopyBlank(DispMagAcc+strlen(DispMagAcc),MagAccLen-strlen(DispMagAcc));
	Gui_DrawFont_GBK16(0,96,BLACK,WHITE,(u8 *)DispMagAcc);
	
	sprintf(DispMagAcc,"z轴:%4.1f",running_max.z);
	CopyBlank(DispMagAcc+strlen(DispMagAcc),MagAccLen-strlen(DispMagAcc));
	Gui_DrawFont_GBK16(0,112,BLACK,WHITE,(u8 *)DispMagAcc);
}

#define DrawLCDStackSize		512
#define DrawLCDPriority			(tskIDLE_PRIORITY+2)
void vDrawLCD(void *p)
{
	Lcd_Clear(WHITE);
	ReadCalibrate(&cal);
	while(1){
		vTaskSuspendAll();
		
		if(ClearPage == 1){Lcd_Clear(WHITE);ClearPage=0;}
		if(PageNum == 1){
				DispCalender();
				DispPressSensor();
		}
		
		if(PageNum == 2){
			DispMagAccSensor();
		}
		if(PageNum == 3){
			DispThreeAxis();
		}
		
		if(PageNum == 4)GPIO_ResetBits(GPIOB,GPIO_Pin_12);
		else GPIO_SetBits(GPIOB,GPIO_Pin_12);
		
		//DispMaxMinMag();
		xTaskResumeAll();
		
		vTaskDelay(300);
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
		CheckDispNum();
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
	LSM303Enable();
	
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

