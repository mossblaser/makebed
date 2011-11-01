#ifndef GPIO_H
#define GPIO_H

/* Standard includes. */
#include "stdio.h"

/* FreeRTOS stuff */
#include "FreeRTOS.h"
#include "semphr.h"

/* LPC Register Definitions */
#include "LPC17xx.h"

#define PCONP_PCGPIO 0x00008000

/**
 * Pin modes for a GPIO pin
 */
typedef enum gpio_mode {
	GPIO_INPUT  = 0,
	GPIO_OUTPUT = 1
} gpio_mode_t;

/**
 * Pin values for a GPIO pin
 */
typedef enum gpio_value {
	GPIO_LOW  = 0,
	GPIO_HIGH = 1
} gpio_value_t;

/**
 * A port on the device (not intended for external use)
 */
typedef struct gpio_port {
	LPC_GPIO_TypeDef *lpc_port;
	xSemaphoreHandle  mutex;
} gpio_port_t;


/**
 * A GPIO pin refrence.
 */
typedef struct gpio {
	gpio_port_t  *port;
	unsigned int  pin;
} gpio_t;


/* GPIOs available on the mbed (setup by gpio_init()) */
extern gpio_t gpio_mbed_p5;
extern gpio_t gpio_mbed_p6;
extern gpio_t gpio_mbed_p7;
extern gpio_t gpio_mbed_p8;
extern gpio_t gpio_mbed_p9;
extern gpio_t gpio_mbed_p10;
extern gpio_t gpio_mbed_p11;
extern gpio_t gpio_mbed_p12;
extern gpio_t gpio_mbed_p13;
extern gpio_t gpio_mbed_p14;
extern gpio_t gpio_mbed_p15;
extern gpio_t gpio_mbed_p16;
extern gpio_t gpio_mbed_p17;
extern gpio_t gpio_mbed_p18;
extern gpio_t gpio_mbed_p19;
extern gpio_t gpio_mbed_p20;
extern gpio_t gpio_mbed_p21;
extern gpio_t gpio_mbed_p22;
extern gpio_t gpio_mbed_p23;
extern gpio_t gpio_mbed_p24;
extern gpio_t gpio_mbed_p25;
extern gpio_t gpio_mbed_p26;
extern gpio_t gpio_mbed_p27;
extern gpio_t gpio_mbed_p28;
extern gpio_t gpio_mbed_p29;
extern gpio_t gpio_mbed_p30;

/* LEDs on the mbed (setup by gpio_init()) */
extern gpio_t gpio_mbed_led1;
extern gpio_t gpio_mbed_led2;
extern gpio_t gpio_mbed_led3;
extern gpio_t gpio_mbed_led4;


/**
 * Initialise the GPIO system. Call before using any functions in this file
 */
void gpio_init(void);

/**
 * Initialise a gpio_port_t with the given port register address.
 */
gpio_port_t gpio_init_port_t(LPC_GPIO_TypeDef *lpc_port);

/**
 * Initialise a gpio_t with the given port and pin number
 */
gpio_t gpio_init_gpio_t(gpio_port_t *port, unsigned int pin);

/**
 * Enable pin as GPIO and set the pin mode as specified.
 */
void gpio_set_mode(gpio_t *gpio, gpio_mode_t mode);

/**
 * Disable the pin leaving it in its default state.
 */
void gpio_disable(gpio_t *gpio);

/**
 * Read the value from the specified GPIO pin
 */
gpio_value_t gpio_read (gpio_t *gpio);

/**
 * Write the value to the specified GPIO pin
 */
void gpio_write(gpio_t *gpio, gpio_value_t value);

#endif
