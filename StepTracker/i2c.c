//---------------------------------------------------------------//
// Nombre: Jose Carlos Sánchez Morales
// Proyecto: registro de pasos basado en MSP430G2553
// Titulación: Ingeniería de sistemas electrónicos
// Curso: 2017-2018
// Archivo: i2c.c
//---------------------------------------------------------------//
#include "i2c.h"

//rutinas de interrupción USCI//
void rx_interrupt(void) __attribute__((interrupt(USCIAB0RX_VECTOR)));
void tx_interrupt(void) __attribute__((interrupt(USCIAB0TX_VECTOR)));

//variables locales de I2C//
static int busy_flag;
static uint8_t *tx_data, tx_byte_count;
static uint8_t *rx_data, rx_byte_count;
static int tx_stop_flag, nack_flag;

//-----SUBPROGRAMAS I2C-------//
//	*********************************************************************** //
// 1) Init_i2c(void): 

//	Descripción:
//		Se encarga de iniciar el modo en I2C
//		en el MSP430G2553.
//	*********************************************************************** //
void init_i2c(void)
{
	//desactiva USCI B
	UCB0CTL1 |= UCSWRST;	
	/* Activa modo maestro con 7 bits de direccion esclavo */
	UCB0CTL0 = UCSYNC | UCMODE_3 | UCMST;
	/* Selecciona SMCLK como reloj fuente */
	UCB0CTL1 |= UCSSEL_2;
	/* UCBR = f_SMCLK / f_BitClock = 1 MHz / 100 kHz = 10 */
	UCB0BR0 = 10;
	UCB0BR1 = 0;
	
	//CONFIGURACION PIN I2C

	/* configura P1.6 como SCL */
	P1SEL |= BIT6;
	P1SEL2 |= BIT6;
	/* configura P1.7 como SDA */
	P1SEL |= BIT7;
	P1SEL2 |= BIT7;
	/* reactiva USCI B */
	UCB0CTL1 &= ~UCSWRST;
	/* activa interrupciones Tx/Rx */
	IE2 |= UCB0TXIE | UCB0RXIE;
	/* desactiva interrupciones STOP, START, y perdidas arbitrarias */
	UCB0I2CIE &= ~(UCSTPIE | UCSTTIE | UCALIE);
	/* activa interrupción NACK */
	UCB0I2CIE |= UCNACKIE;
	/* declara variables de control de linea */
	busy_flag = 0; //linea ocupada
	nack_flag = 0; //dato no recibido (o linea vacia)
}

//	*********************************************************************** //
// 2) int i2c_busy(void)

//	Descripción:
//		Devuelve el valor de la variable busy_flag.
//		si la linea esta ocupada, devuelve un 1, si
//		está libre, devuelve un 0.
//	*********************************************************************** //
int i2c_busy(void)
{
	return busy_flag;
}

//	*********************************************************************** //
// 3) int i2c_nack_received(void)

//	Descripción:
//		Devuelve el valor de la variable nack_flag.
//		si el esclavo no ha enviado la confirmación de
//		la recepción del dato, devuelve un 1. En caso
//		contrario, devuelve un 0.
//	*********************************************************************** //
int i2c_nack_received(void)
{
	return nack_flag;
}
//	*********************************************************************** //
// 4) void i2c_write(uint8_t slave_addr, uint8_t *data, int count, int stop)
//
//		-> uint8_t slave_addr: dirección del dispositivo esclavo
//		-> uint8_t *data: puntero apuntando a la dirección de memoria del dato.
//		-> int count: variable que indica la cuenta de datos a enviar
//		-> int stop: variable que indica la señal de stop.

//	Descripción:
//		Se encarga de escribir datos en el bus de transmisión de I2C.
//	*********************************************************************** //
void i2c_write(uint8_t slave_addr, uint8_t *data, int count, int stop)
{
	tx_data = data;
	tx_byte_count = count;
	tx_stop_flag = stop;

	busy_flag = 1;
	nack_flag = 0;

	/* establece direccion del dispositivo esclavo */
	UCB0I2CSA = slave_addr;
	/* selecciona modo Tx */
	UCB0CTL1 |= UCTR;
	/* genera la condicion START (comienzo) y comienza la transmisión */
	UCB0CTL1 |= UCTXSTT;
}

//	*********************************************************************** //
// 5) void i2c_read(uint8_t slave_addr, uint8_t *data, int count)
//
//		-> uint8_t slave_addr: dirección del dispositivo esclavo
//		-> uint8_t *data: puntero apuntando a la dirección de memoria del dato.
//		-> int count: variable que indica la cuenta de datos a enviar

//	Descripción:
//		Se encarga de leer los datos del bus de recepción de I2C
//	*********************************************************************** //
void i2c_read(uint8_t slave_addr, uint8_t *data, int count)
{
	rx_data = data;

	busy_flag = 1;
	nack_flag = 0;

	/* establece direccion del dispositivo esclavo */
	UCB0I2CSA = slave_addr;
	/* selecciona modo Rx */
	UCB0CTL1 &= ~UCTR;

	if (count == 1) {
		rx_byte_count = 0;

		/* genera la condicion START (comienzo) y comienza la transmisión */
		UCB0CTL1 |= UCTXSTT;
		/* Espera a que el esclavo responda a la direccion establecida */
		while (UCB0CTL1 & UCTXSTT)
			;
		/* genera condición de STOP */
		UCB0CTL1 |= UCTXSTP;
	} else {
		rx_byte_count = count - 1;

		/* genera la condicion START (comienzo) y comienza la recepción */
		UCB0CTL1 |= UCTXSTT;
	}
}
//	*********************************************************************** //
// 6) void rx_interrupt(void)

//	Descripción:
//		rutina de interrupción para la recepción de datos
//	*********************************************************************** //
void rx_interrupt(void)
{
	/* comprueba NACK enviado por el esclavo */
	if (UCB0STAT & UCNACKIFG) {
		nack_flag = 1;
		/* resetea variable de interrupción NACK */
		UCB0STAT &= ~UCNACKIFG;
		/* genera la condición STOP */
		UCB0CTL1 |= UCTXSTP;
		/* espera hasta que la señal STOP ha sido enviada */
		while (UCB0CTL1 & UCTXSTP)
			;
		busy_flag = 0;
	}
}
//	*********************************************************************** //
// 7) void tx_interrupt(void)

//	Descripción:
//		rutina de interrupción para la transmisión de datos
//	*********************************************************************** //
void tx_interrupt(void)
{
	/* está el buffer de transmisión lleno? */
	if (IFG2 & UCB0TXIFG) {
		if (tx_byte_count--) {
			/* carga el dato en el buffer Tx */
			UCB0TXBUF = *tx_data++;
		} else {
			/* resetea la señal de iterrupción TX */
			IFG2 &= ~UCB0TXIFG;
			if (tx_stop_flag) {
				/* genera la condición STOP */
				UCB0CTL1 |= UCTXSTP;
				/* espera hasta que la condición STOP sea enviada */
				while (UCB0CTL1 & UCTXSTP)
					;
			}
			busy_flag = 0;
		}
	}
	/* comprueba si los bytes han sido recibidos */
	if (IFG2 & UCB0RXIFG) {
		/* último byte? */
		if (rx_byte_count == 0) {
			/* toma datos del buffer de recepción */
			*rx_data = UCB0RXBUF;
			busy_flag = 0;
		} else {
			/* second to last byte? */
			if (rx_byte_count == 1) {
				/* genera la condición STOP despues de la recepción del ultimo byte */
				UCB0CTL1 |= UCTXSTP;
			}
			/* toma el dato */
			*rx_data++ = UCB0RXBUF;
			--rx_byte_count;
		}
	}
}
