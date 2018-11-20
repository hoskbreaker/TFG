//---------------------------------------------------------------//
// Nombre: Jose Carlos Sánchez Morales
// Proyecto: registro de pasos basado en MSP430G2553
// Titulación: Ingeniería de sistemas electrónicos
// Curso: 2017-2018
// Archivo: main.c
//---------------------------------------------------------------//
#include <msp430.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "i2c.h"
#include "mma8451.h"
#include "PCD8544.h"
#include "adc.h"

//variables locales
uint8_t flag = 0;
uint8_t adc = 0;
//-----SUBPROGRAMAS MAIN-------//
//	*********************************************************************** //
// 1) int normalizar(uint8_t x):
//	->uint8_t x: valor a normalizar 

//	Descripción:
//		centra el valor de un eje del acelerómetro entre (-127,127),
//		siendo 0 el valor de equilibrio.
//	*********************************************************************** //
int normalizar(uint8_t x)
{
    int norm;
    if(x>127){
        norm=x-255;
    }else{
        norm=x;
    }
    return norm;
}

void TimerConfiguration(){
    /* comprobacion de las constantes de calibracion estan correctas */
      if (CALBC1_1MHZ == 0xff || CALDCO_1MHZ == 0xff) {
          for (;;)
              ;
      }
      /* carga configuracion de calibración para reloj de 1 MHz DCO */
      BCSCTL1 = CALBC1_1MHZ;
      DCOCTL = CALDCO_1MHZ;

      /* selecciona SMCLK como reloj fuente para el timer A0, resetea el timer */
      TA0CTL = TASSEL_2 | TACLR;
      /* divide el valor del reloj por 8: 125 kHz */
      TA0CTL |= ID_3;
      /* establece el valor de comparacion en 2 Hz frecuencia de ticks */
      TA0CCR0 = 62499;
      /* activa el interruptor de comparacion */
      TA0CCTL0 = CCIE;
      /* inicia el timer (up mode) */
      TA0CTL |= MC_1;
}
//	*********************************************************************** //
// 2) int main(void):

//	Descripción:
//		Subprograma principal. Engloba las llamadas a
//		los distintos subprogramas de los distintos 
//		dispositivos conectados.
//	*********************************************************************** //
int main()
{
	//uint32_t start;
	int eje_x, eje_y, eje_z;

	/* desactiva watchdog */
	WDTCTL = WDTPW | WDTHOLD;

	/*configura timer para interrupcion*/
	TimerConfiguration();

	/*configura P2.0 para interrupcion*/
	//MMA8451GPIOInterruptConfiguration( );

	/*inicia modo I2C */
	init_i2c();
	/* Inicia configuración LCD */
	initLCD();
	/* Inicia configuración del ADC */
	initADClux();
	//limpia pantalla
	clearLCD();
	// Mensaje de inicio en la aplicación
	//punto(10,10,"Step-Tracker");
    /* activa interrupción */
    __bis_SR_register(GIE);
    /* Inicia configuración del acelerómetro */
    MMA8451Init();
	
	for(;;) {

	    __low_power_mode_0();
	    if(flag){

	        //MMA8451StandBy();
	        flag = 0;
	        eje_x=normalizar(MMA8451GetXAxis());	//valor eje x normalizado
            eje_y=normalizar(MMA8451GetYAxis());	//valor eje y normalizado
            eje_z=normalizar(MMA8451GetZAxis());	//valor eje z normalizado

            //si el acelerómetro detecta un paso
            paso(eje_z);
            direccion(eje_y);
            dibujar();
            ADCluxLevel(adc);
//            //si el usuario quiere mirar la pantalla
//            if(visible(eje_x)){
//                //detectar si la luz ambiente da visibilidad
//
//                //refresca la pantalla con el movimiento actualizado
//                dibujar();
//            }
            //MMA8451Active();
	    }
	}

}
//	*********************************************************************** //
// 3) timer_interrupt(void):

//	Descripción:
//		función de interrupción del timer. 
//		Cuenta un tick por cada ciclo de reloj.
//	*********************************************************************** //
#pragma vector = TIMER0_A0_VECTOR
__interrupt void RTI_T0_TACCR0(void) {
    flag = 1;
    ADC10CTL0 |= ENC + ADC10SC;             // comienza muestreo y conversiÃ³n del valor
    __low_power_mode_off_on_exit(); // Sale bajo consumo (LPM0)
}
//  *********************************************************************** //
// 3) timer_interrupt(void):

//  Descripción:
//      función de interrupción del timer.
//      Cuenta un tick por cada ciclo de reloj.
//  *********************************************************************** //
//#pragma vector = PORT2_VECTOR
//__interrupt void RTI_PORT2(void) {
//    flag = 1;
//    P2IFG &= ~BIT0; //P2.0 clear interrupt flag
//    __low_power_mode_off_on_exit(); // Sale bajo consumo (LPM0)
//}
//INTERRUPCION DEL ADC
#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void)
{
    adc = 1; // Sale bajo consumo (LPM0)
};
