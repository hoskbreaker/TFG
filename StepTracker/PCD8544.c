//---------------------------------------------------------------//
// Nombre: Jose Carlos Sánchez Morales
// Proyecto: registro de pasos basado en MSP430G2553
// Titulación: Ingeniería de sistemas electrónicos
// Curso: 2017-2018
// Archivo: PCD8544.c
//---------------------------------------------------------------//
#include <msp430g2553.h>
#include "PCD8544.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//***************************************************************************************
//
//         NOKIA 5110 LCD                                               MSP-EXP430G2
//       -----------------                                           -------------------
//      |              GND|<-- tierra ------------------------------|J6     GND         |
//      |               BL|<-- luz de fondo ---(transistor)---------|		P1.5
//      |              VCC|<-- Vcc 3.3v --------------------------|J1.1   VCC         |
//      |                 |
//      |              CLC|<-- Reloj -------------------------------|J1.7   P1.4        |
//      |              DIN|<-- Entrada de datos---------------------|J2.15  P1.2        |
//      |               DC|<-- Data/Command (high/low) -------------|J1.3   P1.3        |
//      |               CE|<-- Chip Enable (active low) ------------|J1.2   P1.0        |
//      |              RST|<-- Reset -------------------------------|J2.16  RST
//
//
//
//***************************************************************************************

//variables locales

static volatile uint8_t stop[3]={0,0,0};
static volatile uint8_t fila = PUNTO_FILA_INICIAL;
static volatile unsigned int af=0;
static volatile unsigned int ac=0;
static uint8_t f_ant[5]={0,0,0,0,0};
static uint8_t c_ant[5]={0,0,0,0,0};
static volatile uint8_t columna = PUNTO_COLUMNA_INICIAL;
static volatile uint8_t mov;

//----------SUBPROGRAMAS ACELERÓMETRO--------------------//
//	*********************************************************************** //
// 1) void setAddr(unsigned char xAddr, unsigned char yAddr):
//	->unsigned char xAddr: dirección columna (0..83)
//	->unsigned char yAddr: dirección fila (0..5)

//	Descripción:
//		selecciona el punto en la matriz de pixeles de la pantalla
//	*********************************************************************** //
void setAddr(unsigned char xAddr, unsigned char yAddr)
{
    writeToLCD(LCD5110_COMMAND, PCD8544_SETXADDR | xAddr);
    writeToLCD(LCD5110_COMMAND, PCD8544_SETYADDR | yAddr);
}
//	*********************************************************************** //
// 2) void writeToLCD(unsigned char dataCommand, unsigned char data):
//	->unsigned char dataCommand: tipo de datos a enviar
//	->unsigned char data: datos a enviar

//	Descripción:
//		escribe datos en la LCD. Los datos pueden ser
//		comandos de actuacion en la LCD, o datos para
//		mostrar en la pantalla.
//	*********************************************************************** //
void writeToLCD(unsigned char dataCommand, unsigned char data) {
    LCD5110_SELECT;

    if(dataCommand) {
        LCD5110_SET_DATA;
    } else {
        LCD5110_SET_COMMAND;
    }

    UCA0TXBUF = data;
    while(!(IFG2 & UCA0TXIFG));
    LCD5110_DESELECT;
}
//	*********************************************************************** //
// 3) void initLCD():

//	Descripción:
//		inicializa la configuración para el funcionamiento
//		de la pantalla LCD.
//	*********************************************************************** //
void initLCD() {

    P1OUT |= LCD5110_SCE_PIN | LCD5110_DC_PIN;  // Disable LCD, set Data mode
    P1DIR |= LCD5110_SCE_PIN | LCD5110_DC_PIN;  // Set pins to output direction

    // Setup USIA
    P1SEL |= LCD5110_SCLK_PIN | LCD5110_DN_PIN;
    P1SEL2 |= LCD5110_SCLK_PIN | LCD5110_DN_PIN;

    UCA0CTL0 |= UCCKPH | UCMSB | UCMST | UCSYNC; // 3-pin, 8-bit SPI master
    UCA0CTL1 |= UCSSEL_2;               // SMCLK
    UCA0BR0 |= 0x01;                    // 1:1
    UCA0BR1 = 0;
    UCA0CTL1 &= ~UCSWRST;               // clear SW

    __delay_cycles(500000);             //se puede quitar?

    writeToLCD(LCD5110_COMMAND, PCD8544_FUNCTIONSET | PCD8544_EXTENDEDINSTRUCTION);
    writeToLCD(LCD5110_COMMAND, PCD8544_SETVOP | 0x3F);
    writeToLCD(LCD5110_COMMAND, PCD8544_SETTEMP | 0x02);
    writeToLCD(LCD5110_COMMAND, PCD8544_SETBIAS | 0x03);
    writeToLCD(LCD5110_COMMAND, PCD8544_FUNCTIONSET);
    writeToLCD(LCD5110_COMMAND, PCD8544_DISPLAYCONTROL | PCD8544_DISPLAYNORMAL);
}
//	*********************************************************************** //
// 4) void writeCharToLCD(char c):
//	->char c: caracter a escribir

//	Descripción:
//		Escribe un caracter en la pantalla
//	*********************************************************************** //
void writeCharToLCD(char c) {
    unsigned char i;
    for(i = 0; i < 5; i++) {
        writeToLCD(LCD5110_DATA, font[c - 0x20][i]);
    }
    writeToLCD(LCD5110_DATA, 0);
}
//	*********************************************************************** //
// 5) void writeStringToLCD(const char *string):
//	->const char *string: palabra a escribir

//	Descripción:
//		escribe palabra por pantalla
//	*********************************************************************** //
void writeStringToLCD(const char *string) {
    while(*string) {
        writeCharToLCD(*string++);
    }
}
//	*********************************************************************** //
// 6) void clearLCD():

//	Descripción:
//		elimina caracteres de la pantalla
//	*********************************************************************** //
void clearLCD() {
    setAddr(0, 0);
    int i = 0;
    while(i < PCD8544_MAXBYTES) {
        writeToLCD(LCD5110_DATA, 0);
        i++;
    }
    setAddr(0, 0);
}
//	*********************************************************************** //
// 6) void clearBank(unsigned char bank):
//	->unsigned char bank: valor de banco de memoria

//	Descripción:
//		elimina el valor del banco de memoria
//	*********************************************************************** //
void clearBank(unsigned char bank) {
    setAddr(0, bank);
    int i = 0;
    while(i < PCD8544_HPIXELS) {
        writeToLCD(LCD5110_DATA, 0);
        i++;
    }
    setAddr(0, bank);
}

//	*********************************************************************** //
// 7) void visible(int x):
//	->int x: valor de visibilidad

//	Descripción:
//		Si x es mayor a 0, es visible, mientras que 
//		si x es menor que -10, no lo es.
//	*********************************************************************** //

int visible(int x)
{
    uint8_t aux = 0;
    if(stop[0]==0){
        if(x>=0){
            stop[0] = 1;
            aux = 0;
        }
    }else if(stop[0] == 1){
        if( x<=-10){
            stop[0] = 0;
            aux=1;
        }
    }
    return aux;
}
//	*********************************************************************** //
// 8) void punto(uint8_t fila, uint8_t columna, char *caracter):
//	->uint8_t fila: valor de la fila a actuar
//	->uint8_t columna: valor de la columna a actuar
//	->uint8_t *caracter: caracter o cadena de caracteres a escribir

//	Descripción:
//		Dibuja el caracter o cadena de caracteres
//		deseado en el punto indicado por la columna
//		y la fila.
//	*********************************************************************** //

void punto(uint8_t fila, uint8_t columna, char *caracter)
{
    setAddr(fila, columna);
    writeStringToLCD(caracter);
}

//	*********************************************************************** //
// 9) int direccion(int d):
//	->int d: dirección tomada.

//	Descripción:
//		detecta si el usuario a cambiado la dirección
//		o la ha mantenido.
//	*********************************************************************** //
int direccion(int d)
{
    int aux=0;
    if(stop[1]==0){
       if(d>=20){
           //izquierda
           stop[1] = 1;
       }else if(d<=-20){
           //derecha
           stop[1]=2;
       }
   }else if(stop[1] == 1){
       //completa izquierda
       if( d<=-20){
           stop[1] = 0;
           aux=-1;
       }
   }else if(stop[1]==2){
       //completa derecha
       if( d<=-20){
          stop[1] = 0;
          aux=1;
      }
   }
   return aux;
}
//	*********************************************************************** //
// 10) uint8_t paso(int p):
//	->int p: paso cometido.

//	Descripción:
//		detecta si el usuario a dado un paso o no.
//	*********************************************************************** //
uint8_t paso(int p)
{
    uint8_t aux = 0;
    if(stop[2]==0){
        if(p>=20){
            stop[2] = 1;
        }
    }else if(stop[2] == 1){
        if( p<=-20){
            stop[2] = 0;
            aux=1;
        }
    }
    return aux;
}
//	*********************************************************************** //
// 11) void mover (int d):
//	->int d: movimiento dado.

//	Descripción:
//		actúa en la pantalla indicando el movimiento cometido
//		por el usuario.
//	*********************************************************************** //
void mover (int d)
{
    unsigned int i;
    movimiento(d);
    f_ant[af]=fila;
    c_ant[ac]=columna;
    fila = fila + Fila[mov];
    columna = columna +Columna[mov];
    af++;
    ac++;
    if(ac<=5 && af<=5){
        for(i=0;i<4;i++){
            f_ant[i]=f_ant[i+1];
            c_ant[i]=c_ant[i+1];
        }
        ac=4;
        af=4;
    }
}
//	*********************************************************************** //
// 12) void recuadro():

//	Descripción:
//		Marco gráfico que dibuja un recuadro,
//		indicando el área límite
//	*********************************************************************** //
void recuadro()
{
    int i;
    for(i=0;i<40;i++){
        punto(0,i/2,"|");
        punto(i*2,21,"-");
        punto(80,i/2,"|");
        punto(i*2,0,"-");
    }
    punto(0,0,"+");
    punto(0,21,"+");
    punto(80,0,"+");
    punto(80,21,"+");
}
//	*********************************************************************** //
// 13) void dibujar ():

//	Descripción:
//		Realiza todos los cambios gráficos
//		en la pantalla
//	*********************************************************************** //
void dibujar ()
{
    int i;
    clearLCD();
    recuadro();
    for (i=0; i<5; i++){
        if(c_ant[i]!= 0 && f_ant[i] != 0){
            punto(c_ant[i], f_ant[i], ".");
        }
    }
    punto(columna, fila, "*");
}
//	*********************************************************************** //
// 14) void movimiento (int d):
//	->int d: tipo de movimiento hecho

//	Descripción:
//		Indica el tipo de dibujo a hacer por pantalla
//		dependiendo del movimiento del usuario
//	*********************************************************************** //
void movimiento (int d)
{
    mov= mov + d;

    if(mov>=8){
        mov = 0;
    }else if(mov<0){
        mov = 7;
    }
}
