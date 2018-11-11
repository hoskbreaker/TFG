//---------------------------------------------------------------//
// Nombre: Jose Carlos Sánchez Morales
// Proyecto: registro de pasos basado en MSP430G2553
// Titulación: Ingeniería de sistemas electrónicos
// Curso: 2017-2018
// Archivo: i2c.h
//---------------------------------------------------------------//
#ifndef I2C_H
#define I2C_H

#include <msp430.h>
#include <stdint.h>

void init_i2c(void);
void i2c_write(uint8_t, uint8_t *, int, int);
void i2c_read(uint8_t, uint8_t *, int);
int i2c_busy(void);
int i2c_nack_received(void);

enum {STOP = 1, RSTART = 0};

#endif
