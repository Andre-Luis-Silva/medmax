/*
 * display.h
 *
 *  Created on: 9 de ago de 2022
 *      Author: andre
 */

#ifndef DISPLAY_H_
#define DISPLAY_H_

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

#define cd_off		GPIO_PinWrite( GPIOD, 1, 0 )
#define cd_on		GPIO_PinWrite( GPIOD, 1, 1 )
#define rd_off		GPIO_PinWrite( GPIOE, 1, 0 )
#define rd_on		GPIO_PinWrite( GPIOE, 1, 1 )
#define wr_off		GPIO_PinWrite( GPIOD, 0, 0 )
#define wr_on		GPIO_PinWrite( GPIOD, 0, 1 )
#define db0_off		GPIO_PinWrite( GPIOE, 0, 0 )
#define db0_on		GPIO_PinWrite( GPIOE, 0, 1 )
#define db1_off		GPIO_PinWrite( GPIOC, 18, 0 )
#define db1_on		GPIO_PinWrite( GPIOC, 18, 1 )
#define db2_off		GPIO_PinWrite( GPIOD, 7, 0 )
#define db2_on		GPIO_PinWrite( GPIOD, 7, 1 )
#define db3_off		GPIO_PinWrite( GPIOC, 17, 0 )
#define db3_on		GPIO_PinWrite( GPIOC, 17, 1 )
#define db4_off		GPIO_PinWrite( GPIOD, 6, 0 )
#define db4_on		GPIO_PinWrite( GPIOD, 6, 1 )
#define db5_off		GPIO_PinWrite( GPIOC, 16, 0 )
#define db5_on		GPIO_PinWrite( GPIOC, 16, 1 )
#define db6_off		GPIO_PinWrite( GPIOD, 5, 0 )
#define db6_on		GPIO_PinWrite( GPIOD, 5, 1 )
#define db7_off		GPIO_PinWrite( GPIOC, 15, 0 )
#define db7_on		GPIO_PinWrite( GPIOC, 15, 1 )
#define ce_off		GPIO_PinWrite( GPIOD, 4, 0 )
#define ce_on		GPIO_PinWrite( GPIOD, 4, 1 )
#define rst_off		GPIO_PinWrite( GPIOC, 14, 0 )
#define rst_on		GPIO_PinWrite( GPIOC, 14, 1 )

#define db0_read	GPIO_PinRead(GPIOE, 0)
#define db1_read	GPIO_PinRead(GPIOC, 18)
#define db2_read	GPIO_PinRead(GPIOD, 7)
#define db3_read	GPIO_PinRead(GPIOC, 17)
#define db4_read	GPIO_PinRead(GPIOD, 6)
#define db5_read	GPIO_PinRead(GPIOC, 16)
#define db6_read	GPIO_PinRead(GPIOD, 5)
#define db7_read	GPIO_PinRead(GPIOC, 15)

#define sensorRead 	GPIO_PinRead(GPIOE, 2)

#define agulhaAberta	GPIO_PinRead(GPIOA, 17) == 1
#define agulhaFechada	GPIO_PinRead(GPIOA, 17) == 0

// Biblioteca do display
#define Set_cursor_pointer		0x21
#define Set_offset_register		0x22
#define Set_address_pointer		0x24
#define Set_text_home_addr		0x40
#define Set_text_area			0x41
#define Set_graphic_home_addr	0x42
#define Set_graphic_area		0x43
#define Mode_set_or				0x80
#define Mode_set_exor			0x81
#define Mode_set_and 			0x83
#define Mode_set_ext 			0x88
#define Display_mode_blink  	0x93
#define Display_mode_text   	0x94
#define Display_mode_graphic   	0x98
#define Set_data_auto_write   	0xB0
#define Auto_reset			   	0xB2

// Erro para os eletrodos
#define ErrorK					0
#define ErrorNa					1
#define ErrorCl					2
#define ErrorCa					3
#define ErrorpH					4

// Definição dos tipos de calibração
#define TYPEA					1
#define TYPEB					2

// Leitura de erro em anormal e não anormal
#define NOABNORMAL				0
#define ABNORMAL				1

// Se define como calibração ou número
#define CAL						0
#define NUM						1

// Retorno de funções
#define OK						0
#define ERRO_FALTA_CALIBA		1
#define ERRO					2
#define ERRO_FALTA_CALIBB		3

// Definição do tipo de exame
#define SORO					0
#define SANGUE					1
#define URINA					2

// Definição dos menus
#define PRINCIPAL				0
#define CONFIGURACAO			1
#define SERVICO					2

// Tempo de atraso para verificação de exame
#define TIMERCOM	250

void display_run( void );
void pixel( unsigned char pos_x, unsigned char pos_y, unsigned char on_off );
void send_command( unsigned char command );
void send_data( unsigned char data );
unsigned char read_data( void );
unsigned char status( unsigned char tipo );
void linha_horizontal8( unsigned char pos_y, unsigned char quant_dados8 );
void linha_horizontal( unsigned char pos_x, unsigned char pos_y, unsigned char quant_dados );
void linha_vertical( unsigned char pos_x, unsigned char pos_y, unsigned char quant_dados );
void desenha_fundo_menu( unsigned char posicao, unsigned char on_off );
void desenho_menu1( void );
void desenho_menu2( void );
void desenho_menu3( void );
void desenho_configuracao1( void );
void desenho_configuracao2( void );
void desenho_configuracao3( void );
void desenho_servicos1( void );
void desenho_servicos2( void );
void desenho_inicializacao( void );
void telaManutencao( void );
void escrita_texto( unsigned int posicao, char *texto, unsigned char tamanho );
void clear_display_text( void );
unsigned char calibA( unsigned char wash );
unsigned char calibB( void );
unsigned char TesteAmostras( unsigned char tipoTeste );
unsigned char clearLine( unsigned int line );
void WriteMenuName( unsigned char menu, unsigned char tipoMenu );
void calibValues( void );
char *numtolcd( unsigned int num, unsigned char typeCalOrNum );
unsigned int verifyError(  unsigned char typeAorB, unsigned char abnormal );
void stateMachineError( unsigned int position, unsigned char error, unsigned char typeError );
unsigned char verifyKeyBoard( void );
void initEquip( void );
void condicionamento( void );
void desproteinizacao( void );
void limpezaTubulacao( void );
unsigned char writeLine( unsigned int line );
void EscreveCedilhaAOTil( void );
char *ConverteNumParaLcd( unsigned char qtdDigitos, unsigned char qtdCasasDecimais, unsigned long num );
char ContaCaracteres( void );
void AjustaHora( void );
void AjustaCorrelacao( void );
void Cursor( unsigned char posicaoX, unsigned char posicaoY, unsigned char piscar, unsigned char tamanhoCursor );
float *EscreveTela( unsigned char posicaoX, unsigned char posicaoY, unsigned char contDigitos, unsigned char apenasNumeros  );
void EscreveDigito( unsigned int posicao, unsigned char tecla );
void SalvaFlashConfiguracao( unsigned int enderecoInicial, unsigned int enderecoFinal, int *dados );
unsigned char ContaDigitos( unsigned int num );
unsigned long long EscreveCodigoDeBarras( void );
#endif /* DISPLAY_H_ */
