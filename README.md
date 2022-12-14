# CESE Protocolos de comunicacion - Trabajo Final
## Implementacion de un driver generico para un sensor de temperatura DS18B20
### Introduccion
Se implemento un driver generico para poder realizar la lectura de temperatura de un sensor DS18B20. Se porto el driver para ser utilizado en un STM32F429xx. El driver permite utilizar cualquier gpio como puerto oneWire para comunicarse con el sensor de temperatura.
### Implementacion
El driver es del tipo `Polled driver` y se desarrollo siguiendo la siguiente estructura de archivos:
```
maxim_ds18b20/
|__ Inc
|     |__ ds18b20.h
|     |__ ds18b20_stm32f429xx_port.h
|__ Src
      |__ ds18b20.c
      |__ ds18b20_stm32f429xx_port.c
```
### Ejemplo con STM32F429
```c
#include "ds18b20_stm32f429xx_port.h"

#define DS18B20_Port_Pin GPIO_PIN_0
#define DS18B20_Port_GPIO_Port GPIOC

/* Inicializacion de los gpios como puerto oneWire */
ds18b20_init_port(DS18B20_Port_GPIO_Port, DS18B20_Port_Pin);

float current_temperature;

/* Si la operacion de lectura de temperatura fue exitosa */
if (ds18b20_get_temperature_port(&current_temperature) == DS18B20_OK) printf("%f", current_temperature);
/* Si hubo algun error */
else printf("Error")
```
### Ejemplo con gcc
```c
#include "ds18b20.h"
#include <stdio.h>

void pulldown(void* bus)
{
    printf("..");
}

void release(void* bus)
{
    printf("||");
}

bool_t read(void* bus)
{
    return true;
}

void delay(uint16_t delay)
{
    //do nothing
}

int main(void)
{
    oneWire_t oneWire;
    oneWire.oneWireBus_pulldown = pulldown;
    oneWire.oneWireBus_release = release;
    oneWire.oneWireBus_read = read;
    oneWire.delay_us = delay;
    oneWire.oneWireBus = 0;


    ds18b20_is_present(&oneWire);
    ds18b20_get_temperature(&oneWire);

    return 0;
}
```
