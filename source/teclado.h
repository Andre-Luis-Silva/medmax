/*
 * teclado.h
 *
 *  Created on: 5 de ago de 2022
 *      Author: andre
 */

#ifndef TECLADO_H_
#define TECLADO_H_
#include "board.h"
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
#include "fsl_pdb.h"

#define zero	0
#define um		1
#define dois	2
#define tres 	3
#define quatro	4
#define cinco	5
#define seis	6
#define sete	7
#define oito	8
#define nove	9
#define left	10
#define right	11
#define no		12
#define dot		13
#define hifen	14
#define yes		15

#define linha1_0	GPIO_PinWrite( GPIOC, 4, 0 )
#define linha1_1	GPIO_PinWrite( GPIOC, 4, 1 )
#define linha2_0	GPIO_PinWrite( GPIOC, 5, 0 )
#define linha2_1	GPIO_PinWrite( GPIOC, 5, 1 )
#define linha3_0	GPIO_PinWrite( GPIOC, 6, 0 )
#define linha3_1	GPIO_PinWrite( GPIOC, 6, 1 )
#define linha4_0	GPIO_PinWrite( GPIOC, 7, 0 )
#define linha4_1	GPIO_PinWrite( GPIOC, 7, 1 )

#define read_col1	GPIO_PinRead(GPIOC, 8)
#define read_col2	GPIO_PinRead(GPIOC, 9)
#define read_col3	GPIO_PinRead(GPIOC, 10)
#define read_col4	GPIO_PinRead(GPIOC, 11)


void teclado_run( void );

#endif /* TECLADO_H_ */
