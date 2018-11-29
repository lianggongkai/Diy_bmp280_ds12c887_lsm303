#ifndef __I2C_SOFT_H_
#define __I2C_SOFT_H_
#include "stm32f10x.h"

#define SCL_PIN	GPIO_Pin_10//PB10
#define SDA_PIN	GPIO_Pin_11//PB11
#define SCL_PORT	GPIOB
#define SDA_PORT	GPIOB
#define SCL_RCC_CLOCK	RCC_APB2Periph_GPIOB
#define SDA_RCC_CLOCK	RCC_APB2Periph_GPIOB
#define SCL_H	GPIOB->BSRR = SCL_PIN
#define SCL_L	GPIOB->BRR	= SCL_PIN

#define SDA_H	GPIOB->BSRR = SDA_PIN
#define SDA_L	GPIOB->BRR	= SDA_PIN

#define SCL_read	(GPIOB->IDR	&	SCL_PIN)
#define SDA_read	(GPIOB->IDR	&	SDA_PIN)

void I2C_GPIO_Config(void);
void I2C_Start(void);
void I2C_Stop(void);
void I2C_Ack(void);
void I2C_NoAck(void);
u8	I2C_WaitAck(void);
void I2C_SendByte(u8 SendByte);
u8	I2C_ReceiveByte(void);
	
#endif
