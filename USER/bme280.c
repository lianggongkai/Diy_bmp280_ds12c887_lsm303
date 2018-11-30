#include "i2c_soft.h"
#include "bme280_defs.h"
#include "bme280.h"

#define BME280_WriteAddress (BME280_I2C_ADDR_SEC<<1)
#define BME280_ReadAddress	(BME280_WriteAddress|1)

void BMP280_WR_Byte(u8 addr,u8 bytedata)
{
	I2C_Start();
	I2C_SendByte(BME280_WriteAddress);
	I2C_WaitAck();
	I2C_SendByte(addr);
	I2C_WaitAck();
	I2C_SendByte(bytedata);
	I2C_WaitAck();
	I2C_Stop();
}

u8 BMP280_RD_Byte(u8 addr)
{
	u8 Dat=0;

	I2C_Start();
	I2C_SendByte(BME280_WriteAddress);
	I2C_WaitAck();
	I2C_SendByte(addr);
	I2C_WaitAck();
	I2C_Start();
	I2C_SendByte(BME280_ReadAddress);
	I2C_WaitAck();
	Dat=I2C_ReceiveByte();
	I2C_Stop();
	return Dat;
}

void ReadCalibrate(struct bme280_calib_data *calibrate)
{
	u8 i = 0;
	u8 CalbrtPTBuff[BME280_TEMP_PRESS_CALIB_DATA_LEN] = {0};
	u8 CalbrtHBuff[BME280_HUMIDITY_CALIB_DATA_LEN] = {0};
	
	//Read Calibrate parameters of Pressure and Tempature
	I2C_Start();
	I2C_SendByte(BME280_WriteAddress);
	I2C_WaitAck();
	I2C_SendByte(BME280_TEMP_PRESS_CALIB_DATA_ADDR);
	I2C_WaitAck();
	I2C_Start();
	I2C_SendByte(BME280_ReadAddress);
	I2C_WaitAck();
	i = 0;
	while(i < BME280_TEMP_PRESS_CALIB_DATA_LEN){
		CalbrtPTBuff[i++] = I2C_ReceiveByte();
		if(i < BME280_TEMP_PRESS_CALIB_DATA_LEN)I2C_Ack();
		else I2C_NoAck();
	}
	I2C_Stop();
	
	//Read Calibrate parameters of Humidity
	I2C_Start();
	I2C_SendByte(BME280_WriteAddress);
	I2C_WaitAck();
	I2C_SendByte(BME280_HUMIDITY_CALIB_DATA_ADDR);
	I2C_WaitAck();
	I2C_Start();
	I2C_SendByte(BME280_ReadAddress);
	I2C_WaitAck();
	i = 0;
	while(i < BME280_HUMIDITY_CALIB_DATA_LEN){
		CalbrtHBuff[i++] = I2C_ReceiveByte();
		if(i < BME280_HUMIDITY_CALIB_DATA_LEN)I2C_Ack();
		else I2C_NoAck();
	}
	I2C_Stop();
	
	calibrate->dig_T1 = (u16)(CalbrtPTBuff[0] + (CalbrtPTBuff[1] << 8));
	calibrate->dig_T2 = (s16)(CalbrtPTBuff[2] + (CalbrtPTBuff[3] << 8));
	calibrate->dig_T3 = (s16)(CalbrtPTBuff[4] + (CalbrtPTBuff[5] << 8));
	
	calibrate->dig_P1 = (u16)(CalbrtPTBuff[6] + (CalbrtPTBuff[7] << 8));
	calibrate->dig_P2 = (s16)(CalbrtPTBuff[8] + (CalbrtPTBuff[9] << 8));
	calibrate->dig_P3 = (s16)(CalbrtPTBuff[10] + (CalbrtPTBuff[11] << 8));
	calibrate->dig_P4 = (s16)(CalbrtPTBuff[12] + (CalbrtPTBuff[13] << 8));
	calibrate->dig_P5 = (s16)(CalbrtPTBuff[14] + (CalbrtPTBuff[15] << 8));
	calibrate->dig_P6 = (s16)(CalbrtPTBuff[16] + (CalbrtPTBuff[17] << 8));
	calibrate->dig_P7 = (s16)(CalbrtPTBuff[18] + (CalbrtPTBuff[19] << 8));
	calibrate->dig_P8 = (s16)(CalbrtPTBuff[20] + (CalbrtPTBuff[21] << 8));
	calibrate->dig_P9 = (s16)(CalbrtPTBuff[22] + (CalbrtPTBuff[23] << 8));
	
	calibrate->dig_H1 = (u8)CalbrtPTBuff[25];
	calibrate->dig_H2 = (s16)CalbrtHBuff[0] + (CalbrtHBuff[1] << 8);
	calibrate->dig_H3 = (u8)CalbrtHBuff[2];
	calibrate->dig_H4 = (s16)(CalbrtHBuff[3]<<4) + (CalbrtHBuff[4] & 0x0f);
	calibrate->dig_H5 = (s16)(((CalbrtHBuff[4]&0xf0)>>4) + (CalbrtHBuff[5]<<4));
	calibrate->dig_H6 = (s8)CalbrtHBuff[6];
}

void BME280_SetOperationMode(void)
{
	u8 CtrlMeas = 0;
	//Normal mode,No filter,Oversampling setting 1
	
	//Ctrl humidity oversampling 1 times
	BMP280_WR_Byte(BME280_CTRL_HUM_ADDR,BME280_OVERSAMPLING_4X);
	
	//Pressure and temperature oversamplinng 1times , forced mode
	CtrlMeas = BME280_NORMAL_MODE<<BME280_SENSOR_MODE_POS;
	CtrlMeas |= BME280_OVERSAMPLING_4X << BME280_CTRL_PRESS_POS;
	CtrlMeas |= BME280_OVERSAMPLING_4X << BME280_CTRL_TEMP_POS;
	BMP280_WR_Byte(BME280_CTRL_MEAS_ADDR, CtrlMeas);
	
	//No filter , Standby 10ms
	CtrlMeas = 0;
	CtrlMeas |= BME280_FILTER_COEFF_16 << BME280_FILTER_POS;
	CtrlMeas |= BME280_STANDBY_TIME_62_5_MS << BME280_STANDBY_POS;
	BMP280_WR_Byte(BME280_CONFIG_ADDR,CtrlMeas);
}

void ReadMeasureResult(struct bme280_uncomp_data *pdata)
{
	//BME280_DATA_ADDR
	//BME280_P_T_H_DATA_LEN
	u8 pth_buff[BME280_P_T_H_DATA_LEN] = {0};
	u8 i = 0;
	I2C_Start();
	I2C_SendByte(BME280_WriteAddress);
	I2C_WaitAck();
	I2C_SendByte(BME280_DATA_ADDR);
	I2C_WaitAck();
	I2C_Start();
	I2C_SendByte(BME280_ReadAddress);
	I2C_WaitAck();
	while(i < BME280_P_T_H_DATA_LEN){
		pth_buff[i++] = I2C_ReceiveByte();
		if(i < BME280_P_T_H_DATA_LEN)I2C_Ack();
		else I2C_NoAck();
	}
	I2C_Stop();
	
	pdata->pressure = (uint32_t)((pth_buff[0]<<12) + (pth_buff[1]<<8) + (pth_buff[2]>>4));
	pdata->temperature= (uint32_t)((pth_buff[3]<<12) + (pth_buff[4]<<8) + (pth_buff[5]>>4));
	pdata->humidity = (uint32_t)((pth_buff[6]<<8) + pth_buff[7]);
}

/*
int32_t compensate_temperature(const struct bme280_uncomp_data *uncomp_data,
						struct bme280_calib_data *calib_data)
{
	int32_t var1;
	int32_t var2;
	int32_t temperature;
	int32_t temperature_min = -4000;
	int32_t temperature_max = 8500;

	var1 = (int32_t)((uncomp_data->temperature / 8) - ((int32_t)calib_data->dig_T1 * 2));
	var1 = (var1 * ((int32_t)calib_data->dig_T2)) / 2048;
	var2 = ((int32_t)((uncomp_data->temperature >> 4) - ((int32_t)calib_data->dig_T1))) * ((int32_t)((uncomp_data->temperature >> 4) - ((int32_t)calib_data->dig_T3)));
	var2 >>= 12;
	var2 = (((int32_t)calib_data->dig_T3) * var2)>>14;
	//var2 = (((var2 * var2) / 4096) * ((int32_t)calib_data->dig_T3)) / 16384;
	calib_data->t_fine = var1 + var2;
	temperature = (calib_data->t_fine * 5 + 128) >> 8;

	if (temperature < temperature_min)
		temperature = temperature_min;
	else if (temperature > temperature_max)
		temperature = temperature_max;

	return temperature;
}*/



double compensate_temperature_f32(const struct bme280_uncomp_data *uncomp_data,
						struct bme280_calib_data *calib_data) 
{  
    double v_x1_u32 = 0;  
    double v_x2_u32 = 0;  
    double temperature = 0;  
		s32 v_uncom_temperature_s32 = uncomp_data->temperature;
    v_x1_u32  = (((double)v_uncom_temperature_s32) / 16384.0 -  
    ((double)calib_data->dig_T1) / 1024.0) *  
    ((double)calib_data->dig_T2);  
    v_x2_u32  = ((((double)v_uncom_temperature_s32) / 131072.0 -  
    ((double)calib_data->dig_T1) / 8192.0) *  
    (((double)v_uncom_temperature_s32) / 131072.0 -  
    ((double)calib_data->dig_T1) / 8192.0)) *  
    ((double)calib_data->dig_T3);  
    calib_data->t_fine = (s32)(v_x1_u32 + v_x2_u32);  
    temperature  = (v_x1_u32 + v_x2_u32) / 5120.0;  
  
  
    return temperature;  
} 

uint32_t compensate_pressure(const struct bme280_uncomp_data *uncomp_data,
						const struct bme280_calib_data *calib_data)
{
	int32_t var1;
	int32_t var2;
	int32_t var3;
	int32_t var4;
	uint32_t var5;
	uint32_t pressure;
	uint32_t pressure_min = 30000;
	uint32_t pressure_max = 110000;

	var1 = (((int32_t)calib_data->t_fine) / 2) - (int32_t)64000;
	var2 = (((var1 / 4) * (var1 / 4)) / 2048) * ((int32_t)calib_data->dig_P6);
	var2 = var2 + ((var1 * ((int32_t)calib_data->dig_P5)) * 2);
	var2 = (var2 / 4) + (((int32_t)calib_data->dig_P4) * 65536);
	var3 = (calib_data->dig_P3 * (((var1 / 4) * (var1 / 4)) / 8192)) / 8;
	var4 = (((int32_t)calib_data->dig_P2) * var1) / 2;
	var1 = (var3 + var4) / 262144;
	var1 = (((32768 + var1)) * ((int32_t)calib_data->dig_P1)) / 32768;
	 /* avoid exception caused by division by zero */
	if (var1) {
		var5 = (uint32_t)((uint32_t)1048576) - uncomp_data->pressure;
		pressure = ((uint32_t)(var5 - (uint32_t)(var2 / 4096))) * 3125;
		if (pressure < 0x80000000)
			pressure = (pressure << 1) / ((uint32_t)var1);
		else
			pressure = (pressure / (uint32_t)var1) * 2;

		var1 = (((int32_t)calib_data->dig_P9) * ((int32_t)(((pressure / 8) * (pressure / 8)) / 8192))) / 4096;
		var2 = (((int32_t)(pressure / 4)) * ((int32_t)calib_data->dig_P8)) / 8192;
		pressure = (uint32_t)((int32_t)pressure + ((var1 + var2 + calib_data->dig_P7) / 16));

		if (pressure < pressure_min)
			pressure = pressure_min;
		else if (pressure > pressure_max)
			pressure = pressure_max;
	} else {
		pressure = pressure_min;
	}

	return pressure;
}

uint32_t compensate_humidity(const struct bme280_uncomp_data *uncomp_data,
						const struct bme280_calib_data *calib_data)
{
	int32_t var1;
	int32_t var2;
	int32_t var3;
	int32_t var4;
	int32_t var5;
	uint32_t humidity;
	uint32_t humidity_max = 102400;

	var1 = calib_data->t_fine - ((int32_t)76800);
	var2 = (int32_t)(uncomp_data->humidity * 16384);
	var3 = (int32_t)(((int32_t)calib_data->dig_H4) * 1048576);
	var4 = ((int32_t)calib_data->dig_H5) * var1;
	var5 = (((var2 - var3) - var4) + (int32_t)16384) / 32768;
	var2 = (var1 * ((int32_t)calib_data->dig_H6)) / 1024;
	var3 = (var1 * ((int32_t)calib_data->dig_H3)) / 2048;
	var4 = ((var2 * (var3 + (int32_t)32768)) / 1024) + (int32_t)2097152;
	var2 = ((var4 * ((int32_t)calib_data->dig_H2)) + 8192) / 16384;
	var3 = var5 * var2;
	var4 = ((var3 / 32768) * (var3 / 32768)) / 128;
	var5 = var3 - ((var4 * ((int32_t)calib_data->dig_H1)) / 16);
	var5 = (var5 < 0 ? 0 : var5);
	var5 = (var5 > 419430400 ? 419430400 : var5);
	humidity = (uint32_t)(var5 / 4096);

	if (humidity > humidity_max)
		humidity = humidity_max;

	return humidity;
}


















