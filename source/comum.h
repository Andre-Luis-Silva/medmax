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

#define ADDR_FLASH		0xFC000
#define DADOS_SALVOS	24
#define CONT_DADOS		32
#define DADO_MEMORIA	4

#endif /* COMUM_H_ */
