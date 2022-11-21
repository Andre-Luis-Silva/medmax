/*
 * teclado.c
 *
 *  Created on: 5 de ago de 2022
 *      Author: andre
 */

#include "comum.h"

/* Structs para filas */
extern QueueHandle_t fila_teclado;

void teclado_run( void ){

	vTaskDelay(100);

    adc16_config_t adc16ConfigStruct2;
    adc16_channel_config_t adc16ChannelConfigStruct2;

    adc16ChannelConfigStruct2.enableInterruptOnConversionCompleted = false;
    adc16ChannelConfigStruct2.enableDifferentialConversion = false;
	unsigned int cont_matriz = 0, comando = 0, teste_motor[2] = {1,1}, numero = 0;
	unsigned int col1, col2, col3, col4;

	while(1){

		adc16ChannelConfigStruct2.channelNumber = 4;
        ADC16_SetChannelConfig(ADC1, 1, &adc16ChannelConfigStruct2);
        while (0U == (kADC16_ChannelConversionDoneFlag &
                      ADC16_GetChannelStatusFlags(ADC1, 1)))
        {
        }
        col1 = ADC16_GetChannelConversionValue(ADC1, 1);


		adc16ChannelConfigStruct2.channelNumber = 5;
        ADC16_SetChannelConfig(ADC1, 1, &adc16ChannelConfigStruct2);
        while (0U == (kADC16_ChannelConversionDoneFlag &
                      ADC16_GetChannelStatusFlags(ADC1, 1)))
        {
        }
        col2 = ADC16_GetChannelConversionValue(ADC1, 1);

		adc16ChannelConfigStruct2.channelNumber = 6;
        ADC16_SetChannelConfig(ADC1, 1, &adc16ChannelConfigStruct2);
        while (0U == (kADC16_ChannelConversionDoneFlag &
                      ADC16_GetChannelStatusFlags(ADC1, 1)))
        {
        }
        col3 = ADC16_GetChannelConversionValue(ADC1, 1);

		adc16ChannelConfigStruct2.channelNumber = 7;
        ADC16_SetChannelConfig(ADC1, 1, &adc16ChannelConfigStruct2);
        while (0U == (kADC16_ChannelConversionDoneFlag &
                      ADC16_GetChannelStatusFlags(ADC1, 1)))
        {
        }
        col4 = ADC16_GetChannelConversionValue(ADC1, 1);

		switch( cont_matriz ){

		case 0:

			linha1_0;
			linha2_1;
			linha3_1;
			linha4_1;
			if( col1 < 3500 ){
				comando = zero;
			}
			else if( col2 < 3500 ){
				comando = dot;
			}
			else if( col3 < 3500 ){
				comando = hifen;
			}
			else if( col4 < 3500 ){
				comando = yes;
			}


			break;

		case 1:

			linha1_1;
			linha2_0;
			linha3_1;
			linha4_1;
			if( col1 < 3500 ){
				comando = um;
			}
			else if( col2 < 3500 ){
				comando = dois;
			}
			else if( col3 < 3500 ){
				comando = tres;
			}
			else if( col4 < 3500 ){
				comando = quatro;
			}
			break;

		case 2:

			linha1_1;
			linha2_1;
			linha3_0;
			linha4_1;
			if( col1 < 3500 ){
				comando = cinco;
				teste_motor[0] = numero;
			}
			else if( col2 < 3500 ){
				comando = left;
			}
			else if( col3 < 3500 ){
				comando = right;
			}
			else if( col4 < 3500 ){
				comando = no;
			}
			break;

		case 3:

			linha1_1;
			linha2_1;
			linha3_1;
			linha4_0;
			if( col1 < 3500 ){
				comando = seis;
			}
			else if( col2 < 3500 ){
				comando = sete;
			}
			else if( col3 < 3500 ){
				comando = oito;
			}
			else if( col4 < 3500 ){
				comando = nove;
			}
			break;

		}

		if( cont_matriz >= 3)
			cont_matriz = 0;
		else
			cont_matriz++;

		if( comando != 0 ){
			xQueueSendToBack(fila_teclado,&comando, 0);
			comando = 0;
		}
		vTaskDelay(75);

	}

}
