#ifndef _BMP280_H_
#define _BMP280_H_
#include "stm32f10x.h"
#include "bme280_defs.h"

void BMP280_WR_Byte(u8 addr,u8 bytedata);
u8 BMP280_RD_Byte(u8 addr);
void ReadCalibrate(struct bme280_calib_data *calibrate);

void BME280_SetOperationMode(void);
void ReadMeasureResult(struct bme280_uncomp_data *pdata);

int32_t compensate_temperature(const struct bme280_uncomp_data *uncomp_data,
						struct bme280_calib_data *calib_data);
uint32_t compensate_pressure(const struct bme280_uncomp_data *uncomp_data,
						const struct bme280_calib_data *calib_data);
uint32_t compensate_humidity(const struct bme280_uncomp_data *uncomp_data,
						const struct bme280_calib_data *calib_data);

double compensate_temperature_f32(const struct bme280_uncomp_data *uncomp_data,
						struct bme280_calib_data *calib_data) ;
							
#endif
