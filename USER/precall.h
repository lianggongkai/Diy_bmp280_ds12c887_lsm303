#ifndef _PRECALL_H_
#define _PRECALL_H_

#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "lcd_driver.h"
#include "gui.h"
#include "math.h"
#include "stdlib.h"
#include "i2c_ds3231.h"
#include "ds3231.h"
#include "string.h"
#include "bme280.h"
#include "bme280_defs.h"
#include "lsm303.h"

static void CopyBlank(char *des,u8 strlen);
static void ScreenSwitch(void);
static void Read_All_I2C_Device(void);

void RunLedInit(void);
void vtaskDrawLCD(void *p);
void vtaskReadI2CDevice(void *p);
void vtaskSwingLed(void *p);
void vtaskSetCalender(void *p);

#endif

