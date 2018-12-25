
#include "ds3231.h"
#include "i2c_soft.h"

//Calendar_OBJ calendar;

#define DS3231_WriteAddress	0xD0	
#define DS3231_ReadAddress	0xD1

u8 BCD2HEX(u8 val)
{
	u8 i;
	i= val&0x0f;
	val >>= 4;
	val &= 0x0f;
	val *= 10;
	i += val;

	return i;
}


u16 B_BCD(u8 val)
{
	u8 i,j,k;
	i=val/10;
	j=val%10;
	k=j+(i<<4);
	return k;
}

void DS3231_WR_Byte(u8 addr,u8 bytedata)
{
	I2C_Start();
	I2C_SendByte(DS3231_WriteAddress);
	I2C_WaitAck();
	I2C_SendByte(addr);
	I2C_WaitAck();
	I2C_SendByte(bytedata);
	I2C_WaitAck();
	I2C_Stop();
}

u8 DS3231_RD_Byte(u8 addr)
{
	u8 Dat=0;

	I2C_Start();
	I2C_SendByte(DS3231_WriteAddress);
	if(I2C_WaitAck() == WAIT_ACK_OV)return Dat;
	I2C_SendByte(addr);
	if(I2C_WaitAck() == WAIT_ACK_OV)return Dat;
	I2C_Start();
	I2C_SendByte(DS3231_ReadAddress);
	if(I2C_WaitAck() == WAIT_ACK_OV)return Dat;
	Dat=I2C_ReceiveByte();
	I2C_Stop();

	return Dat;
}

void delay_ms(u16 x)
{
	int y = 12768;
	while(x-->0){
		while(y-->0);
	}
}

void DS3231_Init(void)
{
	DS3231_WR_Byte(0x0e,0);
	delay_ms(2);
	DS3231_WR_Byte(0x0f,0x0);
	delay_ms(2);
}

void Set_DS3231_Time(u8 yea,u8 mon,u8 da,u8 hou,u8 min,u8 sec,u8 week)
{
	u8 temp=0;
	temp=B_BCD(yea);
	DS3231_WR_Byte(0x06,temp);

	temp=B_BCD(mon);
	DS3231_WR_Byte(0x05,temp);

	temp=B_BCD(da);
	DS3231_WR_Byte(0x04,temp);

	temp=B_BCD(hou);
	DS3231_WR_Byte(0x02,temp);

	temp=B_BCD(min);
	DS3231_WR_Byte(0x01,temp);

	temp=B_BCD(sec);
	DS3231_WR_Byte(0x00,temp);

	temp=B_BCD(week);
	DS3231_WR_Byte(0x03,temp);
}


void Read_DS3231(Calendar_Type *calendar)
{
	calendar->w_year=DS3231_RD_Byte(0x06);
	calendar->w_year=BCD2HEX(calendar->w_year);

	calendar->w_month=DS3231_RD_Byte(0x05); 
	calendar->w_month=BCD2HEX(calendar->w_month);

	calendar->w_date=DS3231_RD_Byte(0x04);
	calendar->w_date=BCD2HEX(calendar->w_date);
	 
	calendar->hour=DS3231_RD_Byte(0x02); 
	calendar->hour&=0x3f;
	calendar->hour=BCD2HEX(calendar->hour);

	calendar->min=DS3231_RD_Byte(0x01);
	calendar->min=BCD2HEX(calendar->min);

	calendar->sec=DS3231_RD_Byte(0x00);
	calendar->sec=BCD2HEX(calendar->sec);

	calendar->week=DS3231_RD_Byte(0x03);
	calendar->week=BCD2HEX(calendar->week);

	DS3231_WR_Byte(0x0e,0x20);
	calendar->temper_H=DS3231_RD_Byte(0x11);
	calendar->temper_L=(DS3231_RD_Byte(0x12)>>6)*25;
}




