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



#define FTM_SOURCE_CLOCK (CLOCK_GetFreq(kCLOCK_BusClk) / 4)
#define DEMO_ADC16_BASE          ADC0
#define DEMO_ADC16_CHANNEL_GROUP 0U
#define DEMO_ADC16_USER_CHANNEL  15U
#define TIME_MUX				 3	// Cada 1 vale 5 milissegundos. Nesse caso 15 milissegundos
#define TIME_MOTOR				 6  // Cálculo para 30 milissegundos


#define ma1	16
#define ma2	17
#define ma3	18
#define ma4	19
#define mb1	20
#define mb2	21
#define mb3	22
#define mb4	23
#define ma1_on	GPIO_PinWrite( GPIOB, ma1, 0 )
#define ma2_on	GPIO_PinWrite( GPIOB, ma2, 0 )
#define ma3_on	GPIO_PinWrite( GPIOB, ma3, 0 )
#define ma4_on	GPIO_PinWrite( GPIOB, ma4, 0 )
#define mb1_on	GPIO_PinWrite( GPIOB, mb1, 0 )
#define mb2_on	GPIO_PinWrite( GPIOB, mb2, 0 )
#define mb3_on	GPIO_PinWrite( GPIOB, mb3, 0 )
#define mb4_on	GPIO_PinWrite( GPIOB, mb4, 0 )

#define ma1_off	GPIO_PinWrite( GPIOB, ma1, 1 )
#define ma2_off	GPIO_PinWrite( GPIOB, ma2, 1 )
#define ma3_off	GPIO_PinWrite( GPIOB, ma3, 1 )
#define ma4_off	GPIO_PinWrite( GPIOB, ma4, 1 )
#define mb1_off	GPIO_PinWrite( GPIOB, mb1, 1 )
#define mb2_off	GPIO_PinWrite( GPIOB, mb2, 1 )
#define mb3_off	GPIO_PinWrite( GPIOB, mb3, 1 )
#define mb4_off	GPIO_PinWrite( GPIOB, mb4, 1 )

#define SPEEDMUX1	1
#define SPEEDMUX2	2
#define POSITION1	1
#define POSITION2	2
#define POSITION3	3
#define POSITION4	4
#define POSITION5	5

#define SPEEDTRP1	1
#define SPEEDTRP2	2
#define SPEEDTRP3	3
#define SPEEDTRP4	4
#define WAYAHOUR	0
#define WAYHOUR		1
#define TIMER1		250
#define TIMER2		838
#define TIMER3		2000
#define PULSE_TM	52
#define END_TM		6640
#define END_TM2		15000
#define TIME_MEAS	300

void motores_run( void );
void move_mux( unsigned char posicao, unsigned char velocidade );
unsigned char move_tripa( unsigned char sentido, unsigned char velocidade, unsigned int tempo_on );

