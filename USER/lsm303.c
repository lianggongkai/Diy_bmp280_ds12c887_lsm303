#include "lsm303.h"
#include "i2c_soft.h"
#include "math.h"

typedef enum
{
	LSM303_MAGGAIN_1_3                        = 0x20,  // +/- 1.3
	LSM303_MAGGAIN_1_9                        = 0x40,  // +/- 1.9
	LSM303_MAGGAIN_2_5                        = 0x60,  // +/- 2.5
	LSM303_MAGGAIN_4_0                        = 0x80,  // +/- 4.0
	LSM303_MAGGAIN_4_7                        = 0xA0,  // +/- 4.7
	LSM303_MAGGAIN_5_6                        = 0xC0,  // +/- 5.6
	LSM303_MAGGAIN_8_1                        = 0xE0   // +/- 8.1
} lsm303MagGain;
		
typedef enum
{
	LSM303_REGISTER_MAG_CRA_REG_M             = 0x00,
	LSM303_REGISTER_MAG_CRB_REG_M             = 0x01,
	LSM303_REGISTER_MAG_MR_REG_M              = 0x02,
	LSM303_REGISTER_MAG_OUT_X_H_M             = 0x03,
	LSM303_REGISTER_MAG_OUT_X_L_M             = 0x04,
	LSM303_REGISTER_MAG_OUT_Z_H_M             = 0x05,
	LSM303_REGISTER_MAG_OUT_Z_L_M             = 0x06,
	LSM303_REGISTER_MAG_OUT_Y_H_M             = 0x07,
	LSM303_REGISTER_MAG_OUT_Y_L_M             = 0x08,
	LSM303_REGISTER_MAG_SR_REG_Mg             = 0x09,
	LSM303_REGISTER_MAG_IRA_REG_M             = 0x0A,
	LSM303_REGISTER_MAG_IRB_REG_M             = 0x0B,
	LSM303_REGISTER_MAG_IRC_REG_M             = 0x0C,
	LSM303_REGISTER_MAG_TEMP_OUT_H_M          = 0x31,
	LSM303_REGISTER_MAG_TEMP_OUT_L_M          = 0x32
} lsm303MagRegisters_t;

vector m_max,m_min;

void writeAccReg(u8 reg,u8 value)
{
	I2C_Start();
	I2C_SendByte(acc_address_write);
	I2C_WaitAck();
	I2C_SendByte(reg);
	I2C_WaitAck();
	I2C_SendByte(value);
	I2C_WaitAck();
	I2C_Stop();
}

u8 readAccReg(u8 reg)
{
	u8 Dat = 0;
	I2C_Start();
	I2C_SendByte(acc_address_write);
	I2C_WaitAck();
	
	I2C_SendByte(reg);
	I2C_WaitAck();
	I2C_Start();
	I2C_SendByte(acc_address_read);
	I2C_WaitAck();
	Dat=I2C_ReceiveByte();
	I2C_NoAck();
	I2C_Stop();
	return Dat;
}

void writeMagReg(u8 reg, u8 value)
{
	I2C_Start();
	I2C_SendByte(MAG_ADDRESS_WRITE);
	I2C_WaitAck();
	I2C_SendByte(reg);
	I2C_WaitAck();
	I2C_SendByte(value);
	I2C_WaitAck();
	I2C_Stop();
}


u8 readMagReg(int reg)
{
  u8 value;
	u8 _device = LSM303DLHC_DEVICE;
  
  // if dummy register address (magnetometer Y/Z), use device type to determine actual address
  if (reg < 0)
  {
    switch (reg)
    {
      case LSM303_OUT_Y_H_M:
        reg = (_device == LSM303DLH_DEVICE) ? LSM303DLH_OUT_Y_H_M : LSM303DLM_OUT_Y_H_M;
        break;
      case LSM303_OUT_Y_L_M:
        reg = (_device == LSM303DLH_DEVICE) ? LSM303DLH_OUT_Y_L_M : LSM303DLM_OUT_Y_L_M;
        break;
      case LSM303_OUT_Z_H_M:
        reg = (_device == LSM303DLH_DEVICE) ? LSM303DLH_OUT_Z_H_M : LSM303DLM_OUT_Z_H_M;
        break;
      case LSM303_OUT_Z_L_M:
        reg = (_device == LSM303DLH_DEVICE) ? LSM303DLH_OUT_Z_L_M : LSM303DLM_OUT_Z_L_M;
        break;
    }
  }
	
	I2C_Start();
	I2C_SendByte(MAG_ADDRESS_WRITE);
	I2C_WaitAck();
	
	I2C_SendByte(reg);
	I2C_WaitAck();
	I2C_Start();
	I2C_SendByte(MAG_ADDRESS_READ);
	I2C_WaitAck();
	value=I2C_ReceiveByte();
	I2C_NoAck();
	I2C_Stop();
  
  return value;
}

void setMagGain(magGain value)
{
	writeMagReg(LSM303_CRB_REG_M,value);
}

void readAcc(vector *acc)
{
	I2C_Start();
	I2C_SendByte(acc_address_write);
	I2C_WaitAck();
	
	I2C_SendByte(LSM303_OUT_X_L_A | (1 << 7));
	I2C_WaitAck();
	
	I2C_Start();
	I2C_SendByte(acc_address_read);
	I2C_WaitAck();
	
	u8 xla = I2C_ReceiveByte();
	I2C_Ack();
	u8 xha = I2C_ReceiveByte();
	I2C_Ack();
	u8 yla = I2C_ReceiveByte();
	I2C_Ack();
	u8 yha = I2C_ReceiveByte();
	I2C_Ack();
	u8 zla = I2C_ReceiveByte();
	I2C_Ack();
	u8 zha = I2C_ReceiveByte();
	
	I2C_NoAck();
	I2C_Stop();
	
	acc->x = ((s16)(xha<<8 | xla)) >>4 ;
	acc->y = ((s16)(yha<<8 | yla)) >>4;
	acc->z = ((s16)(zha<<8 | zla)) >>4;
}

void readMag(vector *mag)
{
	
	I2C_Start();
	I2C_SendByte(MAG_ADDRESS_WRITE);
	I2C_WaitAck();
	
	I2C_SendByte(LSM303_OUT_X_H_M);
	I2C_WaitAck();
	
	I2C_Start();
	I2C_SendByte(MAG_ADDRESS_READ);
	I2C_WaitAck();
	
	u8 xhm = I2C_ReceiveByte();
	I2C_Ack();
	u8 xlm = I2C_ReceiveByte();
	I2C_Ack();
	u8 zhm = I2C_ReceiveByte();
	I2C_Ack();
	u8 zlm = I2C_ReceiveByte();
	I2C_Ack();
	u8 yhm = I2C_ReceiveByte();
	I2C_Ack();
	u8 ylm = I2C_ReceiveByte();
	I2C_NoAck();
	
	I2C_Stop();
	
	//Make Magnetometer 3 channels in vectors mag
	mag->x = (s16)(xhm << 8 | xlm);
	mag->y = (s16)(yhm << 8 | ylm);
	mag->z = (s16)(zhm << 8 | zlm);
}


s16 min(s16 a,s16 b)
{
	if(a < b) return a;
	else return b;
}
s16 max(s16 a,s16 b)
{
	if(a > b)return a;
	else return b;
}

void LSM303Enable(void)
{
	// Enable Accelerometer
  // 0x27 = 0b00100111
  // Normal power mode, all axes enabled
	writeAccReg(LSM303_CTRL_REG1_A,0X27);
	
	// Enable Magnetometer
  // 0x00 = 0b00000000
  // Continuous conversion mode
	writeMagReg(LSM303_MR_REG_M,0X00);
}
void LSM303Read(vector *acc,vector *mag)
{
	readAcc(acc);
	readMag(mag);
}

int LSM303Heading(vector acc, vector mag)
{
	//return Heading((vector){0,-1,0});
	return Heading((vector){1,0,0},mag,acc);
}

int Heading(vector from,vector mag,vector acc)
{
	vector magt;
	
	m_max.x = 516;
	m_max.y = 592;
	m_max.z = 441;
	m_min.x = -445;
	m_min.y = -378;
	m_min.z = -445;
	
 // shift and scale
	magt.x = (mag.x - m_min.x) / (m_max.x - m_min.x) * 2 - 1.0;
	magt.y = (mag.y - m_min.y) / (m_max.y - m_min.y) * 2 - 1.0;
	magt.z = (mag.z - m_min.z) / (m_max.z - m_min.z) * 2 - 1.0;

	vector temp_a ;
	temp_a.x = acc.x;
	temp_a.y = acc.y;
	temp_a.z = acc.z;
	// normalize
	vector_normalize(&temp_a);
	//vector_normalize(&m);

	// compute E and N
	vector E;
	vector N;
	vector_cross(&magt, &temp_a, &E);
	vector_normalize(&E);
	vector_cross(&temp_a, &E, &N);

	// compute heading
	int heading = round(atan2(vector_dot(&E, &from), vector_dot(&N, &from)) * 180 / PI);
	if (heading < 0) heading += 360;
  return heading;
}

void vector_cross(const vector *a,const vector *b, vector *out)
{
  out->x = a->y*b->z - a->z*b->y;
  out->y = a->z*b->x - a->x*b->z;
  out->z = a->x*b->y - a->y*b->x;
}

float vector_dot(const vector *a,const vector *b)
{
  return a->x*b->x+a->y*b->y+a->z*b->z;
}

void vector_normalize(vector *a)
{
  float mag = sqrt(vector_dot(a,a));
  a->x /= mag;
  a->y /= mag;
  a->z /= mag;
}






























