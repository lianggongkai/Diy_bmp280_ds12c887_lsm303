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

#define WAIT_ACK_OK	0	//Device ack mcu success
#define WAIT_ACK_OV	1	//MCU wait for device ack overtime

#define MCU_I2C_WR_OK 0
#define MCU_I2C_WR_OV 1

#define MCU_I2C_RD_OK	0
#define MCU_I2C_RD_OV 1

void I2C_GPIO_Config(void);
void I2C_Start(void);
void I2C_Stop(void);
void I2C_Ack(void);
void I2C_NoAck(void);
u8	I2C_WaitAck(void);
void I2C_SendByte(u8 SendByte);
u8	I2C_ReceiveByte(void);
u8 MCU_I2C_Write_Byte(u8 _7bitDevAddr,u8 regAddr,u8 Bytedata);
u8 MCU_I2C_Read_Byte(u8 _7bitDevAddr,u8 regAddr,u8 *val);
u8 MCU_I2C_Read_Bytes(u8 _7bitDevAddr,u8 regAddr,u8 len,u8 *buff);
#endif
