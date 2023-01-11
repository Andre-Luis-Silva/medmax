#include "comum.h"

void EscrevePrinter( char *texto, unsigned char tamanhoTexto, unsigned char tamanhoLetra ){

	EnviaComando( COMANDO_INICIO_PRINTER >> 8 );	// Inicia printer MSB
	EnviaComando( COMANDO_INICIO_PRINTER & 0xFF );	// Inicia printer LSB

	EnviaComando( COMANDO_TAMANHO_LETRA >> 8 );	// Envia o comando do tamanho da letra MSB
	EnviaComando( COMANDO_TAMANHO_LETRA & 0xFF );	// Envia o comando do tamanho da letra LSB

	if( tamanhoLetra == LETRA_GRANDE ){	// Se tamanho letra igual a LETRA_GRANDE
		EnviaComando( LETRA_GRANDE );	// Envia o comando para letra grande
	}
	else if( tamanhoLetra == LETRA_PEQUENA ){	// Senão se tamanho da letra igual a LETRA_PEQUENA7
		EnviaComando( LETRA_PEQUENA );	// Envia o comando para letra grande
	}

	while( tamanhoTexto > 0 ){	// Enquanto tamanhoTexto é maior que zero
		EnviaComando( *(texto++) );	// Envia o texto a ser escrito

		tamanhoTexto--; // Decrementa o tamanho do texto.
	}

	EnviaComando( PRINT_TEXTO );	// Envia o comando PRINT_TEXTO
}

void EnviaComando( unsigned char data ){

	while(GPIO_PinRead( GPIOB, 11 ) == 1);// Enquanto a printer está em BUSY

	/* Acionamento a partir de cada bit */
	if( data & 0x01 )
		db0_on;
	else
		db0_off;

	if( data & 0x02 )
		db1_on;
	else
		db1_off;

	if( data & 0x04 )
		db2_on;
	else
		db2_off;

	if( data & 0x08 )
		db3_on;
	else
		db3_off;

	if( data & 0x10 )
		db4_on;
	else
		db4_off;

	if( data & 0x20 )
		db5_on;
	else
		db5_off;

	if( data & 0x40 )
		db6_on;
	else
		db6_off;

	if( data & 0x80 )
		db7_on;
	else
		db7_off;

	STB_ON;	// Liga o pino STB
	for( int i = 0; i < 65535 * 2; i++ );	// Faz um delay com for

	STB_OFF;	// Desliga o pino STB
	for( int i = 0; i < 65535 * 2; i++ );	// Faz um delay com for

}
