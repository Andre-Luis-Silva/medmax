/*
 * rtc.c
 *
 *  Created on: 20 de out de 2022
 *      Author: andre
 */


#include "comum.h"

extern i2c_master_transfer_t masterXfer;

// Essas funções retornam o valor em decimal do código do RTC. Exemplo: 0x10 em hexa para BCD é 10
char bcdtodec( char val ){

    return ( (val/16*10) + (val%16) );

}

// Essa função converte o valor decimal em BCD
char dectobcd(char val){

    return ( (val/10*16) + (val%10) );

}

unsigned long I2C_WRITE_PCF8653(uint8_t *pBuff, uint16_t buffLen)
{
	masterXfer.slaveAddress = PCF8563_I2C_ADDR;
    masterXfer.direction = kI2C_Write;
    masterXfer.data = pBuff;
    masterXfer.dataSize = buffLen;

    return I2C_MasterTransferBlocking(I2C0, &masterXfer);
}

unsigned long  I2C_READ_PCF8653(uint8_t *pBuff, uint8_t reg)
{
	masterXfer.slaveAddress = PCF8563_I2C_ADDR;
    masterXfer.direction = kI2C_Write;
    masterXfer.data = &reg;
    masterXfer.dataSize = 1;
    I2C_MasterTransferBlocking(I2C0, &masterXfer);

    masterXfer.direction = kI2C_Read;
    masterXfer.data = pBuff;
    masterXfer.dataSize = 1;
    return I2C_MasterTransferBlocking(I2C0, &masterXfer);
}

void configRTC( void ){

	/* Pseudocódigo para configuração default
	 * Para o RTC (STOP)
	 * Escreve no registo de segundos o valor 0
	 * Escreve no registro de minutos o valor 0
	 * Escreve no registro de horas o valor 12 em BCD
	 * Escreve no registro de dias o valor 10 em BCD
	 * Escreve no registro de dias da semana o valor do dia atual
	 * Escreve no registro de mês o valor 10 em BCD
	 * Escreve no registro de ano o valor 22 em BCD
	 * Inicia o RTC (START)
	*/

	uint8_t g_master_txBuff[] = {Control_Status_1, STOPrtc};
	I2C_WRITE_PCF8653(g_master_txBuff, 2);

	g_master_txBuff[0] = Control_Status_2;
	g_master_txBuff[1] = 0x00;
	I2C_WRITE_PCF8653(g_master_txBuff, 2);

	g_master_txBuff[0] = VL_seconds;
	g_master_txBuff[1] = 0;
	I2C_WRITE_PCF8653(g_master_txBuff, 2);

	g_master_txBuff[0] = Minutes;
	g_master_txBuff[1] = 0;
	I2C_WRITE_PCF8653(g_master_txBuff, 2);

	g_master_txBuff[0] = Hours;
	g_master_txBuff[1] = dectobcd(12);
	I2C_WRITE_PCF8653(g_master_txBuff, 2);

	g_master_txBuff[0] = Days;
	g_master_txBuff[1] = dectobcd(21);
	I2C_WRITE_PCF8653(g_master_txBuff, 2);

	g_master_txBuff[0] = Weekdays;
	g_master_txBuff[1] = 5;
	I2C_WRITE_PCF8653(g_master_txBuff, 2);

	g_master_txBuff[0] = Century_months;
	g_master_txBuff[1] = dectobcd(10);
	I2C_WRITE_PCF8653(g_master_txBuff, 2);

	g_master_txBuff[0] = Years;
	g_master_txBuff[1] = dectobcd(22);
	I2C_WRITE_PCF8653(g_master_txBuff, 2);

	g_master_txBuff[0] = Control_Status_1;
	g_master_txBuff[1] = STARTrtc;
	I2C_WRITE_PCF8653(g_master_txBuff, 2);

	uint8_t g_master_rxBuff[1];
	I2C_READ_PCF8653(&g_master_rxBuff[0], Control_Status_1);
	I2C_READ_PCF8653(&g_master_rxBuff[0], Control_Status_2);
	I2C_READ_PCF8653(&g_master_rxBuff[0], VL_seconds);
	I2C_READ_PCF8653(&g_master_rxBuff[0], Minutes);
	I2C_READ_PCF8653(&g_master_rxBuff[0], Hours);
	I2C_READ_PCF8653(&g_master_rxBuff[0], Days);
	I2C_READ_PCF8653(&g_master_rxBuff[0], Weekdays);
	I2C_READ_PCF8653(&g_master_rxBuff[0], Century_months);
	I2C_READ_PCF8653(&g_master_rxBuff[0], Years);

}

void AtualizaHoraRTC( void ){

	clear_display_text();	// Limpa a tela
	unsigned char dia, mes, ano, hora, minuto;
	unsigned short posicaoDesenho = 186;
	unsigned char posicaoX = 6, posicaoY = 6, contDigito = 0, tecla = 16, primeiroDigito = 10, segundoDigito = 10;
	unsigned char posicaoDia = 186, posicaoMes = 189, posicaoAno = 194, posicaoHora = 199, posicaoMinuto = 202;
	I2C_READ_PCF8653( &dia, Days );	// Faz a leitura do dia
	I2C_READ_PCF8653( &mes, Century_months );	// Faz a leitura do mês
	I2C_READ_PCF8653( &ano, Years );	// Faz a leitura do ano
	I2C_READ_PCF8653( &hora, Hours );	// Faz a leitura da hora
	I2C_READ_PCF8653( &minuto, Minutes );	// Faz a leitura do minuto
	escrita_texto( 9, "CONFIGURA", sizeof("CONFIGURA") );	// Escreve CONFIGURA
	EscreveCedilhaAOTil();	// Chama função AOTil
	escrita_texto( posicaoDia, ConverteNumParaLcd( 2, 0, bcdtodec( dia & 0x3F ) ), ContaCaracteres() + 1 );	// Escreve o dia salvo com 2 dígitos
	escrita_texto( posicaoDesenho + 2, "/" , sizeof("/") );	// Escreve "/"
	escrita_texto( posicaoMes, ConverteNumParaLcd( 2, 0, bcdtodec( mes & 0x1F ) ), ContaCaracteres() + 1 );	// Escreve o mês salvo com 2 dígitos
	escrita_texto( posicaoDesenho + 5, "/20", sizeof("/20") );		// Escreve "/20"
	escrita_texto( posicaoAno, ConverteNumParaLcd( 2, 0, bcdtodec( ano ) ), ContaCaracteres() + 1 );	// Escreve o ano salvo com 2 dígitos
	escrita_texto( posicaoHora, ConverteNumParaLcd( 2, 0, bcdtodec( hora & 0x3F ) ), ContaCaracteres() + 1 );	// Escreve a hora salvo com 2 dígitos
	escrita_texto( posicaoDesenho + 15, ":" , sizeof(":") );	// Escreve ":"
	escrita_texto( posicaoMinuto, ConverteNumParaLcd( 2, 0, bcdtodec( minuto & 0x7F ) ), ContaCaracteres() + 1 );	// Escreve o minuto salvo com 2 dígitos
	escrita_texto( 400, "DATA E HORA", sizeof("DATA E HORA") );	// Escreve "DATA E HORA"
	escrita_texto( 450, "YES=SALVAR NO=SAIR", sizeof("YES=SALVAR NO=SAIR") );	// Escreve "YES=SALVAR NO=SAIR"
	escrita_texto( 471, "<=VOLTA", sizeof("<=VOLTA") );	// Escreve ←
	Cursor( posicaoX, posicaoY, 1, 1 );	// Desenha o cursor
	while( 1 ){
		tecla = verifyKeyBoard();	// Faz a leitura da tecla
		if( tecla != 16 ){	// Se tecla for diferente de 16
			if( tecla == no){	// Se tecla igual a NO
				return;
			}
			else if( tecla == yes ){	// Senão se tecla igual a YES
				if( contDigito % 2 == 0 ){	// Se contDigito % 2 é 0
					contDigito += 2;	// contDigito soma 2
				}
				else{	// Senão se contDigito % 2 é 1
					if( contDigito == 1 ){	// Se contDigito é 1
						if( primeiroDigito != zero ){	// Se o primeiroDigito é zero
							dia = primeiroDigito;	// dia recebe o primeiroDigito
						}
						else{	// Senão, escreve o valor anterior.
							escrita_texto( posicaoDia, ConverteNumParaLcd( 2, 0, bcdtodec( dia & 0x3F ) ), ContaCaracteres() + 1 );
						}
					}
					else if( contDigito == 3 ){	// Senão se contDigito é 3
						if( primeiroDigito != zero ){	// Se o primeiroDigito é zero
							mes = primeiroDigito;	// mes recebe o primeiroDigito
						}
						else{	// Senão, escreve o valor anterior.
							escrita_texto( posicaoMes, ConverteNumParaLcd( 2, 0, bcdtodec( mes & 0x1F ) ), ContaCaracteres() + 1 );
						}
					}
					else if( contDigito == 5 ){	// Senão se contDigito é 5
						if( primeiroDigito != zero ){	// Se o primeiroDigito é zero
							ano = primeiroDigito;	// ano recebe o primeiroDigito
						}
						else{	// Senão, escreve o valor anterior.
							escrita_texto( posicaoAno, ConverteNumParaLcd( 2, 0, bcdtodec( ano ) ), ContaCaracteres() + 1 );
						}
					}
					else if( contDigito == 7 ){	// Senão se contDigito é 7
						if( primeiroDigito != zero ){	// Se o primeiroDigito é zero
							hora = primeiroDigito;	// hora recebe o primeiroDigito
						}
						else{	// Senão, escreve o valor anterior.
							escrita_texto( posicaoHora, ConverteNumParaLcd( 2, 0, bcdtodec( hora & 0x3F ) ), ContaCaracteres() + 1 );
						}
					}
					else if( contDigito == 9 ){	// Senão se contDigito é 9
						if( primeiroDigito != zero ){	// Se o primeiroDigito é zero
							minuto = primeiroDigito;	// minuto recebe o primeiroDigito
						}
						else{	// Senão, escreve o valor anterior.
							escrita_texto( posicaoMinuto, ConverteNumParaLcd( 2, 0, bcdtodec( minuto & 0x7F ) ), ContaCaracteres() + 1 );
						}
					}
					contDigito++;	// contDigito incrementa de 1
				}
			}
			else if( tecla == left ){	// Senão se tecla é igual a ←
				if( contDigito % 2 == 0 ){	// Se contDigito % 2 é 0

					if( contDigito == 0 ){	// Se contDigito é 0
						escrita_texto( posicaoDia , ConverteNumParaLcd( 2, 0, bcdtodec( dia & 0x3F ) ), ContaCaracteres() + 1 );	// Escreve dia
					}
					else if( contDigito == 2 ){		// Senão se contDigito é 2
						escrita_texto( posicaoMes, ConverteNumParaLcd( 2, 0, bcdtodec( mes & 0x1F ) ), ContaCaracteres() + 1 );	// Escreve mes
					}
					else if( contDigito == 4 ){		// Senão se contDigito é 4
						escrita_texto( posicaoAno, ConverteNumParaLcd( 2, 0, bcdtodec( ano ) ), ContaCaracteres() + 1 );	// Escreve ano
					}
					else if( contDigito  == 6 ){	// Senão se contDigito é 6
						escrita_texto( posicaoHora, ConverteNumParaLcd( 2, 0, bcdtodec( hora & 0x3F ) ), ContaCaracteres() + 1 );// Escreve hora
					}
					else if( contDigito == 8 ){	// Senão se contDigito é 8
						escrita_texto( posicaoMinuto, ConverteNumParaLcd( 2, 0, bcdtodec( minuto & 0x7F ) ), ContaCaracteres() + 1 );	// Escreve minuto
					}
					if( contDigito == 0 ){	// Se contDigito é menor que 0
						contDigito = 0;	// contDigito recebe 0
					}
					else{
						contDigito -= 2;	// contDigito subtrai 2
					}
				}
				else{	// Senão se contDigito % 2 é 1
					if( contDigito == 1 ){	// Se contDigito é 0
						escrita_texto( posicaoDia + 1, " ", sizeof(" ") );	// Escreve dia
					}
					else if( contDigito == 3 ){		// Senão se contDigito é 2
						escrita_texto( posicaoMes + 1, " ", sizeof(" ") );	// Escreve dia
					}
					else if( contDigito == 5 ){		// Senão se contDigito é 4
						escrita_texto( posicaoAno + 1, " ", sizeof(" ") );	// Escreve dia
					}
					else if( contDigito  == 7 ){	// Senão se contDigito é 6
						escrita_texto( posicaoHora + 1, " ", sizeof(" ") );	// Escreve dia
					}
					else if( contDigito == 9 ){	// Senão se contDigito é 8
						escrita_texto( posicaoMinuto + 1, " ", sizeof(" ") );	// Escreve dia
					}
					contDigito--;	// contDigito subtrai 1
				}
			}
			else if( tecla != dot && tecla != hifen && tecla != right ) {// Senão se tecla é diferente de . , -
				if( contDigito % 2 == 0 ){	// Se contDigito % 2 é 0
					primeiroDigito = tecla;	// primeiroDigito recebe tecla
				}
				else{	// Senão
					segundoDigito = tecla;	// segundoDigito recebe tecla
				}
				if( contDigito < 2 ){	// Se contDigito menor que 2
					escrita_texto( posicaoDia + contDigito % 2, ConverteNumParaLcd(1, 0, tecla), ContaCaracteres() + 1);	// Escreve a tecla
				}
				else if( contDigito < 4 ){	// Se contDigito menor que 2
					escrita_texto( posicaoMes + contDigito % 2, ConverteNumParaLcd(1, 0, tecla), ContaCaracteres() + 1);	// Escreve a tecla
				}
				else if( contDigito < 6 ){	// Se contDigito menor que 2
					escrita_texto( posicaoAno + contDigito % 2, ConverteNumParaLcd(1, 0, tecla), ContaCaracteres() + 1);	// Escreve a tecla
				}
				else if( contDigito < 8 ){	// Se contDigito menor que 2
					escrita_texto( posicaoHora + contDigito % 2, ConverteNumParaLcd(1, 0, tecla), ContaCaracteres() + 1);	// Escreve a tecla
				}
				else{	// Se contDigito menor que 2
					escrita_texto( posicaoMinuto + contDigito % 2, ConverteNumParaLcd(1, 0, tecla), ContaCaracteres() + 1);	// Escreve a tecla
				}
				contDigito++;	// contDigito incrementa 1
				if( contDigito == 2 ){	// Se contDigito igual a 2
					dia = dectobcd( primeiroDigito*10 + segundoDigito );// dia recebe primeiroDigito * 10 + segundoDigito
				}
				else if( contDigito == 4 ){	// Se contDigito igual a 2
					mes = dectobcd( primeiroDigito*10 + segundoDigito );// dia recebe primeiroDigito * 10 + segundoDigito
				}
				else if( contDigito == 6 ){	// Se contDigito igual a 2
					ano = dectobcd( primeiroDigito*10 + segundoDigito );// dia recebe primeiroDigito * 10 + segundoDigito
				}
				else if( contDigito == 8 ){	// Se contDigito igual a 2
					hora = dectobcd( primeiroDigito*10 + segundoDigito );// dia recebe primeiroDigito * 10 + segundoDigito
				}
				else{	// Se contDigito igual a 2
					minuto = dectobcd( primeiroDigito*10 + segundoDigito ); // dia recebe primeiroDigito * 10 + segundoDigito
				}

			}
			if( contDigito == 10 ){	// Se contDigito é 10
				/* Armazena os valores no RTC */
				uint8_t g_master_txBuff[] = {Control_Status_1, STOPrtc};	// Para o RTC
				I2C_WRITE_PCF8653(g_master_txBuff, 2);

				// Configura o RTC
				g_master_txBuff[0] = Control_Status_2;
				g_master_txBuff[1] = 0x00;
				I2C_WRITE_PCF8653(g_master_txBuff, 2);

				// Escreve os segundos
				g_master_txBuff[0] = VL_seconds;
				g_master_txBuff[1] = 0;
				I2C_WRITE_PCF8653(g_master_txBuff, 2);

				// Escreve os minutos
				g_master_txBuff[0] = Minutes;
				g_master_txBuff[1] = minuto ;
				I2C_WRITE_PCF8653(g_master_txBuff, 2);

				// Escreve as horas
				g_master_txBuff[0] = Hours;
				g_master_txBuff[1] = hora;
				I2C_WRITE_PCF8653(g_master_txBuff, 2);

				// Escreve o dia
				g_master_txBuff[0] = Days;
				g_master_txBuff[1] = dia;
				I2C_WRITE_PCF8653(g_master_txBuff, 2);

				// Escreve a semana
				g_master_txBuff[0] = Weekdays;
				g_master_txBuff[1] = 5;
				I2C_WRITE_PCF8653(g_master_txBuff, 2);

				// Escreve os meses
				g_master_txBuff[0] = Century_months;
				g_master_txBuff[1] = mes;
				I2C_WRITE_PCF8653(g_master_txBuff, 2);

				// Escreve os anos
				g_master_txBuff[0] = Years;
				g_master_txBuff[1] = ano;
				I2C_WRITE_PCF8653(g_master_txBuff, 2);

				// Inicia o RTC
				g_master_txBuff[0] = Control_Status_1;
				g_master_txBuff[1] = STARTrtc;
				I2C_WRITE_PCF8653(g_master_txBuff, 2);
				return;
			}
		}
		if( contDigito < 2 ){	// Se contDigito menor que 2
			Cursor( posicaoDia % 180 + contDigito % 2, posicaoY, 1, 1 );// Move o cursor de acordo com o contDigito
		}
		else if( contDigito < 4 ){	// Se contDigito menor que 2
			Cursor( posicaoMes % 180 + contDigito % 2, posicaoY, 1, 1 );// Move o cursor de acordo com o contDigito
		}
		else if( contDigito < 6 ){	// Se contDigito menor que 2
			Cursor( posicaoAno % 180 + contDigito % 2,  posicaoY, 1, 1 );// Move o cursor de acordo com o contDigito
		}
		else if( contDigito < 8 ){	// Se contDigito menor que 2
			Cursor( posicaoHora % 180 + contDigito % 2, posicaoY, 1, 1 );// Move o cursor de acordo com o contDigito
		}
		else{	// Se contDigito menor que 2
			Cursor( posicaoMinuto % 180 + contDigito % 2, posicaoY, 1, 1 );// Move o cursor de acordo com o contDigito
		}
	}
}
