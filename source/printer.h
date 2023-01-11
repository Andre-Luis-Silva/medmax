/*
 * printer.h
 *
 *  Created on: 9 de jan de 2023
 *      Author: andre
 */

#ifndef PRINTER_H_
#define PRINTER_H_


#include "comum.h"

/* Biblioteca da printer */
#define BUSY		GPIO_PinRead( GPIOB, 11 )
#define STB_ON		GPIO_PinWrite( GPIOC, 0, 0 )
#define STB_OFF		GPIO_PinWrite( GPIOC, 0, 1 )

/* Funções do teclado */
#define COMANDO_INICIO_PRINTER	0x1B40
#define COMANDO_TAMANHO_LETRA	0x1B57
#define LETRA_GRANDE			0x03
#define	LETRA_PEQUENA			0x02
#define PRINT_TEXTO				0x0A
void EscrevePrinter( char *texto, unsigned char tamanhoTexto, unsigned char tamanhoLetra );
void EnviaComando( unsigned char data );
#endif /* PRINTER_H_ */
