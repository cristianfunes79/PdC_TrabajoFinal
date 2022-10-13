/*
 * ds18b20_stm32f429xx_port
 *
 *  Created on: Oct 11, 2022
 *      Author: cfunes
 */
#include "ds18b20_stm32f429xx_port.h"
#include "stm32f4xx_hal.h"

/* Private definitions ------------------------*/
static TIM_HandleTypeDef htim7;

/* @struct
 * @brief	Esta estructura se define para poder configurar el puerto oneWire.
 *
 *  Como se requiere cambiar la configuracion del gpio durante la lectura del sensor
 *  de temperatura, es necesario pasar tanto el pin como el seteo del gpio a las
 *  funciones de manejo del puerto.
 */
typedef struct
{
	GPIO_TypeDef* settings;
	uint16_t pin;
}GenericGPIO_t;

/**
 * @brief   Puerto oneWire
 */
static GenericGPIO_t oneWirePort;

/**
 * @brief  Estructura de datos para controlar el sensor ds18b20. 
 */
static oneWire_t oneWireSensor;

/* Private functions ----------------------------------------------*/

/*
 * @brief	Seteo del gpio como salida en estado bajo.
 * @param	gpio es un puntero a la estructura de manejo de gpios.
 * @retval	None
 */
static void set_gpio_low(void* gpio)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	HAL_GPIO_WritePin(((GenericGPIO_t*)gpio)->settings, ((GenericGPIO_t*)gpio)->pin, GPIO_PIN_RESET);

	/* Configure GPIO pin */
	GPIO_InitStruct.Pin = ((GenericGPIO_t*)gpio)->pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(((GenericGPIO_t*)gpio)->settings, &GPIO_InitStruct);
}

/*
 * @brief	Seteo del gpio como entrada. Se usa para poder hacer un release del puerto oneWire
 * 			y tambien para luego poder leer los valores seteados por el sensor.
 * @param	gpio es un puntero a la estructura de manejo de gpios.
 * @retval 	None
 */
static void set_gpio_input(void* gpio)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	/* Configure GPIO pin */
	GPIO_InitStruct.Pin = ((GenericGPIO_t*)gpio)->pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(((GenericGPIO_t*)gpio)->settings, &GPIO_InitStruct);
}

/*
 * @brief	Devuelve el estado de la entrada seleccionada.
 * @param	gpio es un puntero a la estructura de manejo de gpios.
 * @retval	true si el gpio esta en alto y false si esta en estado bajo.
 */
static bool read_gpio(void* gpio)
{
	return (HAL_GPIO_ReadPin(((GenericGPIO_t*)gpio)->settings,((GenericGPIO_t*)gpio)->pin) == GPIO_PIN_SET) ? true : false;
}

/* TIM7 init function */
static void MX_TIM7_Init(void)
{

  /* USER CODE BEGIN TIM7_Init 0 */

  /* USER CODE END TIM7_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM7_Init 1 */

  /* USER CODE END TIM7_Init 1 */
  htim7.Instance = TIM7;
  htim7.Init.Prescaler = 84;
  htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim7.Init.Period = 65535;
  htim7.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim7) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim7, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM7_Init 2 */
  HAL_TIM_Base_Start(&htim7);
  /* USER CODE END TIM7_Init 2 */

}


/*
 * @brief	Inicializa el timer por hardware que sera utilizado como
 * 			base de tiempo para el delay.
 * @retval	None
 */
static void delay_us_init(void)
{
	if (htim7.Instance == NULL)
	{
		MX_TIM7_Init();
	}
}

/*
 * @brief	Realiza una espera bloqueante en microsegundos.
 * @param 	delay es el retardo en microsegundos.
 * @retval	None
 */
static void delay_us(uint16_t delay)
{
	  __HAL_TIM_SET_COUNTER(&htim7, 0);
	  while (__HAL_TIM_GET_COUNTER(&htim7) < delay);
}

/* Public functions ---------------------------------------------------*/

/*
 * @brief	Inicializa el puerto oneWire.
 * @param	gpio_settings y gpio_pin indican el puerto que se desea configurar.
 * @retval	None
 */
void ds18b20_init_port(GPIO_TypeDef * gpio_settings, uint16_t gpio_pin)
{
	oneWirePort.settings = gpio_settings;
	oneWirePort.pin = gpio_pin;

	oneWireSensor.oneWireBus_pulldown = set_gpio_low;
	oneWireSensor.oneWireBus_read = read_gpio;
	oneWireSensor.oneWireBus_release = set_gpio_input;
	oneWireSensor.delay_us = delay_us;
	oneWireSensor.oneWireBus = &oneWirePort;

	delay_us_init();
}

/*
 * @brief	Indica si se encuentra conectado un sensor ds18b20 en el puerto inicializado
 * @retval	true si hay un sensor conectado y false en caso contrario.
 *
 */
bool ds18b20_is_present_port(void)
{
	return ds18b20_is_present(&oneWireSensor);
}

/*
 * @brief	Inicializa la conversion de temperatura y devuelve el valor de la misma/
 * @retval	OK en caso de que la operacion sea exitosa y TEMP_ERROR de lo contrario
 */
ds18b20_status_t ds18b20_get_temperature_port(float* temperature)
{
	ds18b20_status_t status = ds18b20_get_temperature(&oneWireSensor);

	if (status == DS18B20_OK)
	{
		*temperature = oneWireSensor.temperature;
		status = DS18B20_OK;
	}

	return status;
}
