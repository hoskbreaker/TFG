//---------------------------------------------------------------//
// Nombre: Jose Carlos Sánchez Morales
// Proyecto: registro de pasos basado en MSP430G2553
// Titulación: Ingeniería de sistemas electrónicos
// Curso: 2017-2018
// Archivo: mma8451.c
//---------------------------------------------------------------//
#include <stdbool.h>
#include "i2c.h"
#include "mma8451.h"
//----------SUBPROGRAMAS ACELERÓMETRO--------------------//
//	*********************************************************************** //
// 1) static inline void write_byte(uint8_t, uint8_t, uint8_t):

//	Descripción:
//		Escribe un byte para enviar al acelerómetro
//	*********************************************************************** //
static inline void write_byte(uint8_t, uint8_t, uint8_t);
//	*********************************************************************** //
// 2) static inline void write_word(uint8_t, uint8_t, uint16_t):

//	Descripción:
//		Escribe una palabra (conjunto de bytes) para enviar al acelerómetro
//	*********************************************************************** //
static inline void write_word(uint8_t, uint8_t, uint16_t);
//	*********************************************************************** //
// 3) static inline uint8_t read_byte(uint8_t, uint8_t):

//	Descripción:
//		Lee un byte enviado desde el acelerómetro
//	*********************************************************************** //
static inline uint8_t read_byte(uint8_t, uint8_t);
//	*********************************************************************** //
// 4) static inline uint16_t read_word(uint8_t, uint8_t):

//	Descripción:
//		Lee una palabra (conjunto de bytes) enviada desde el acelerómetro
//	*********************************************************************** //
static inline uint16_t read_word(uint8_t, uint8_t);

//variables locales
static uint8_t I2cDeviceAddr = MMA8451_I2C_ADDRESS;
static bool MMA8451Initialized = false;

//	*********************************************************************** //
// 5) void MMA8451SetDeviceAddr( uint8_t addr ):
//	->uint8_t addr: dirección del esclavo a asignar

//	Descripción:
//		configura la dirección del dispositivo esclavo
//	*********************************************************************** //
void MMA8451SetDeviceAddr( uint8_t addr );

//	*********************************************************************** //
// 6) uint8_t MMA8451GetDeviceAddr( void ):

//	Descripción:
//		devuelve la dirección del dispositivo esclavo
//	*********************************************************************** //
uint8_t MMA8451GetDeviceAddr( void );

//	*********************************************************************** //
// 7) uint8_t MMA8451Init( void ):

//	Descripción:
//		inicializa el acelerómetro con la configuración I2C
//	*********************************************************************** //
uint8_t MMA8451Init( void )
{
    uint8_t regVal;

	//asigna dirección del dispositivo deseado
    MMA8451SetDeviceAddr( MMA8451_I2C_ADDRESS );
	//si el dispositivo no esta ya inicializado...
    if( MMA8451Initialized == false )
    {
        MMA8451Initialized = true;
		//comprueba que el dispositivo tiene la dirección asignada
        regVal = read_byte(MMA8451_I2C_ADDRESS, MMA8451_ID);
        if( regVal != 0x1A )
        {
			//dirección incorrecta
            return 1;
        }
        MMA8451Reset( );
        MMA8451OrientDetect( );
    }
    return 0;
}
//	*********************************************************************** //
// 7) uint8_t MMA8451Reset( ):

//	Descripción:
//		resetea el acelerómetro con la configuración I2C
//	*********************************************************************** //
uint8_t MMA8451Reset( )
{
    //MMA8451Write( 0x2B, 0x40 )
    uint8_t word;
    write_byte(MMA8451_I2C_ADDRESS, MMA8451_CTRL_REG2,0x40 & 0xff );//activa reset
    word = read_byte (MMA8451_I2C_ADDRESS, MMA8451_CTRL_REG2);
    if( word == 0 ) // Reset the MMA8451 with CTRL_REG2
    {
        return 0;
    }
    return 1;
}
//	*********************************************************************** //
//	Funciones explicadas en el prototipo
//	*********************************************************************** //
void MMA8451SetDeviceAddr( uint8_t addr )
{
    I2cDeviceAddr = addr;
}
uint8_t MMA8451GetDeviceAddr( void )
{
    return I2cDeviceAddr;
}
//	*********************************************************************** //
// 8) uint8_t MMA8451GetXAxis( void ):

//	Descripción:
//		devuelve el valor del eje X del acelerómetro
//	*********************************************************************** //
uint8_t MMA8451GetXAxis( void )
{
    uint8_t orientation;
    orientation = read_byte(MMA8451_I2C_ADDRESS, MMA8451_OUT_X_MSB);
    return orientation;
}
//	*********************************************************************** //
// 9) uint8_t MMA8451GetYAxis( void ):

//	Descripción:
//		devuelve el valor del eje Y del acelerómetro
//	*********************************************************************** //
uint8_t MMA8451GetYAxis( void )
{
    uint8_t orientation;
    orientation = read_byte(MMA8451_I2C_ADDRESS, MMA8451_OUT_Y_MSB);
    return orientation;
}
//	*********************************************************************** //
// 10) uint8_t MMA8451GetZAxis( void ):

//	Descripción:
//		devuelve el valor del eje Z del acelerómetro
//	*********************************************************************** //
uint8_t MMA8451GetZAxis( void )
{
    uint8_t orientation;
    orientation = read_byte(MMA8451_I2C_ADDRESS, MMA8451_OUT_Z_MSB);
    return orientation;
}
//	*********************************************************************** //
// 11) uint8_t MMA8451Status( void ):

//	Descripción:
//		devuelve el estado en el que se encuentra el acelerómetro
//	*********************************************************************** //
uint8_t MMA8451Status( void )
{
    uint8_t estado;
    estado = read_byte(MMA8451_I2C_ADDRESS, MMA8451_STATUS);
    return estado;
}
//	*********************************************************************** //
// 12) void MMA8451StandBy( void ):

//	Descripción:
//		coloca al acelerómetro en modo StandBy
//	*********************************************************************** //
void MMA8451StandBy( void )
{
    uint8_t ctrl;
    ctrl = read_byte(MMA8451_I2C_ADDRESS, MMA8451_CTRL_REG1);
    uint8_t StandBy =ctrl & ~MMA8451_ACTIVE_MASK;
    write_byte(MMA8451_I2C_ADDRESS, MMA8451_CTRL_REG1, StandBy);
}
//	*********************************************************************** //
// 13) void MMA8451Active ( void ):

//	Descripción:
//		activa al acelerómetro y sale del modo StandBy
//	*********************************************************************** //
void MMA8451Active ( void )
{
    uint8_t ctrl;
    ctrl = read_byte(MMA8451_I2C_ADDRESS, MMA8451_CTRL_REG1);
    uint8_t Active= ctrl | MMA8451_ACTIVE_MASK;
    write_byte(MMA8451_I2C_ADDRESS, MMA8451_CTRL_REG1, Active);
}
//	*********************************************************************** //
// 14) void MMA8451_2g ( void ):

//	Descripción:
//		establece un rango de 2g para el acelerómetro
//	*********************************************************************** //
void MMA8451_2g ( void )
{
    uint8_t ctrl, mode_2g;
    MMA8451StandBy();
    ctrl = read_byte(MMA8451_I2C_ADDRESS, MMA8451_XYZ_DATA_CFG);
    mode_2g = ctrl & 0xff;
    write_byte (MMA8451_I2C_ADDRESS, MMA8451_XYZ_DATA_CFG, mode_2g);
    MMA8451Active();
}
//	*********************************************************************** //
// 15) void MMA8451_4g ( void ):

//	Descripción:
//		establece un rango de 4g para el acelerómetro
//	*********************************************************************** //
void MMA8451_4g ( void )
{
    uint8_t ctrl, mode_2g, mode_4g;
    MMA8451StandBy();
    ctrl = read_byte(MMA8451_I2C_ADDRESS, MMA8451_XYZ_DATA_CFG);
    mode_2g= ctrl & 0xff;
    mode_4g = ctrl | 0x01;
    write_byte (MMA8451_I2C_ADDRESS, MMA8451_XYZ_DATA_CFG, mode_2g);
    write_byte (MMA8451_I2C_ADDRESS, MMA8451_XYZ_DATA_CFG, mode_4g);
    MMA8451Active();
}
//	*********************************************************************** //
// 15) void MMA8451_8g ( void ):

//	Descripción:
//		establece un rango de 8g para el acelerómetro
//	*********************************************************************** //
void MMA8451_8g ( void )
{
    uint8_t ctrl, mode_2g, mode_8g;
    MMA8451StandBy();
    ctrl = read_byte(MMA8451_I2C_ADDRESS, MMA8451_XYZ_DATA_CFG);
    mode_2g = ctrl & 0xff;
    mode_8g = ctrl | 0x02;
    write_byte (MMA8451_I2C_ADDRESS, MMA8451_XYZ_DATA_CFG, mode_2g);
    write_byte (MMA8451_I2C_ADDRESS, MMA8451_XYZ_DATA_CFG, mode_8g);
    MMA8451Active();
}
//	*********************************************************************** //
// 16) void MMA8451DataRate( void ):

//	Descripción:
//		establece la frecuencia de captura de datos para el acelerómetro
//	*********************************************************************** //
void MMA8451DataRate( void )
{
    uint8_t DRValue = MMA8451_DR_6_25_HZ;
    uint8_t ctrl;
    MMA8451StandBy();
    ctrl = read_byte(MMA8451_I2C_ADDRESS, MMA8451_CTRL_REG1);
    write_byte(MMA8451_I2C_ADDRESS, MMA8451_CTRL_REG1, ctrl & 0xff);
    write_byte(MMA8451_I2C_ADDRESS, MMA8451_CTRL_REG1, ctrl | DRValue);
    MMA8451Active();
}
//	*********************************************************************** //
// 17) void MMA8451OverSampling(uint8_t mode):
//	->uint8_t mode: modo de sampleado

//	Descripción:
//		establece el modo de sampleado del acelerómetro
//	*********************************************************************** //
void MMA8451OverSampling(uint8_t mode)
{
    uint8_t ctrl;
    MMA8451StandBy();
    ctrl = read_byte(MMA8451_I2C_ADDRESS, MMA8451_CTRL_REG2);
    write_byte(MMA8451_I2C_ADDRESS, MMA8451_CTRL_REG2, ctrl & 0xff);
    if(mode == MMA8451_OSM_LNLP){
        write_byte(MMA8451_I2C_ADDRESS, MMA8451_CTRL_REG2, ctrl | MMA8451_OSM_LNLP);
        MMA8451Active();
    }else if(mode == MMA8451_OSM_HR){
        write_byte(MMA8451_I2C_ADDRESS, MMA8451_CTRL_REG2, ctrl | MMA8451_OSM_HR);
        MMA8451Active();
    }else if(mode == MMA8451_OSM_LP){
        write_byte(MMA8451_I2C_ADDRESS, MMA8451_CTRL_REG2, ctrl | MMA8451_OSM_LP);
        MMA8451Active();
    }else if(mode == MMA8451_OSM_N){
        MMA8451Active();
    }
}
//	*********************************************************************** //
// 18) void MMA8451FilterCutOff( void ):


//	Descripción:
//		establece un filtro de muestras para el acelerómetro
//	*********************************************************************** //
void MMA8451FilterCutOff( void )
{
    uint8_t FCOValue = MMA8451_FCO_0;
    uint8_t ctrl;
    MMA8451StandBy();
    ctrl = read_byte(MMA8451_I2C_ADDRESS, MMA8451_HP_FILTER_CUTOFF);
    write_byte(MMA8451_I2C_ADDRESS, MMA8451_HP_FILTER_CUTOFF, ctrl & 0xff);
    write_byte(MMA8451_I2C_ADDRESS, MMA8451_HP_FILTER_CUTOFF, ctrl | FCOValue);
    MMA8451Active();
}
//	*********************************************************************** //
// 19) void MMA8451OrientDetect( void ):


//	Descripción:
//		inicializa la detección de orientación
//	*********************************************************************** //
void MMA8451OrientDetect( void )
{
    MMA8451_2g();
    MMA8451DataRate();
    MMA8451OverSampling(MMA8451_OSM_LP);
}


//	*********************************************************************** //
//	Funciones explicadas en el prototipo
//	*********************************************************************** //
static inline void write_byte(uint8_t slave_addr, uint8_t reg, uint8_t byte)
{
    uint8_t buf[2];
    buf[0] = reg ;
    buf[1] = byte;
    i2c_write(slave_addr, buf, sizeof buf, STOP);
    while (i2c_busy())
        ;
}
static inline void write_word(uint8_t slave_addr, uint8_t reg, uint16_t word)
{
    uint8_t buf[3];
    buf[0] = reg;
    buf[1] = word;
    buf[2] = word >> 8;
    i2c_write(slave_addr, buf, sizeof buf, STOP);
    while (i2c_busy())
        ;
}
static inline uint8_t read_byte(uint8_t slave_addr, uint8_t reg)
{
    uint8_t cmd = (reg & 0xf), buf;
    i2c_write(slave_addr, &cmd, sizeof cmd, RSTART);
    while (i2c_busy())
        ;
    i2c_read(slave_addr, &buf, sizeof buf);
    while (i2c_busy())
        ;
    return buf;
}
static inline uint16_t read_word(uint8_t slave_addr, uint8_t reg)
{
    uint8_t cmd = (reg & 0xff);
    uint8_t buf[2];
    i2c_write(slave_addr, &cmd, sizeof cmd, RSTART);
    while (i2c_busy())
        ;
    i2c_read(slave_addr, buf, sizeof buf);
    while (i2c_busy())
        ;
    return (buf[1] << 8) | buf[0];
}
