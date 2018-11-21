//---------------------------------------------------------------//
// Nombre: Jose Carlos Sánchez Morales
// Proyecto: registro de pasos basado en MSP430G2553
// Titulación: Ingeniería de sistemas electrónicos
// Curso: 2017-2018
// Archivo: adc.c
//---------------------------------------------------------------//
#include "adc.h"

#define UMBRAL 0x01B5
#define ON     1
#define OFF    0


//	*********************************************************************** //
// 1) void initADClux(void):

//	Descripción:
//		inicializa la lectura de valores analógicos
//		en el MSP430G2553.
//	*********************************************************************** //
void initADClux(void)
{
  //WDTCTL = WDTPW + WDTHOLD;                     // Stop WDT 1.5
  ADC10CTL0 = SREF_0 + ADC10SHT_2 + ADC10ON + ADC10IE;   //set ADC10’s sample and hold time + ADC10 converter on + enables the ADC interrupt
  ADC10CTL1 = INCH_5;                           //Set channel A5
  ADC10AE0 |= BIT5;                             //Set pin 1.5
  P1DIR |= BIT1;                            // Set P1.1 to output direction
}
//	*********************************************************************** //
// 2) void initADClux(int allow):
//	->int allow: variable de permiso de activación

//	Descripción:
//		Activa o desactiva la lectura del ADC
//	*********************************************************************** //
void ADCluxLevel(uint8_t adc)
{

}
