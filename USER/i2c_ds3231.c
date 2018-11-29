#include "stm32f10x.h"
 
#define HIGH 1
#define LOW 0
 
#define TRUE  1
#define FALSE 0
 
//PB10 --> SCL
//PB11 --> SDA
#define SCL_Pin GPIO_Pin_10
#define SDA_Pin GPIO_Pin_11

#define DS3231_ADDR 0x68
//#define WR_DS3231_ADDR (DS3231_ADDR<<1)
//#define RD_DS3231_ADDR (WR_DS3231_ADDR|1)
#define WR_DS3231_ADDR 0x32
#define RD_DS3231_ADDR 0X31

#define SDA_IN()  {/*GPIOB->CRL &= 0XFFFF0FFF; GPIOB->CRL |= 4<<12;*/}
#define SDA_OUT() {/*GPIOB->CRL &= 0XFFFF0FFF; GPIOB->CRL |= 3<<12;*/}

#define IIC_SCL(n)    {if(n)GPIOB->BSRR= SCL_Pin;else GPIOB->BRR= SCL_Pin;} //SCL
#define IIC_SDA(n)    {if(n)GPIOB->BSRR= SDA_Pin;else GPIOB->BRR= SDA_Pin;} //SDA	 
#define READ_SDA   		(GPIOB->IDR & SDA_Pin)  //??SDA
 
void delay_us(u16 time)
{    
   u16 i = 0;  
   while(time--)
   {
      i = 10;    
      while(i--);    
   }
}

void IIC_Init(void)
{
	GPIO_InitTypeDef GPIO_Init_Structure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	
	GPIO_Init_Structure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
	GPIO_Init_Structure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init_Structure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_Init_Structure);
	GPIO_SetBits(GPIOB,GPIO_Pin_10|GPIO_Pin_11);
}

void IIC_Start(void)
{
	SDA_OUT();        
	IIC_SDA(HIGH);	  	  
	IIC_SCL(HIGH);
    
	delay_us(5);
    
 	IIC_SDA(LOW);
	delay_us(5);
	IIC_SCL(LOW);
}
 
void IIC_Stop(void)
{
	SDA_OUT();
	
	IIC_SCL(LOW);
	IIC_SDA(LOW);
    
 	delay_us(5);
	
	IIC_SCL(HIGH); 
	IIC_SDA(HIGH);
	
	delay_us(5);							   	
}

u8 IIC_Wait_Ack(void)
{
	u8 ucErrTime = 0; 
	IIC_SDA(HIGH);
	delay_us(5);

	SDA_IN();

	IIC_SCL(HIGH);
	delay_us(5); 	
		
	while(READ_SDA)
	{
		ucErrTime++;
		if(ucErrTime > 250)
		{
			return FALSE;
		}
	}

	IIC_SCL(LOW);
	return TRUE;  
}
 

void IIC_Ack(void)
{

	IIC_SCL(LOW);
    
  SDA_OUT();
	IIC_SDA(LOW);
    
	delay_us(5);
	IIC_SCL(HIGH);
    
	delay_us(5);
	IIC_SCL(LOW);
}

void IIC_NAck(void)
{
	IIC_SCL(LOW);
	SDA_OUT();
	IIC_SDA(HIGH);
	delay_us(5);
	IIC_SCL(HIGH);
	delay_us(5);
	IIC_SCL(LOW);
}
 
void IIC_Send_Byte(u8 txd)
{                        
	u8 t;    
	SDA_OUT(); 	 
	IIC_SCL(LOW);
	for(t = 0; t < 8; t++)
	{     
		if((txd&0x80)){
			IIC_SDA(HIGH);
		}
		else{
			IIC_SDA(LOW);
		}
			
		txd <<= 1;       
		delay_us(5); 
		IIC_SCL(HIGH);
				
		delay_us(5);
		IIC_SCL(LOW);
		
		delay_us(5);
	}	 
} 
 
u8 IIC_Read_Byte(void)
{
	unsigned char i, receive = 0;
	SDA_IN();
	for(i = 0; i < 8; i++ )
	{
		IIC_SCL(LOW); 
		delay_us(5);
		IIC_SCL(HIGH);
		receive = (receive << 1) | READ_SDA;
		delay_us(5); 
	}
	return receive;
}

u8 ReadDS3231(u8 ADDR,u8 *val)
{
	IIC_Start();
	IIC_Send_Byte(WR_DS3231_ADDR);
	
	if(IIC_Wait_Ack()==FALSE){
			IIC_Stop();
			return FALSE;
	}
	
	IIC_Send_Byte(ADDR);
	if(IIC_Wait_Ack()==FALSE){
			IIC_Stop();
			return FALSE;
	}
	
	IIC_Start();
	IIC_Send_Byte(RD_DS3231_ADDR);
	if(IIC_Wait_Ack()==FALSE){
			IIC_Stop();
			return FALSE;
	}
	*val = IIC_Read_Byte();
	IIC_Ack();
	IIC_Stop();
	return TRUE;
}
