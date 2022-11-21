/*
 * rtc.h
 *
 *  Created on: 20 de out de 2022
 *      Author: andre
 */

#ifndef RTC_H_
#define RTC_H_

#define Control_Status_1	0x00
#define Control_Status_2	0x01
#define VL_seconds			0x02
#define Minutes				0x03
#define Hours				0x04
#define Days				0x05
#define Weekdays			0x06
#define Century_months		0x07
#define Years				0x08

#define STOPrtc				0x20
#define STARTrtc			0x00

#define PCF8563_I2C_ADDR		(0x51)
#define PCF8563_I2C_BAUDRATE	100000

char bcdtodec( char val );
char dectobcd( char val );
void configRTC( void );
void initRTC( void );
void writeHourRTC( void );
unsigned long I2C_READ_PCF8653(uint8_t *pBuff, uint8_t reg);
unsigned long I2C_WRITE_PCF8653(uint8_t *pBuff, uint16_t buffLen);

#endif /* RTC_H_ */
