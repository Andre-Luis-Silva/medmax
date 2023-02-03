#include "comum.h"

#define TIMERBUZ	100

/* Variáveis de 8 bits */
volatile unsigned char flag_timer = 0, timerExam = 0, flagBuz = 0, flagSensorAtivo = 0;

/* Variáveis de 16 bits */
volatile unsigned int timerRTC = 0, timerBuz = 0, timerI2c = 0;

void move_mux( unsigned char posicao, unsigned char velocidade ){

	static unsigned char posicao_anterior, temporizador = 3;
	static unsigned char tbob1 = 0,tbob2 = 0,tbob3 = 0,tbob4 = 0;
	unsigned char flag_inicio = posicao, sentido;
	unsigned int passos = 0;
	unsigned char cont_passos = 0;
	uint32_t medida_ad = 0;
	adc16_config_t adc16ConfigStruct;
	adc16_channel_config_t adc16ChannelConfigStruct;
	if( posicao > 5 )
		posicao = 5;
	if( flag_inicio ){

		if( posicao == posicao_anterior )
			return;
		else if( posicao < posicao_anterior ){
			sentido = 0;
			passos = 60 * 1.11 * ( posicao_anterior - posicao );
		}
		else{
			sentido = 1;
			passos = 60 * 1.11 * ( posicao - posicao_anterior );
		}
		posicao_anterior = posicao;

		while( passos > 0 ){
			if( posicao == 3 )	// Se a posição for 3, ele vai até fazer a leitura do AD
			{
			    adc16ChannelConfigStruct.enableInterruptOnConversionCompleted = false;
			    adc16ChannelConfigStruct.enableDifferentialConversion = false;
			    adc16ChannelConfigStruct.channelNumber                        = 15;
				if( medida_ad < 4090 ){

					ADC16_SetChannelConfig(DEMO_ADC16_BASE, DEMO_ADC16_CHANNEL_GROUP, &adc16ChannelConfigStruct);
					while (0U == (kADC16_ChannelConversionDoneFlag &
							ADC16_GetChannelStatusFlags(DEMO_ADC16_BASE, DEMO_ADC16_CHANNEL_GROUP)))
					{
					}
					medida_ad = ADC16_GetChannelConversionValue(DEMO_ADC16_BASE, DEMO_ADC16_CHANNEL_GROUP);

				}
				else
				{
					ma1_off;
					ma2_off;
					ma3_off;
					ma4_off;
					return;
				}
			}
			if( flag_timer ){

				flag_timer = 0;
				temporizador--;
				if( temporizador == 0 ){
					cont_passos++;
					if( cont_passos > 1 ){
						cont_passos = 0;
						passos--;
					}
					tbob1++;
					if( tbob1 > 7 )
						tbob1 = 0;
					tbob2++;
					if( tbob2 > 9 )
						tbob2 = 2;
					tbob3++;
					if( tbob3 > 11 )
						tbob3 = 4;
					tbob4++;
					if( tbob4 > 13 )
						tbob4 = 6;

					if( velocidade == 1)
						temporizador = 3;
					else
						temporizador = 20;

				}

			}
			if( sentido == 1 ){

				if( tbob1 >= 0 && tbob1 < 5 )
					ma1_on;
				else
					ma1_off;
				if( tbob2 >= 2 && tbob2 < 7 )
					ma2_on;
				else
					ma2_off;
				if( tbob3 >= 4 && tbob3 < 9 )
					ma4_on;
				else
					ma4_off;
				if( tbob4 >= 6 && tbob4 < 11 )
					ma3_on;
				else
					ma3_off;

			}
			else{

				if( tbob1 >= 0 && tbob1 < 5 )
					ma3_on;
				else
					ma3_off;
				if( tbob2 >= 2 && tbob2 < 7 )
					ma4_on;
				else
					ma4_off;
				if( tbob3 >= 4 && tbob3 < 9 )
					ma2_on;
				else
					ma2_off;
				if( tbob4 >= 6 && tbob4 < 11 )
					ma1_on;
				else
					ma1_off;

			}
		}
		ma1_off;
		ma2_off;
		ma3_off;
		ma4_off;
	}
	else{

		if( flag_timer ){

			flag_timer = 0;
			temporizador--;
			if( !temporizador ){

				tbob1++;
				if( tbob1 > 7 )
					tbob1 = 0;
				tbob2++;
				if( tbob2 > 9 )
					tbob2 = 2;
				tbob3++;
				if( tbob3 > 11 )
					tbob3 = 4;
				tbob4++;
				if( tbob4 > 13 )
					tbob4 = 6;

				if( velocidade == 1)
					temporizador = 3;
				else
					temporizador = 20;

			}

		}
		if( tbob1 >= 0 && tbob1 < 5 )
			ma1_on;
		else{
			ma1_off;
		}
		if( tbob2 >= 2 && tbob2 < 7 )
			ma2_on;
		else
			ma2_off;
		if( tbob3 >= 4 && tbob3 < 9 )
			ma4_on;
		else
			ma4_off;
		if( tbob4 >= 6 && tbob4 < 11 )
			ma3_on;
		else
			ma3_off;

		posicao_anterior = 3;

	}

}

unsigned char move_tripa( unsigned char sentido, unsigned char velocidade, unsigned int tempo_on ){

	static unsigned char temporizador = 3;
	static unsigned char tbob1 = 0,tbob2 = 0,tbob3 = 0,tbob4 = 0;
	uint32_t medida_ad = 0;
	adc16_channel_config_t adc16ChannelConfigStruct;
	unsigned char flagLimpeza = 0;
	if( tempo_on > 15000 )
		flagLimpeza = 1;
	adc16ChannelConfigStruct.enableInterruptOnConversionCompleted = false;
	adc16ChannelConfigStruct.enableDifferentialConversion = false;
	adc16ChannelConfigStruct.channelNumber                        = 4;
	ADC16_SetChannelMuxMode(ADC0, 1);
	while( tempo_on > 0 ){
		if( flag_timer ){

			flag_timer = 0;
			tempo_on--;
			temporizador--;
			if( !temporizador ){
				tbob1++;
				if( tbob1 > 7 )
					tbob1 = 0;
				tbob2++;
				if( tbob2 > 9 )
					tbob2 = 2;
				tbob3++;
				if( tbob3 > 11 )
					tbob3 = 4;
				tbob4++;
				if( tbob4 > 13 )
					tbob4 = 6;

				if( velocidade == 1)
					temporizador = 3;
				else if( velocidade == 2 )
					temporizador = 6;
				else if( velocidade == 3 )
					temporizador = 2;
				else
					temporizador = 20;

			}

		}
		if( sentido == 1 ){
			if( tbob1 >= 0 && tbob1 < 5 )
				mb1_on;
			else{
				mb1_off;
			}
			if( tbob2 >= 2 && tbob2 < 7 )
				mb2_on;
			else
				mb2_off;
			if( tbob3 >= 4 && tbob3 < 9 )
				mb4_on;
			else
				mb4_off;
			if( tbob4 >= 6 && tbob4 < 11 )
				mb3_on;
			else
				mb3_off;
		}
		else{
			if( tbob1 >= 0 && tbob1 < 5 )
				mb3_on;
			else{
				mb3_off;
			}
			if( tbob2 >= 2 && tbob2 < 7 )
				mb4_on;
			else
				mb4_off;
			if( tbob3 >= 4 && tbob3 < 9 )
				mb2_on;
			else
				mb2_off;
			if( tbob4 >= 6 && tbob4 < 11 )
				mb1_on;
			else
				mb1_off;

		}
		if( velocidade == 4 /*&& tempo_on < 8360*/ ){

			ADC16_SetChannelConfig(DEMO_ADC16_BASE, DEMO_ADC16_CHANNEL_GROUP, &adc16ChannelConfigStruct);
			while (0U == (kADC16_ChannelConversionDoneFlag &
					ADC16_GetChannelStatusFlags(DEMO_ADC16_BASE, DEMO_ADC16_CHANNEL_GROUP)))
			{
			}
			if( ( medida_ad = ADC16_GetChannelConversionValue(DEMO_ADC16_BASE, DEMO_ADC16_CHANNEL_GROUP ) )  > 1350 && !flagLimpeza ){

				if( timerExam > TIME_MEAS ){
					mb1_off;
					mb2_off;
					mb3_off;
					mb4_off;
					flagBuz = 1;
					return 1;
				}

			}
			else
				timerExam = 0;
		}

	}
	mb1_off;
	mb2_off;
	mb3_off;
	mb4_off;
	return 0;
}


void FTM0_IRQHandler(void)
{
	//Clear interrupt flag.
	FTM_ClearStatusFlags(FTM0, kFTM_TimeOverflowFlag);
	flag_timer = 1;

	if( timerI2c < 65535 )	// Conta timerI2c
	{
		timerI2c++;
	}

	if( timerRTC < 65535 )
		timerRTC++;

	if( timerExam < 255 )
		timerExam++;

	if( sensorRead || flagBuz )	// Se sensor aberto ou flag para apitar
	{

		if( sensorRead )	//Se sensorRead ativo
		{
			if( flagSensorAtivo != 1 )
			{
				clearLine(14);
				clearLine(15);// Apaga as duas linhas 14 e 15 do display
			}
			flagSensorAtivo = 1;
			escrita_texto(420, "Feche a camara", sizeof("Feche a camara"));	// Escreve "Feche a câmara"
		}

		if( timerBuz < TIMERBUZ )		// Se tempo de acionamento do buzzer for menor que TEMPOBUZ
			GPIO_PinWrite( GPIOA, 16, 0 );	// Liga buzzer
		else if( timerBuz < TIMERBUZ * 2 )	// Senão se tempo de acionamento do buzzer for menor que TEMPOBUZ * 2
			GPIO_PinWrite( GPIOA, 16, 0 );	// Desliga Buzzer
		else{	// Senão
			timerBuz = 0;	// Zera tempo de acionamento do buzzer
			flagBuz = 0;	// Zera flag para apitar
		}
		if( timerBuz < 65535 )	// Se tempo de acionamento do buzzer for menor 65535
			timerBuz++;	// Incrementa tempo de acionamento do buzzer
	}
	else
	{
		GPIO_PinWrite(GPIOA, 16, 0);	// Desliga Buzzer
		if( flagSensorAtivo == 1 )
		{
			flagSensorAtivo = 0;
			clearLine(14);// Apaga as duas linhas 14 e 15 do display
		}
	}

	__DSB();

}



