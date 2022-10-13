/**
 * @file ds18b20.h
 * @brief   Driver para controlar el sensor DS18B20.
 *
 */

#ifndef DS18B20_INC_DS18B20_H_
#define DS18B20_INC_DS18B20_H_

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief   Definicion de tipo de dato booleano
 *
 */
typedef bool bool_t;

/**
 * @brief	Estado del comando ejecutado por el sensor.
 */
typedef enum
{
    DS18B20_OK,         ///< Operacion exitosa
    DS18B20_NACK,       ///< No se recibio ACK del sensor
    DS18B20_CRC_ERROR   ///< Se produjo un error de CRC
} ds18b20_status_t; 

/**
 * @brief	Esta estructura se usa como interfaz para poder portar el driver a distintas arquitecturas
 */
typedef struct
{
    // Punteros a funciones
	void	(*oneWireBus_pulldown) (void*); ///< Setea el puerto como salida en estado bajo
	bool_t 	(*oneWireBus_read) (void*);     ///< Lee el estado del puerto
	void 	(*oneWireBus_release) (void*);  ///< Setea el puerto como entrada para liberar el bus
	void 	(*delay_us) (uint16_t);		    ///< Retardo en microsegundos

    //
	void* oneWireBus; 						///< Puntero a la estructura de datos correspondiente para manejar gpios
    float temperature;					    ///< Ultimo valor de temperatura leido
} oneWire_t;

/**
 * @brief	Realiza la conversion y lectura de temperatura del sensor oneWire.
 * @param	oneWireBus  puerto donde se conecta el sensor oneWire.
 * @retval	DS18B20_OK  si se realiza una operacion exitosa.
 * @retval  DS18B20_NACK    si no se recibe ACK del sensor.
 * @retval  DS18B20_CRC_ERROR   si no coincide el valor de CRC leido con el calculado.
 */
ds18b20_status_t ds18b20_get_temperature(oneWire_t* oneWireBus);

/**
 * @brief	Checkea si se encuentra conectado un sensor en el puerto oneWire.
 * @param	oneWireBus puerto donde se conecta el sensor oneWire.
 * @retval	true si se detecta un sensor.
 * @retval  false si no se detecta un sensor.
 */
bool_t ds18b20_is_present(oneWire_t* oneWireBus);

#endif /* DS18B20_INC_DS18B20_H_ */
