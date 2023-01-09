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
#define BUSY		GPIO_Read( GPIOB, 11 )
#define STB_ON		GPIO_PinWrite( GPIOC, 11, 0 )
#define STB_OFF		GPIO_PinWrite( GPIOC, 11, 1 )

void EscrevePrinter( char *texto, unsigned char tamanhoTexto, unsigned char tamanhoLetra, unsigned char printaOuPulaTexto );

#endif /* PRINTER_H_ */
