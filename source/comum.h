/*
 * comum.h
 *
 *  Created on: 30 de ago de 2022
 *      Author: andre
 */

#ifndef COMUM_H_
#define COMUM_H_

#include "board.h"
#include "math.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "MK64F12.h"
#include "fsl_debug_console.h"
#include "FreeRTOS.h"
#include "timers.h"
#include "queue.h"
#include "task.h"
#include <nfc_task.h>
#include "fsl_adc16.h"
#include "fsl_ftm.h"
#include "fsl_i2c.h"
#include "fsl_i2c_freertos.h"
#include "fsl_uart.h"
#include "motor.h"
#include "display.h"
#include "teclado.h"
#include "nfc_task.h"
#include "rtc.h"
#include "fsl_flash.h"
#include <Nfc.h>
#include <ndef_helper.h>
#include "printer.h"

#define ADDR_CALIBRACAO		0xE0000
#define ADDR_CONFIGURACAO	0xE2000
#define ADDR_EXAME			0xE3000
#define DADOS_CALIBRACAO	24
#define DADOS_EXAMES		32
#define DADO_MEMORIA		4
#define ADDR_SLOPE_K		0
#define ADDR_SLOPE_Na		4
#define ADDR_SLOPE_Cl		8
#define ADDR_SLOPE_Ca		12
#define ADDR_SLOPE_pH		16
#define ADDR_INTERCEPT_K	20
#define ADDR_INTERCEPT_Na	24
#define ADDR_INTERCEPT_Cl	28
#define ADDR_INTERCEPT_Ca	32
#define ADDR_INTERCEPT_pH	36
#endif /* COMUM_H_ */
