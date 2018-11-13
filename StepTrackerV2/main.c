//---------------------------------------------------------------//
// Nombre: Jose Carlos S�nchez Morales
// Proyecto: registro de pasos basado en MSP430G2553
// Titulaci�n: Ingenier�a de sistemas electr�nicos
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
static volatile int posicion;
static volatile char barra;
//-----SUBPROGRAMAS MAIN-------//
//	*********************************************************************** //
// 1) int normalizar(uint8_t x):
//	->uint8_t x: valor a normalizar 

//	Descripci�n:
//		centra el valor de un eje del aceler�metro entre (-127,127),
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
//	*********************************************************************** //
// 2) int main(void):

//	Descripci�n:
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

	/* comprobacion de las constantes de calibracion estan correctas */
	if (CALBC1_1MHZ == 0xff || CALDCO_1MHZ == 0xff) {
		for (;;)
			;
	}
	/* carga configuracion de calibraci�n para reloj de 1 MHz DCO */
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

	/* activa interrupci�n */
	__bis_SR_register(GIE);

	/*inicia modo I2C */
	init_i2c();
	/* Inicia configuraci�n LCD */
	initLCD();
	/* Inicia configuraci�n del aceler�metro */
	MMA8451Init();
	/* Inicia configuraci�n del ADC */
	initADClux();
	
	//limpia pantalla
	clearLCD();
	// Mensaje de inicio en la aplicaci�n
	punto(10,10,"Step-Tracker");
	
	for(;;) {

	    __low_power_mode_0();
	    if(flag){
	        flag = 0;
	        eje_x=normalizar(MMA8451GetXAxis());	//valor eje x normalizado
            eje_y=normalizar(MMA8451GetYAxis());	//valor eje y normalizado
            eje_z=normalizar(MMA8451GetZAxis());	//valor eje z normalizado

            //si el aceler�metro detecta un paso
            if(paso(eje_z)){
                //si el paso incluye cambio de direcci�n
                mover(direccion(eje_y));
            }
            //si el usuario quiere mirar la pantalla
            if(visible(eje_x)){
                //detectar si la luz ambiente da visibilidad
                ADCluxLevel(1);
                //refresca la pantalla con el movimiento actualizado
                dibujar();
            }
	    }
	}

}
//	*********************************************************************** //
// 3) timer_interrupt(void):

//	Descripci�n:
//		funci�n de interrupci�n del timer. 
//		Cuenta un tick por cada ciclo de reloj.
//	*********************************************************************** //
#pragma vector = TIMER0_A0_VECTOR
__interrupt void RTI_T0_TACCR0(void) {
    flag = 1;
    __low_power_mode_off_on_exit(); // Sale bajo consumo (LPM0)
}