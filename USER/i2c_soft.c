#include "i2c_soft.h"


void I2C_GPIO_Config(void)
{
	GPIO_InitTypeDef	GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(SCL_RCC_CLOCK | SDA_RCC_CLOCK ,ENABLE);
	//初始化SCL管脚  
	GPIO_InitStructure.GPIO_Pin =SCL_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_Init(SCL_PORT, &GPIO_InitStructure);

	//初始化SDA管脚  
	GPIO_InitStructure.GPIO_Pin =SDA_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_Init(SDA_PORT, &GPIO_InitStructure);
} 

/**************************************************************************  
 * 函数名: void I2C_delay(void) 
 * 描述  : 短暂延时 
 * 输入  : 无 
 * 输出  : 无 
 * 说明  : 内部定义的i可以优化速度，经测试最低到5还能写入 
***************************************************************************/
static void I2C_delay(void)
{	
	u8 i=5; 
	while(i)
	{
		i--;
	}
} 


/************************************************************************** 
 * 函数名: void I2C_Start(void)
 * 描述  : 起始信号 
 * 输入  : 无 
 * 输出  : 无 
 * 说明  :  
***************************************************************************/
void I2C_Start(void)
{
	SCL_H;I2C_delay();
	SDA_H;I2C_delay();
	SDA_L;I2C_delay();
	SCL_L;I2C_delay();
}


/************************************************************************** 
 * 函数名: I2C_Stop(void)
 * 描述  : 终止信号 
 * 输入  : 无 
 * 输出  : 无 
 * 说明  :  
***************************************************************************/
void I2C_Stop(void)
{
	SDA_L;I2C_delay();
	SCL_H;I2C_delay();
	SDA_H;I2C_delay();
}


/************************************************************************** 
 * 函数名: void I2C_Ack(void) 
 * 描述  : 应答信号 
 * 输入  : 无 
 * 输出  : 无 
 * 说明  :  
***************************************************************************/ 
void I2C_Ack(void)
{
	SCL_L;I2C_delay();
	SDA_L;I2C_delay();
	SCL_H;I2C_delay();
	SCL_L;I2C_delay();
}

/**************************************************************************
 * 函数名: void I2C_NoAck(void) 
 * 描述  : 无应答信号 
 * 输入  : 无 
 * 输出  : 无 
 * 说明  :  
***************************************************************************/
void I2C_NoAck(void)
{
	SCL_L;I2C_delay();
	SDA_H;I2C_delay();
	SCL_H;I2C_delay();
	SCL_L;I2C_delay();
}


/**************************************************************************
 * 函数名: u8 I2C_WaitAck(void)
 * 描述  : 等待应答信号 
 * 输入  : 无 
 * 输出  : TRUE :  有应答 
       FALSE : 无应答 
 * 说明  :  
***************************************************************************/
u8 I2C_WaitAck(void)
{
	u8 ucErrTime=0;
	SCL_L;I2C_delay();
	SDA_H;I2C_delay();
	SCL_H;I2C_delay();
	while(SDA_read)
	{
		ucErrTime++;
		if(ucErrTime>250)
		{
			I2C_Stop();
			return WAIT_ACK_OV;
		}
	}
	SCL_L;
	return WAIT_ACK_OK;
}


/**************************************************************************  
 * 函数名: void I2C_SendByte(u8 SendByte)  
 * 描述  : 发送一个字节 
 * 输入  : SendByte : 字节数据 
 * 输出  : 无 
 * 说明  : 数据从高位到低位 
***************************************************************************/
void I2C_SendByte(u8 SendByte)
{
	u8 i=8;
	while(i--)
	{
		SCL_L;
		I2C_delay();
		if(SendByte & 0x80)
			SDA_H;
		else
			SDA_L;
		SendByte<<=1;
		I2C_delay(); 
		SCL_H;
		I2C_delay();
	}
	SCL_L;
}


/************************************************************************** 
 * 函数名: u8 I2C_ReceiveByte(void)  
 * 描述  : 读取一个字节 
 * 输入  : 无  
 * 输出  : 字节数据 
 * 说明  : ReceiveByte : 数据从高位到低位 
***************************************************************************/ 
u8 I2C_ReceiveByte(void)
{
	u8 i=8;
	u8 ReceiveByte=0;

	SDA_H;
	while(i--)
	{
		ReceiveByte<<=1;
		SCL_L;
		I2C_delay();
		SCL_H;
		I2C_delay();
		if(SDA_read)
		{
			ReceiveByte|=0x01;
		}
	}
	SCL_L;
	return ReceiveByte;
}

/*************************************************************************
 * 函数名  : u8 MCU_I2C_Write_Byte(u8 _7bitDevAddr,u8 regAddr,u8 Bytedata)  
 * 描述  : MCU写IIC设备 
 * 输入  : _7bitDevAddr	, 7位设备地址不带偏移和读写标志
 *				 : regAddr			, 寄存器地址
 *				 : Bytedata			, 写入寄存器的数据
 * 输出  : 成功，返回0；失败，返回1 
 * 说明  : 用户可以根据实际的IIC设备用宏函数变成自己要的函数 
*************************************************************************/
u8 MCU_I2C_Write_Byte(u8 _7bitDevAddr,u8 regAddr,u8 Bytedata)
{
	I2C_Start();
	I2C_SendByte(_7bitDevAddr<<1 | 0);
	if(I2C_WaitAck() == WAIT_ACK_OV)//Wait for ack overtime
		return MCU_I2C_WR_OV;
	
	I2C_SendByte(regAddr);
	if(I2C_WaitAck() == WAIT_ACK_OV)
		return MCU_I2C_WR_OV;
	I2C_SendByte(Bytedata);
	
	if(I2C_WaitAck() == WAIT_ACK_OV)
		return MCU_I2C_WR_OV;
	I2C_Stop();
	return MCU_I2C_WR_OK;
}

/*************************************************************************
 * 函数名  : u8 MCU_I2C_Read_Byte(u8 _7bitDevAddr,u8 regAddr,u8 *val)  
 * 描述  : MCU读取IIC设备 
 * 输入  : _7bitDevAddr	, 7位设备地址不带偏移和读写标志
 * 				 : regAddr				, 寄存器地址
 *				 : val						, 读取到的数据传出地址
 * 输出  : 成功，返回0；失败，返回1 
 * 说明  : 用户可以根据实际的IIC设备用宏函数变成自己要的函数  
*************************************************************************/
u8 MCU_I2C_Read_Byte(u8 _7bitDevAddr,u8 regAddr,u8 *val)
{
	u8 Dat=0;

	I2C_Start();
	I2C_SendByte(_7bitDevAddr<<1 | 0);
	if(I2C_WaitAck() == WAIT_ACK_OV)//Wait for ack overtime
		return MCU_I2C_RD_OV;
	
	I2C_SendByte(regAddr);
	if(I2C_WaitAck() == WAIT_ACK_OV)//Wait for ack overtime
		return MCU_I2C_RD_OV;
	
	I2C_Start();
	I2C_SendByte(_7bitDevAddr<<1 | 1);
	if(I2C_WaitAck() == WAIT_ACK_OV)//Wait for ack overtime
		return MCU_I2C_RD_OV;
	
	*val=I2C_ReceiveByte();
	I2C_Stop();
	return MCU_I2C_RD_OK;
}

/*************************************************************************
 * 函数名  : u8 MCU_I2C_Read_Bytes(u8 _7bitDevAddr,u8 regAddr,u8 len,u8 *buff) 
 * 描述  : MCU读取IIC设备 
 * 输入  : _7bitDevAddr	, 7位设备地址不带偏移和读写标志
 *				 : regAddr			, 寄存器地址
 *				 : len					, 要读取的数据长度
 *				 : buff					, 数据缓冲区地址
 * 输出  : 成功，返回0；失败，返回1 
 * 说明  : 用户可以根据实际的IIC设备用宏函数变成自己要的函数 
*************************************************************************/
u8 MCU_I2C_Read_Bytes(u8 _7bitDevAddr,u8 regAddr,u8 len,u8 *buff)
{
	u8 i = 0;
	
	I2C_Start();
	I2C_SendByte(_7bitDevAddr<<1 | 0);
	if(I2C_WaitAck() == WAIT_ACK_OV)
		return MCU_I2C_RD_OV;
	
	I2C_SendByte(regAddr);
	if(I2C_WaitAck() == WAIT_ACK_OV)
		return MCU_I2C_RD_OV;
	
	I2C_Start();
	I2C_SendByte(_7bitDevAddr<<1 | 1);
	if(I2C_WaitAck() == WAIT_ACK_OV)
		return MCU_I2C_RD_OV;
	
	while(i < len){
		buff[i++] = I2C_ReceiveByte();
		if(i < len)I2C_Ack();
		else I2C_NoAck();
	}
	I2C_Stop();
	return MCU_I2C_RD_OK;
}

