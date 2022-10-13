/*
 * ds18b20.c
 *
 *  Created on: Sep 29, 2022
 *      Author: cfunes
 */

#include "ds18b20.h"

/* Private definitions -------------------------------*/
typedef enum
{
	SEARCH_ROM = 0,
	READ_ROM,
    MATCH_ROM,
    SKIP_ROM,
    ALARM_SEARCH,
    CONVERT_T,
    WRITE_SCRATCHPAD,
    READ_SCRATCHPAD,
    COPY_SCRATCHPAD,
    RECALL_EE,
    READ_POWER_SUPPLY,
    COMMAND_LAST
} oneWire_command_t;

static uint8_t commands[COMMAND_LAST] = { 	0xF0, /* SEARCH_ROM 		*/
								            0x33, /* READ_ROM 			*/
								            0x55, /* MATCH_ROM 			*/
								            0xCC, /* SKIP_ROM 			*/
								            0xEC, /* ALARM_SEARCH		*/
								            0x44, /* CONVERT_T			*/
								            0x4E, /* WRITE_SCRATCHPAD	*/
								            0xBE, /* READ_SCRATCHPAD	*/
								            0x48, /* COPY_SCRATCHPAD	*/
								            0xB8, /* RECALL_EE			*/
								            0xB4, /* READ_POWER_SUPPLY	*/
							            };

typedef enum
{
	DS_TEMPERATURE_LSB=0,
	DS_TEMPERATURE_MSB,
	DS_TH_REG_USER,
	DS_TL_REG_USER,
	DS_CONFIGURATION,
	DS_RESERVED_0,
	DS_RESERVED_1,
	DS_RESERVED_2,
	DS_CRC_REG,
	DS_SCRATCHPAD_SIZE
} scratchpad_mem_t;

/*
 * @brief	Ejecuta un comando de reset.
 * @param	port es el puerto donde se conecta el sensor oneWire.
 * @retval  true si se detecta un sensor conectado y false de lo contrario.
 */
static bool_t one_wire_reset(oneWire_t* oneWireBus)
{
	bool_t ack = false;

	oneWireBus->oneWireBus_pulldown(oneWireBus->oneWireBus);
	oneWireBus->delay_us(480);
	oneWireBus->oneWireBus_release(oneWireBus->oneWireBus);
	oneWireBus->delay_us(210);

	ack = oneWireBus->oneWireBus_read(oneWireBus->oneWireBus);

	if (ack)
	{
		oneWireBus->delay_us(500);
	}

	return ack;
}

/*
 * @brief	Escribe el bit menos significativo del byte que se desea.
 * @param	port es el puerto donde se conecta el sensor oneWire.
 * @param 	data es el dato cuyo bit menos significativo se desea enviar.
 * @retval 	None
 */
static void write_lsb(oneWire_t* oneWireBus, uint8_t data)
{
	if ( data & 0x1)
	{
		oneWireBus->oneWireBus_pulldown(oneWireBus->oneWireBus);
		oneWireBus->delay_us(6);
		oneWireBus->oneWireBus_release(oneWireBus->oneWireBus);
		oneWireBus->delay_us(64);
	}
    else
	{
		oneWireBus->oneWireBus_pulldown(oneWireBus->oneWireBus);
		oneWireBus->delay_us(60);
		oneWireBus->oneWireBus_release(oneWireBus->oneWireBus);
		oneWireBus->delay_us(10);
	}
}

/*
 * @brief	Lee un bit por el puerto oneWire
 * @param	port es el puerto donde se conecta el sensor oneWire.
 * @retval	bit leido en el puerto oneWire
 */
static uint8_t read_lsb(oneWire_t* oneWireBus)
{
    uint8_t lsb = 0;

    oneWireBus->oneWireBus_pulldown(oneWireBus->oneWireBus);
    oneWireBus->delay_us(6);
    oneWireBus->oneWireBus_release(oneWireBus->oneWireBus);
    oneWireBus->delay_us(9);
    
    lsb = oneWireBus->oneWireBus_read(oneWireBus->oneWireBus);

    oneWireBus->delay_us(55);

    return lsb;
}

/*
 * @brief	Escribe un byte completo por el puerto oneWire
 * @param	port es el puerto donde se conecta el sensor oneWire.
 * @para 	data es el byte que se desea enviar
 * @retval 	None
 */
static void write_byte(oneWire_t* oneWireBus, uint8_t data)
{
    for (uint8_t i=0; i<8; ++i)
    {
        write_lsb(oneWireBus, data>>i);
    }
}

/*
 * @brief	Lee un byte completo por el puerto oneWire.
 * @param	port es el puerto donde se conecta el sensor oneWire.
 * @retval	byte leido por el puerto oneWire.
 */
static uint8_t read_byte(oneWire_t* oneWireBus)
{
    uint8_t rbyte = 0x00;

    for (uint8_t i=0; i<8; ++i)
    {
        rbyte >>= 1;
        if (read_lsb(oneWireBus)) rbyte |= 0x80;
    }

    return rbyte;
}

/*
 * @brief	Envia un comando a traves del puerto oneWire.
 * @param	port es el puerto donde se conecta el sensor oneWire.
 * @param	command es el comando que se desea enviar.
 * @retval	None
 */
static void send_command(oneWire_t* oneWireBus, oneWire_command_t command)
{
    write_byte(oneWireBus, commands[command]);
}

/*
 * @brief	Calcula el crc de los datos leidos de la memoria del sensor.
 * @param	addr es un puntero a los valores de memoria leidos del sensor.
 * @param	len es el numero de elementos de la memoria del sensor.
 * @retval	retorna el valor calculado de crc.
 */
static uint8_t dsCRC8(const uint8_t *addr, uint8_t len)
{
    uint8_t crc = 0;
    while (len--)
    {
        uint8_t inbyte = *addr++;
        for (uint8_t i = 8; i; i--)
        {
            uint8_t mix = (crc ^ inbyte) & 0x01;
            crc >>= 1;
            if (mix) crc ^= 0x8C;
            inbyte >>= 1;
        }
    }
    return crc;
}

/* Public functions ----------------------------------------*/

/*
 * @brief	Checkea si se encuentra conectado un sensor en el puerto oneWire.
 * @param	port es el puerto donde se conecta el sensor oneWire.
 * @retval	true si se encuentra un sensor y false de lo contrario.
 */
bool_t ds18b20_is_present(oneWire_t* oneWireBus)
{
    return one_wire_reset(oneWireBus);
}

/*
 * @brief	Realiza la conversion y lectura de temperatura del sensor oneWire.
 * @param	port es el puerto donde se conecta el sensor oneWire.
 * @retval	DS18B20_OK en caso de que se realice una operacion exitosa y ERROR en caso contrario.
 */
ds18b20_status_t ds18b20_get_temperature(oneWire_t* oneWireBus)
{
	// La secuencia de comandos para trabajar con el sensor ds18b20 es la siguiente:
	//		1- Inicializacion
	//		2- Enviar ROM Command.
	//		3- Enviar Function Command.
	//
	// Para realizar la conversion y posterior lectura de temperatura se debe:
	//		1- Ejecutar reset del bus.
	//		2- Enviar comando SKIP_ROM. Este comando se usa cuando hay un solo sensor conectado al bus.
	//		3- Enviar comando de conversion de temperatura.
	//		4- Volver a ejecutar reset.
	//		5- Volver a enviar SKIP_ROM.
	//		6- Leer la memoria del sensor (scratchpad).

    ds18b20_status_t status = DS18B20_NACK;
    uint8_t scratchpad_mem[DS_SCRATCHPAD_SIZE];

    if (one_wire_reset(oneWireBus))
    {
        send_command(oneWireBus, SKIP_ROM);
        send_command(oneWireBus, CONVERT_T);
        while(!oneWireBus->oneWireBus_read(oneWireBus));
        if (one_wire_reset(oneWireBus))
        {
            send_command(oneWireBus, SKIP_ROM);
            send_command(oneWireBus, READ_SCRATCHPAD);
            // Luego de enviar el comando read scratchpad, se pueden leer los 8bytes
            // del scratchpad + 1byte de CRC.
            for(uint8_t i=0; i<9; ++i) scratchpad_mem[i] = read_byte(oneWireBus);
            uint8_t sign = scratchpad_mem[DS_TEMPERATURE_MSB]&0x80;
            uint16_t LSB = scratchpad_mem[DS_TEMPERATURE_LSB];
            uint16_t MSB = scratchpad_mem[DS_TEMPERATURE_MSB];
            one_wire_reset(oneWireBus);

            if (dsCRC8(scratchpad_mem, 8) == scratchpad_mem[DS_CRC_REG])
            {
            	int16_t temp = (MSB<<8) | LSB;
            	if (sign)
            	{
            		temp = ((temp^0xffff) +1)*-1;
            	}
            	oneWireBus->temperature = temp/16.0;
            	status = DS18B20_OK;
            }
            else
            {
            	status = DS18B20_CRC_ERROR;
            }
        }
    }

    return status;
}

