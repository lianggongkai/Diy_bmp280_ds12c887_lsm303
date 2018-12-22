#include "i2c_soft.h"
#include "bme280_defs.h"
#include "bme280.h"

//移植BMP280的读写函数
#define BMP280_WR_Byte(addr,bytedata) 	MCU_I2C_Write_Byte(BME280_I2C_ADDR_SEC,addr,bytedata)
#define BMP280_RD_Byte(addr,pdata)			MCU_I2C_Read_Byte(BME280_I2C_ADDR_SEC,addr,pdata)
#define BMP280_RD_Bytes(addr,len,buff)	MCU_I2C_Read_Bytes(BME280_I2C_ADDR_SEC,addr,len,buff)

//设置BMP280的工作模式
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

//读取BMP280的校准参数
void ReadCalibrate(struct bme280_calib_data *calibrate)
{
	u8 CalbrtPTBuff[BME280_TEMP_PRESS_CALIB_DATA_LEN] = {0};
	u8 CalbrtHBuff[BME280_HUMIDITY_CALIB_DATA_LEN] = {0};
	
	BMP280_RD_Bytes(BME280_TEMP_PRESS_CALIB_DATA_ADDR,BME280_TEMP_PRESS_CALIB_DATA_LEN,CalbrtPTBuff);
	BMP280_RD_Bytes(BME280_HUMIDITY_CALIB_DATA_ADDR,BME280_HUMIDITY_CALIB_DATA_LEN,CalbrtHBuff);
	
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

//读取温湿度压力的原始数据
void ReadMeasureResult(struct bme280_uncomp_data *pdata)
{
	//BME280_DATA_ADDR
	//BME280_P_T_H_DATA_LEN
	u8 pth_buff[BME280_P_T_H_DATA_LEN] = {0};
	
	BMP280_RD_Bytes(BME280_DATA_ADDR,BME280_P_T_H_DATA_LEN,pth_buff);
	
	pdata->pressure = (uint32_t)((pth_buff[0]<<12) + (pth_buff[1]<<4) + (pth_buff[2]>>4));
	pdata->temperature= (uint32_t)((pth_buff[3]<<12) + (pth_buff[4]<<4) + (pth_buff[5]>>4));
	pdata->humidity = (uint32_t)((pth_buff[6]<<8) + pth_buff[7]);
}

//根据读取到的原始数据和校准参数，得出补偿后的温度值
double compensate_temperature_f32(const struct bme280_uncomp_data *uncomp_data,
						struct bme280_calib_data *calib_data) 
{
	double var1;
	double var2;
	double temperature;
	double temperature_min = -40;
	double temperature_max = 85;

	var1 = ((double)uncomp_data->temperature) / 16384.0 - ((double)calib_data->dig_T1) / 1024.0;
	var1 = var1 * ((double)calib_data->dig_T2);
	var2 = (((double)uncomp_data->temperature) / 131072.0 - ((double)calib_data->dig_T1) / 8192.0);
	var2 = (var2 * var2) * ((double)calib_data->dig_T3);
	calib_data->t_fine = (int32_t)(var1 + var2);
	temperature = (var1 + var2) / 5120.0;

	if (temperature < temperature_min)
		temperature = temperature_min;
	else if (temperature > temperature_max)
		temperature = temperature_max;

	return temperature;
} 

//根据读取到的原始数据和校准参数，得出补偿后的气压值
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

//根据读取到的原始数据和校准参数，得出补偿后的湿度值
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


















