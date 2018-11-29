//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//����Ӳ������Ƭ��STM32F103RCT6,����ԭ��MiniSTM32������,��Ƶ72MHZ������12MHZ
//QDtech-TFTҺ������ for STM32 IOģ��
//xiao��@ShenZhen QDtech co.,LTD
//��˾��վ:www.qdtft.com
//�Ա���վ��http://qdtech.taobao.com
//wiki������վ��http://www.lcdwiki.com
//��˾�ṩ����֧�֣��κμ������⻶ӭ��ʱ����ѧϰ
//�̻�(����) :+86 0755-23594567 
//�ֻ�:15989313508���빤�� 
//����:lcdwiki01@gmail.com    support@lcdwiki.com    goodtft@163.com 
//����֧��QQ:3002773612  3002778157
//��������QQȺ:324828016
//��������:2018/7/28
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������ȫ�����Ӽ������޹�˾ 2018-2028
//All rights reserved
/****************************************************************************************************
//=========================================��Դ����================================================//
//   Һ����ģ��            STM32��Ƭ��
//      VCC        ��       DC5V/3.3V      //��Դ
//      GND        ��          GND         //��Դ��
//=======================================Һ���������߽���==========================================//
//��ģ��Ĭ��������������ΪSPI����
//   Һ����ģ��            STM32��Ƭ��    
//      SDA        ��          PB15        //SPI���������ź�
//=======================================Һ���������߽���==========================================//
//   Һ����ģ�� 					 STM32��Ƭ�� 
//      LED        ��          PB9         //��������źţ��������Ҫ���ƣ���5V��3.3V
//      SCK        ��          PB13        //SPI����ʱ���ź�
//    A0(RS/DC)    ��          PB10        //����/��������ź�
//      RST        ��          PB12        //��λ�����ź�
//      CS         ��          PB11        //Ƭѡ�����ź�
//=========================================������������=========================================//
//��ģ�鲻���������ܣ�����Ҫ���д���������
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

//-------------------------��Ļ������������--------------------------------------//
#define LCD_X_SIZE	        130
#define LCD_Y_SIZE	        130

/////////////////////////////////////�û�������///////////////////////////////////	 
//֧�ֺ��������ٶ����л�
#define USE_HORIZONTAL  		0//�����Ƿ�ʹ�ú��� 		0,��ʹ��.1,ʹ��.

#if USE_HORIZONTAL//��������˺��� 
#define X_MAX_PIXEL	        LCD_Y_SIZE
#define Y_MAX_PIXEL	        LCD_X_SIZE
#else
#define X_MAX_PIXEL	        LCD_X_SIZE
#define Y_MAX_PIXEL	        LCD_Y_SIZE
#endif
//////////////////////////////////////////////////////////////////////////////////
	 

//--------------------��ɫ����---------------------------------//
#define RED  		0xf800    //��ɫ
#define GREEN		0x07e0    //��ɫ
#define BLUE 		0x001f    //��ɫ
#define SBLUE 	0x251F    //����ɫ
#define WHITE		0xffff    //��ɫ
#define BLACK		0x0000    //��ɫ
#define YELLOW  0xFFE0    //��ɫ
#define GRAY0   0xEF7D   	//��ɫ0 1110 1111 0111 1100 
#define GRAY1   0x8410    //��ɫ1 1000 0100 0001 0000
#define GRAY2   0x4208    //��ɫ2 0100 0010 0000 1000

//-------------------------------------------------------------------------------------
//�����Գ���ʹ�õ���Ӳ��SPI�ӿ�����
//SPIʱ���ź����ţ�LCD_SCL����SPI�����ź����ţ�LCD_SDA�����ɸ��ģ��������ſ����������
//#define LCD_CTRL   	  	GPIOB		//����TFT���ݶ˿�
//#define LCD_LED        	GPIO_Pin_9  //PB9 ������TFT -LED
//#define LCD_RS         	GPIO_Pin_10	//PB10������TFT --RS
//#define LCD_CS        	GPIO_Pin_11 //PB11 ������TFT --CS
//#define LCD_RST     		GPIO_Pin_12	//PB12������TFT --RST
//VCC:���Խ�5VҲ���Խ�3.3V
//LED:���Խ�5VҲ���Խ�3.3V����ʹ���������IO����(�ߵ�ƽʹ��)
//GND���ӵ�Դ��
//˵��������Ҫ��������ռ��IO�����Խ�LCD_CS�ӵأ�LCD_LED��3.3V��LCD_RST������Ƭ����λ�ˣ�
//�������ͷ�3������IO
//������IO�ӷ������������ʵ�ʽ����޸���ӦIO��ʼ��LCD_GPIO_Init()
//-----------------------------------------------------------------------------------------

//#define LCD_CTRL   	  	GPIOB		//����TFT���ݶ˿�
#define LCD_LED        	GPIO_Pin_12  //MCU_PB9--->>TFT --BL
#define LCD_RS         	GPIO_Pin_14	//PB11--->>TFT --RS/DC
#define LCD_CS        	GPIO_Pin_9 //MCU_PB11--->>TFT --CS/CE
#define LCD_RST     	GPIO_Pin_8	//PB10--->>TFT --RST

//Һ�����ƿ���1�������궨��
#define	LCD_CS_SET  	GPIOA->BSRR=LCD_CS    
#define	LCD_RS_SET  	GPIOB->BSRR=LCD_RS       
#define	LCD_RST_SET  	GPIOA->BSRR=LCD_RST    
#define	LCD_LED_SET  	GPIOB->BSRR=LCD_LED   

//Һ�����ƿ���0�������궨��
#define	LCD_CS_CLR  	GPIOA->BRR=LCD_CS    
#define	LCD_RS_CLR  	GPIOB->BRR=LCD_RS      
#define	LCD_RST_CLR  	GPIOA->BRR=LCD_RST    
#define	LCD_LED_CLR  	GPIOB->BRR=LCD_LED 

#define LCD_DATAOUT(x) LCD_DATA->ODR=x; //�������
#define LCD_DATAIN     LCD_DATA->IDR;   //��������

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
