/**
 * @file ds18b20_stm32f429xx_port.h
 * @brief   Inicializa el driver DS18B20 para la placa NUCLEO-F429ZI
 *
 * Implementa las funciones necesarias para utilizar el driver ds18b20.
 * Inicializa la estructura de datos necesaria para utilizar el sensor.
 *
 */

#ifndef DS18B20_INC_DS18B20_STM32F429XX_PORT_H_
#define DS18B20_INC_DS18B20_STM32F429XX_PORT_H_

#include "gpio.h"
#include "ds18b20.h"

/**
 * @brief	Inicializa el puerto oneWire.
 * @param	puerto que se desea configurar como oneWire.
 * @param   pin que se va a configurar como oneWire.
 * @retval	None
 */
void ds18b20_init_port(GPIO_TypeDef * gpio_settings, uint16_t gpio_pin);

/**
 * @brief	Indica si se encuentra conectado un sensor ds18b20 en el puerto inicializado
 * @retval	true si hay un sensor conectado 
 * @retval  false si no se detecta un sensor conectado.
 *
 */
bool_t ds18b20_is_present_port(void);

/**
 * @brief	Inicia la conversion de temperatura y devuelve el valor de la misma
 * @retval	DS18B20_OK  si se realiza una operacion exitosa.
 * @retval  DS18B20_NACK    si no se recibe ACK del sensor.
 * @retval  DS18B20_CRC_ERROR   si no coincide el valor de CRC leido con el calculado.
 */
ds18b20_status_t ds18b20_get_temperature_port(float* temperature);


#endif /* DS18B20_INC_DS18B20_STM32F429XX_PORT_H_ */
