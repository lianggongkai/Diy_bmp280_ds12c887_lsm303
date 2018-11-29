//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//测试硬件：单片机STM32F103RCT6,正点原子MiniSTM32开发板,主频72MHZ，晶振12MHZ
//QDtech-TFT液晶驱动 for STM32 IO模拟
//xiao冯@ShenZhen QDtech co.,LTD
//公司网站:www.qdtft.com
//淘宝网站：http://qdtech.taobao.com
//wiki技术网站：http://www.lcdwiki.com
//我司提供技术支持，任何技术问题欢迎随时交流学习
//固话(传真) :+86 0755-23594567 
//手机:15989313508（冯工） 
//邮箱:lcdwiki01@gmail.com    support@lcdwiki.com    goodtft@163.com 
//技术支持QQ:3002773612  3002778157
//技术交流QQ群:324828016
//创建日期:2018/7/28
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 深圳市全动电子技术有限公司 2018-2028
//All rights reserved
/****************************************************************************************************
//=========================================电源接线================================================//
//   液晶屏模块            STM32单片机
//      VCC        接       DC5V/3.3V      //电源
//      GND        接          GND         //电源地
//=======================================液晶屏数据线接线==========================================//
//本模块默认数据总线类型为SPI总线
//   液晶屏模块            STM32单片机    
//      SDA        接          PB15        //SPI总线数据信号
//=======================================液晶屏控制线接线==========================================//
//   液晶屏模块 					 STM32单片机 
//      LED        接          PB9         //背光控制信号，如果不需要控制，接5V或3.3V
//      SCK        接          PB13        //SPI总线时钟信号
//    A0(RS/DC)    接          PB10        //数据/命令控制信号
//      RST        接          PB12        //复位控制信号
//      CS         接          PB11        //片选控制信号
//=========================================触摸屏触接线=========================================//
//本模块不带触摸功能，不需要进行触摸屏接线
**************************************************************************************************/	
 /* @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, QD electronic SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
**************************************************************************************************/	

#ifndef _LCD_DRIVER_H_
#define _LCD_DRIVER_H_

//-------------------------屏幕物理像素设置--------------------------------------//
#define LCD_X_SIZE	        130
#define LCD_Y_SIZE	        130

/////////////////////////////////////用户配置区///////////////////////////////////	 
//支持横竖屏快速定义切换
#define USE_HORIZONTAL  		0//定义是否使用横屏 		0,不使用.1,使用.

#if USE_HORIZONTAL//如果定义了横屏 
#define X_MAX_PIXEL	        LCD_Y_SIZE
#define Y_MAX_PIXEL	        LCD_X_SIZE
#else
#define X_MAX_PIXEL	        LCD_X_SIZE
#define Y_MAX_PIXEL	        LCD_Y_SIZE
#endif
//////////////////////////////////////////////////////////////////////////////////
	 

//--------------------颜色定义---------------------------------//
#define RED  		0xf800    //红色
#define GREEN		0x07e0    //绿色
#define BLUE 		0x001f    //蓝色
#define SBLUE 	0x251F    //淡蓝色
#define WHITE		0xffff    //白色
#define BLACK		0x0000    //黑色
#define YELLOW  0xFFE0    //黄色
#define GRAY0   0xEF7D   	//灰色0 1110 1111 0111 1100 
#define GRAY1   0x8410    //灰色1 1000 0100 0001 0000
#define GRAY2   0x4208    //灰色2 0100 0010 0000 1000

//-------------------------------------------------------------------------------------
//本测试程序使用的是硬件SPI接口驱动
//SPI时钟信号引脚（LCD_SCL）和SPI数据信号引脚（LCD_SDA）不可更改，其他引脚可以任意更改
//#define LCD_CTRL   	  	GPIOB		//定义TFT数据端口
//#define LCD_LED        	GPIO_Pin_9  //PB9 连接至TFT -LED
//#define LCD_RS         	GPIO_Pin_10	//PB10连接至TFT --RS
//#define LCD_CS        	GPIO_Pin_11 //PB11 连接至TFT --CS
//#define LCD_RST     		GPIO_Pin_12	//PB12连接至TFT --RST
//VCC:可以接5V也可以接3.3V
//LED:可以接5V也可以接3.3V或者使用任意空闲IO控制(高电平使能)
//GND：接电源地
//说明：如需要尽可能少占用IO，可以将LCD_CS接地，LCD_LED接3.3V，LCD_RST接至单片机复位端，
//将可以释放3个可用IO
//如需变更IO接法，请根据您的实际接线修改相应IO初始化LCD_GPIO_Init()
//-----------------------------------------------------------------------------------------

//#define LCD_CTRL   	  	GPIOB		//定义TFT数据端口
#define LCD_LED        	GPIO_Pin_12  //MCU_PB9--->>TFT --BL
#define LCD_RS         	GPIO_Pin_14	//PB11--->>TFT --RS/DC
#define LCD_CS        	GPIO_Pin_9 //MCU_PB11--->>TFT --CS/CE
#define LCD_RST     	GPIO_Pin_8	//PB10--->>TFT --RST

//液晶控制口置1操作语句宏定义
#define	LCD_CS_SET  	GPIOA->BSRR=LCD_CS    
#define	LCD_RS_SET  	GPIOB->BSRR=LCD_RS       
#define	LCD_RST_SET  	GPIOA->BSRR=LCD_RST    
#define	LCD_LED_SET  	GPIOB->BSRR=LCD_LED   

//液晶控制口置0操作语句宏定义
#define	LCD_CS_CLR  	GPIOA->BRR=LCD_CS    
#define	LCD_RS_CLR  	GPIOB->BRR=LCD_RS      
#define	LCD_RST_CLR  	GPIOA->BRR=LCD_RST    
#define	LCD_LED_CLR  	GPIOB->BRR=LCD_LED 

#define LCD_DATAOUT(x) LCD_DATA->ODR=x; //数据输出
#define LCD_DATAIN     LCD_DATA->IDR;   //数据输入

#define LCD_WR_DATA(data){\
LCD_RS_SET;\
LCD_CS_CLR;\
LCD_DATAOUT(data);\
LCD_WR_CLR;\
LCD_WR_SET;\
LCD_CS_SET;\
} 

#include "stm32f10x.h"

void Lcd_WriteIndex(u8 Index);
void Lcd_WriteData(u8 Data);
void Lcd_WriteReg(u8 Index,u8 Data);
u16 Lcd_ReadReg(u8 LCD_Reg);
void Lcd_Init(void);
void Lcd_Clear(u16 Color);
void Lcd_SetXY(u16 x,u16 y);
void Gui_DrawPoint(u16 x,u16 y,u16 Data);
unsigned int Lcd_ReadPoint(u16 x,u16 y);
void Lcd_SetRegion(u8 x_start,u8 y_start,u8 x_end,u8 y_end);
void Lcd_WriteData_16Bit(u16 Data);

#endif
