/*
 * rtc.c
 *
 *  Created on: 20 de out de 2022
 *      Author: andre
 */


#include "comum.h"

extern i2c_master_transfer_t masterXfer;

// Essas funções retornam o valor em decimal do código do RTC. Exemplo: 0x10 em hexa para BCD é 10
char bcdtodec( char val ){

    return ( (val/16*10) + (val%16) );

}

// Essa função converte o valor decimal em BCD
char dectobcd(char val){

    return ( (val/10*16) + (val%10) );

}

unsigned long I2C_WRITE_PCF8653(uint8_t *pBuff, uint16_t buffLen)
{
	masterXfer.slaveAddress = PCF8563_I2C_ADDR;
    masterXfer.direction = kI2C_Write;
    masterXfer.data = pBuff;
    masterXfer.dataSize = buffLen;

    return I2C_MasterTransferBlocking(I2C0, &masterXfer);
}

unsigned long  I2C_READ_PCF8653(uint8_t *pBuff, uint8_t reg)
{
	masterXfer.slaveAddress = PCF8563_I2C_ADDR;
    masterXfer.direction = kI2C_Write;
    masterXfer.data = &reg;
    masterXfer.dataSize = 1;
    I2C_MasterTransferBlocking(I2C0, &masterXfer);

    masterXfer.direction = kI2C_Read;
    masterXfer.data = pBuff;
    masterXfer.dataSize = 1;
    return I2C_MasterTransferBlocking(I2C0, &masterXfer);
}

void configRTC( void ){

	/* Pseudocódigo para configuração default
	 * Para o RTC (STOP)
	 * Escreve no registo de segundos o valor 0
	 * Escreve no registro de minutos o valor 0
	 * Escreve no registro de horas o valor 12 em BCD
	 * Escreve no registro de dias o valor 10 em BCD
	 * Escreve no registro de dias da semana o valor do dia atual
	 * Escreve no registro de mês o valor 10 em BCD
	 * Escreve no registro de ano o valor 22 em BCD
	 * Inicia o RTC (START)
	*/

	uint8_t g_master_txBuff[] = {Control_Status_1, STOPrtc};
	I2C_WRITE_PCF8653(g_master_txBuff, 2);

	g_master_txBuff[0] = Control_Status_2;
	g_master_txBuff[1] = 0x00;
	I2C_WRITE_PCF8653(g_master_txBuff, 2);

	g_master_txBuff[0] = VL_seconds;
	g_master_txBuff[1] = 0;
	I2C_WRITE_PCF8653(g_master_txBuff, 2);

	g_master_txBuff[0] = Minutes;
	g_master_txBuff[1] = 0;
	I2C_WRITE_PCF8653(g_master_txBuff, 2);

	g_master_txBuff[0] = Hours;
	g_master_txBuff[1] = dectobcd(12);
	I2C_WRITE_PCF8653(g_master_txBuff, 2);

	g_master_txBuff[0] = Days;
	g_master_txBuff[1] = dectobcd(21);
	I2C_WRITE_PCF8653(g_master_txBuff, 2);

	g_master_txBuff[0] = Weekdays;
	g_master_txBuff[1] = 5;
	I2C_WRITE_PCF8653(g_master_txBuff, 2);

	g_master_txBuff[0] = Century_months;
	g_master_txBuff[1] = dectobcd(10);
	I2C_WRITE_PCF8653(g_master_txBuff, 2);

	g_master_txBuff[0] = Years;
	g_master_txBuff[1] = dectobcd(22);
	I2C_WRITE_PCF8653(g_master_txBuff, 2);

	g_master_txBuff[0] = Control_Status_1;
	g_master_txBuff[1] = STARTrtc;
	I2C_WRITE_PCF8653(g_master_txBuff, 2);

	uint8_t g_master_rxBuff[1];
	I2C_READ_PCF8653(&g_master_rxBuff[0], Control_Status_1);
	I2C_READ_PCF8653(&g_master_rxBuff[0], Control_Status_2);
	I2C_READ_PCF8653(&g_master_rxBuff[0], VL_seconds);
	I2C_READ_PCF8653(&g_master_rxBuff[0], Minutes);
	I2C_READ_PCF8653(&g_master_rxBuff[0], Hours);
	I2C_READ_PCF8653(&g_master_rxBuff[0], Days);
	I2C_READ_PCF8653(&g_master_rxBuff[0], Weekdays);
	I2C_READ_PCF8653(&g_master_rxBuff[0], Century_months);
	I2C_READ_PCF8653(&g_master_rxBuff[0], Years);

}

void writeHourRTC( void ){



}
