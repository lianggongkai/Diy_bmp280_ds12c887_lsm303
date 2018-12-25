#include "precall.h"
#include "FreeRTOS.h"
#include "task.h"
#include "stdio.h"

//BME280 calibrate data and unconpensated data and conpensated data
struct bme280_calib_data cal;
struct bme280_uncomp_data uncomp_data;
struct bme280_data	comp_data;
float  Altitude;

//BME280 magnetic min and max data recorded
vector running_min = {2047, 2047, 2047}, running_max = {-2048, -2048, -2048};

//Below is change page index
#define PageMax 4
u8 PageNum = 1,ClearPage = 0;

//LSM303 data
vector Mag,Acc;
float  YawAngle;

//DS3231 calendar data
Calendar_Type calendar;

//Set Calender data buffer and confirm to set 
Calendar_Type SetCalenderPara;
u16 Confirmset;

//Show sin wave
void ShowSin(float angle,u16 color)
{
	int x = 0,y = 0;
	int lastx = 0,lasty = 60;
	for( x = 0 ; x <= 120; x++)
	{
		y = (int)(30 * sin( (double)((x / 10.0f)+angle)));
		y = y - 30;
		y = abs(y)+30;
		Gui_DrawLine(lastx,lasty,x,y,color);
		Gui_DrawPoint(x,y,color);
		lastx = x;
		lasty = y;
	}
}

//Screen page switch
static void ScreenSwitch(void)
{
	static u8 gestureOk = 0;
	vector acc_degree ;
	
	acc_degree.x = Acc.x/4096 * 360;
	acc_degree.y = Acc.y/4096 * 360;
	acc_degree.z = Acc.z/4096 * 360;
	if(acc_degree.x > 60 && acc_degree.x < 160 && gestureOk == 0)
	{
		if(++PageNum>PageMax)PageNum = 1;
		ClearPage = 1;
		gestureOk = 1;
	}else if(acc_degree.x > 0 && acc_degree.x <20 )gestureOk = 0;
}

//Fill blank memory with space characteristic
static void CopyBlank(char *des,u8 strlen)
{
	u8 cnt = strlen-1;
	if(des == NULL||strlen < 1)return;
	while(cnt--){
		*(des++) = ' ';
	}
	*(des++) = '\0';
}

//Run led gpio init
void RunLedInit(void)
{
	GPIO_InitTypeDef a;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
	
	a.GPIO_Mode = GPIO_Mode_Out_PP;
	a.GPIO_Pin  = GPIO_Pin_13;
	a.GPIO_Speed =  GPIO_Speed_10MHz;
	
	GPIO_Init(GPIOC,&a);
}

//MCU read all i2c devicde[DS3231,LSM303DLHC,BME280]
//DS3231		: calendar chip
//LSM303DLHC: e-compass chip
//BME280 		: barometric chip
void Read_All_I2C_Device(void)
{
	//Read DS3231 get calender data
	Read_DS3231(&calendar);
	
	//Read BME280 get barometric temperature and humidity data
	ReadBME280MeasureResult(&uncomp_data);
	
	//Read LSM303 get acc data and megnetic data
	LSM303Read(&Acc,&Mag);
	
	//Calculate lsm303dlhc yaw angle
	YawAngle = LSM303Heading(Acc,Mag);
	
	//Calculate humidity,barometric and temperature
	comp_data.humidity = compensate_humidity(&uncomp_data,&cal);
	comp_data.pressure = compensate_pressure(&uncomp_data,&cal);
	comp_data.temperature = compensate_temperature_f32(&uncomp_data,&cal);
	
	//Calculate the altitude according to the barometric
	Altitude = (1013.25f - (float)comp_data.pressure/100)*9;
}

//flush calender to screen
void DispCalender(void)
{
	char DispCalender[35] = {0};
	
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
	char DispPTH[PTHLen] = {0};
	
	sprintf(DispPTH,"气压:%5.2f(Pa)",(float)comp_data.pressure/100);
	CopyBlank(DispPTH+strlen(DispPTH),PTHLen-strlen(DispPTH));
	Gui_DrawFont_GBK16(0,48,BLACK,WHITE,(u8 *)DispPTH);
	
	sprintf(DispPTH,"温度:%3.2f(C)",(float)comp_data.temperature);
	CopyBlank(DispPTH+strlen(DispPTH),PTHLen-strlen(DispPTH));
	Gui_DrawFont_GBK16(0,64,BLACK,WHITE,(u8 *)DispPTH);
	
	sprintf(DispPTH,"湿度:%4.2f(RH)",(float)comp_data.humidity/1024);
	CopyBlank(DispPTH+strlen(DispPTH),PTHLen-strlen(DispPTH));
	Gui_DrawFont_GBK16(0,80,BLACK,WHITE,(u8 *)DispPTH);
	
	sprintf(DispPTH,"海拔:%4.1f(m)",Altitude);
	CopyBlank(DispPTH+strlen(DispPTH),PTHLen-strlen(DispPTH));
	Gui_DrawFont_GBK16(0,96,BLACK,WHITE,(u8 *)DispPTH);
	
	sprintf(DispPTH,"航向:%4.0f",YawAngle);
	CopyBlank(DispPTH+strlen(DispPTH),PTHLen-strlen(DispPTH));
	Gui_DrawFont_GBK16(0,112,BLACK,WHITE,(u8 *)DispPTH);
	
}

//Display magenetic and accelarator data
void DispMagAccSensor(void)
{
	#define MagAccLen 17
	char DispMagAcc[MagAccLen] = {0};
	
	Gui_DrawFont_GBK16(0,0,BLACK,WHITE,(u8 *)"三轴加速度角度:");
	sprintf(DispMagAcc,"x轴:%4.1f",Acc.x);
	CopyBlank(DispMagAcc+strlen(DispMagAcc),MagAccLen-strlen(DispMagAcc));
	Gui_DrawFont_GBK16(0,16,BLACK,WHITE,(u8 *)DispMagAcc);
	
	sprintf(DispMagAcc,"y轴:%4.1f",Acc.y);
	CopyBlank(DispMagAcc+strlen(DispMagAcc),MagAccLen-strlen(DispMagAcc));
	Gui_DrawFont_GBK16(0,32,BLACK,WHITE,(u8 *)DispMagAcc);
	
	sprintf(DispMagAcc,"z轴:%4.1f",Acc.z);
	CopyBlank(DispMagAcc+strlen(DispMagAcc),MagAccLen-strlen(DispMagAcc));
	Gui_DrawFont_GBK16(0,48,BLACK,WHITE,(u8 *)DispMagAcc);
	
	Gui_DrawFont_GBK16(0,64,BLACK,WHITE,(u8 *)"三轴磁场:");memset(DispMagAcc,0,MagAccLen);
	sprintf(DispMagAcc,"x轴:%5.1f",Mag.x);
	CopyBlank(DispMagAcc+strlen(DispMagAcc),MagAccLen-strlen(DispMagAcc));
	Gui_DrawFont_GBK16(0,80,BLACK,WHITE,(u8 *)DispMagAcc);memset(DispMagAcc,0,MagAccLen);
	
	sprintf(DispMagAcc,"y轴:%5.1f",Mag.y);
	CopyBlank(DispMagAcc+strlen(DispMagAcc),MagAccLen-strlen(DispMagAcc));
	Gui_DrawFont_GBK16(0,96,BLACK,WHITE,(u8 *)DispMagAcc);memset(DispMagAcc,0,MagAccLen);
	
	sprintf(DispMagAcc,"z轴:%5.1f",Mag.z);
	CopyBlank(DispMagAcc+strlen(DispMagAcc),MagAccLen-strlen(DispMagAcc));
	Gui_DrawFont_GBK16(0,112,BLACK,WHITE,(u8 *)DispMagAcc);memset(DispMagAcc,0,MagAccLen);
	
}

//Disp Max and Min magnetic data
void DispMaxMinMag(void)
{
	#define MagAccLen 17
	char DispMagAcc[MagAccLen] = {0};
	vector running_min = {2047, 2047, 2047}, running_max = {-2048, -2048, -2048};
	
	running_min.x = min((s16)running_min.x, (s16)Mag.x);
	running_min.y = min((s16)running_min.y, (s16)Mag.y);
	running_min.z = min((s16)running_min.z, (s16)Mag.z);

	running_max.x = max((s16)running_max.x, (s16)Mag.x);
	running_max.y = max((s16)running_max.y, (s16)Mag.y);
	running_max.z = max((s16)running_max.z, (s16)Mag.z);
	
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

//
void vtaskSetCalender(void *p)
{
	while(1)
	{
		ScreenSwitch();
		if(Confirmset != 5566){
			vTaskDelay(20);
		}else{
			Set_DS3231_Time(SetCalenderPara.w_year,SetCalenderPara.w_month,SetCalenderPara.w_date,\
											SetCalenderPara.hour,SetCalenderPara.min,SetCalenderPara.sec,\
											SetCalenderPara.week);
			vTaskDelay(200);
			Confirmset = 0;
		}
	}
}

void vtaskSwingLed(void *p)
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

void vtaskReadI2CDevice(void *p)
{
	//I2C Device init mode
	DS3231_Init();
	LSM303Enable();
	SetBME280OperationMode();
	
	//Read calibrate data
	ReadBME280Calibrate(&cal);
	while(1)
	{
		vTaskSuspendAll();
		Read_All_I2C_Device();
		xTaskResumeAll();
		vTaskDelay(100);
	}
}

void vtaskDrawLCD(void *p)
{
	Lcd_Clear(WHITE);
	while(1)
	{
		//According x-axis angle switch screen
		//First Clear Page
		if(ClearPage == 1)
		{
			Lcd_Clear(WHITE);
			BackLightOpen();
			ClearPage=0;
		}
		
		//Second according page num show content
		switch(PageNum)
		{
			case 1:
				DispCalender();
				DispPressSensor();
				break;
			
			case 2:
				DispMagAccSensor();
				break;
			
			case 3:
				break;
			
			case 4:
				BackLightClose();break;
			
			default:break;
		}
		vTaskDelay(300);
	}
}

