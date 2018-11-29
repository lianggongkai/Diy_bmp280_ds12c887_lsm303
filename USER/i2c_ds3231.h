#ifndef _I2C_DS3231_H_
#define _I2C_DS3231_H_

#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"

void IIC_Init(void);
u8 ReadDS3231(u8 ADDR,u8 *val);

#endif
