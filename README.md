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
