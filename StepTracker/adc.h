//---------------------------------------------------------------//
// Nombre: Jose Carlos Sánchez Morales
// Proyecto: registro de pasos basado en MSP430G2553
// Titulación: Ingeniería de sistemas electrónicos
// Curso: 2017-2018
// Archivo: adc.h
//---------------------------------------------------------------//
#ifndef ADC_H
#define ADC_H

#include <msp430.h>
#include <stdint.h>

void initADClux(void);
void ADCluxLevel(int allow);

#endif
