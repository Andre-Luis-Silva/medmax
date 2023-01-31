/*
 * Copyright 2016-2022 NXP
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of NXP Semiconductor, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file    projeto_Medmax.c
 * @brief   Application entry point.
 */
#include <motor.h>
#include <stdio.h>
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
#include "teclado.h"
#include "display.h"
#include "comum.h"

/* Structs para filas */
QueueHandle_t fila_teclado = NULL;
QueueHandle_t fila_display = NULL;

TaskHandle_t xHandle;

/* TODO: insert other include files here. */

/* TODO: insert other definitions and declarations here. */

/*
 * @brief   Application entry point.
 */

#define TASK_NFC_STACK_SIZE		1024
#define TASK_NFC_STACK_PRIO		(configMAX_PRIORITIES - 3)

#define MUX_NFC_STACK_SIZE		1024
#define MUX_NFC_STACK_PRIO		(configMAX_PRIORITIES - 1)

#define TECLADO_NFC_STACK_SIZE	1024
#define TECLADO_NFC_STACK_PRIO	(configMAX_PRIORITIES - 3)

#define DISPLAY_NFC_STACK_SIZE	1024
#define DISPLAY_NFC_STACK_PRIO	(configMAX_PRIORITIES - 3)


/* Variáveis de 8 bits */
extern unsigned char cont_mux;
extern unsigned char cont_motor;


int main(void) {

	/* Init board hardware. */
	BOARD_InitBootPins();

	SIM->SCGC6 |= SIM_SCGC6_PIT(1) | SIM_SCGC6_PDB(1);;
	PIT->MCR = 0x00;
	PIT->CHANNEL[0].TCTRL |= PIT_TCTRL_TEN(1);
	PIT->CHANNEL[0].LDVAL = 0xF00;
	SIM->SOPT7 |= 0x9400;

	ftm_config_t ftmInfo;
	FTM_GetDefaultConfig(&ftmInfo);

	// Initialize FTM module
	FTM_Init(FTM0, &ftmInfo);
	// Divide FTM clock by 4
	ftmInfo.prescale = kFTM_Prescale_Divide_4;
	FTM_SetTimerPeriod(FTM0, USEC_TO_COUNT(4000U, FTM_SOURCE_CLOCK));
	FTM_EnableInterrupts(FTM0, kFTM_TimeOverflowInterruptEnable);
	EnableIRQ(FTM0_IRQn);
	FTM_StartTimer(FTM0, kFTM_SystemClock);

	adc16_config_t adc16ConfigStruct;
	adc16_channel_config_t adc16ChannelConfigStruct;
	ADC16_GetDefaultConfig(&adc16ConfigStruct);
	//adc16ConfigStruct.hardwareAverageMode = kADC16_HardwareAverageCount8;
	ADC16_Init(DEMO_ADC16_BASE, &adc16ConfigStruct);
	ADC16_EnableHardwareTrigger(DEMO_ADC16_BASE, 0); /* Make sure the software trigger is used. */
	//ADC16_SetHardwareAverage(DEMO_ADC16_BASE, kADC16_HardwareAverageCount8);
	//ADC16_DoAutoCalibration(ADC0);

	pdb_config_t pdbConfigStruct;
	pdb_adc_pretrigger_config_t pdbAdcPreTriggerConfigStruct;

	PDB_GetDefaultConfig(&pdbConfigStruct);
	pdbConfigStruct.triggerInputSource = kPDB_TriggerSoftware;
	pdbConfigStruct.enableContinuousMode = true;

	PDB_Init(PDB0, &pdbConfigStruct);
	PDB_SetModulusValue(PDB0, 1000U);
	PDB_SetCounterDelayValue(PDB0, 1000U);

	pdbAdcPreTriggerConfigStruct.enablePreTriggerMask          = 1U << 4;
	pdbAdcPreTriggerConfigStruct.enableOutputMask              = 1U << 4;
	pdbAdcPreTriggerConfigStruct.enableBackToBackOperationMask = 0U;
	PDB_SetADCPreTriggerConfig(PDB0, 1, &pdbAdcPreTriggerConfigStruct);
	PDB_SetADCPreTriggerDelayValue(PDB0, 1, 1, 100U);
	PDB_DoLoadValues(PDB0);

	adc16_config_t adc16ConfigStruct2;
	adc16_channel_config_t adc16ChannelConfigStruct2;

	ADC16_GetDefaultConfig(&adc16ConfigStruct2);
	//adc16ConfigStruct2.hardwareAverageMode = kADC16_HardwareAverageCount8;
	ADC16_Init(ADC1, &adc16ConfigStruct2);
	ADC16_SetChannelMuxMode(ADC1, 1);
	ADC16_EnableHardwareTrigger(ADC1, 1); /* Make sure the software trigger is used. */
	//ADC16_SetHardwareAverage(ADC1, kADC16_HardwareAverageCount8);
	//ADC16_DoAutoCalibration(ADC1);
	uint32_t medida_ad = 0;

    adc16ChannelConfigStruct.enableInterruptOnConversionCompleted = false;
    adc16ChannelConfigStruct.enableDifferentialConversion = false;
    adc16ChannelConfigStruct.channelNumber                        = 15;
	while( medida_ad < 4090 ){

		ADC16_SetChannelConfig(DEMO_ADC16_BASE, DEMO_ADC16_CHANNEL_GROUP, &adc16ChannelConfigStruct);
		while (0U == (kADC16_ChannelConversionDoneFlag &
				ADC16_GetChannelStatusFlags(DEMO_ADC16_BASE, DEMO_ADC16_CHANNEL_GROUP)))
		{
		}
		medida_ad = ADC16_GetChannelConversionValue(DEMO_ADC16_BASE, DEMO_ADC16_CHANNEL_GROUP);
		move_mux(0, 1);

	}
	ma1_off;
	ma2_off;
	ma3_off;
	ma4_off;
	mb1_off;
	mb2_off;
	mb3_off;
	mb4_off;

	//BOARD_InitBootClocks();
	//BOARD_InitBootPeripherals();
#ifndef BOARD_INIT_DEBUG_CONSOLE_PERIPHERAL
	/* Init FSL debug console. */
	//BOARD_InitDebugConsole();
#endif

	fila_teclado = xQueueCreate( 1, sizeof( unsigned int ) );
	vQueueAddToRegistry(fila_teclado, "teclado");

	fila_display = xQueueCreate( 1, sizeof( unsigned char ) );
	vQueueAddToRegistry(fila_display, "display");

	/* Create NFC task */
	if (xTaskCreate((TaskFunction_t) task_nfc,
			(const char*) "NFC_task",
			TASK_NFC_STACK_SIZE,
			NULL,
			TASK_NFC_STACK_PRIO,
			NULL) != pdPASS)
	{
		printf("Failed to create NFC task");
	}

	/* Create teclado task */
	if (xTaskCreate((TaskFunction_t) teclado_run,
			(const char*) "Teclado_task",
			TECLADO_NFC_STACK_SIZE,
			NULL,
			TECLADO_NFC_STACK_PRIO,
			NULL) != pdPASS)
	{
		printf("Failed to create NFC task");
	}

	/* Create display task */
	if (xTaskCreate((TaskFunction_t) display_run,
			(const char*) "Teclado_task",
			DISPLAY_NFC_STACK_SIZE,
			NULL,
			DISPLAY_NFC_STACK_PRIO,
			NULL) != pdPASS)
	{
		printf("Failed to create NFC task");
	}

	vTaskStartScheduler();
	while(1);

	return 0;
}
