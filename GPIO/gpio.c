#include "gpio.h"

/* USB Serial Support includes. */
#include "usb_cdc.h"


/* GPIO Ports, private */
gpio_port_t gpio_port_0;
gpio_port_t gpio_port_1;
gpio_port_t gpio_port_2;
gpio_port_t gpio_port_3;
gpio_port_t gpio_port_4;

/* GPIO Pins, public */
gpio_t gpio_mbed_p5;
gpio_t gpio_mbed_p6;
gpio_t gpio_mbed_p7;
gpio_t gpio_mbed_p8;
gpio_t gpio_mbed_p9;
gpio_t gpio_mbed_p10;
gpio_t gpio_mbed_p11;
gpio_t gpio_mbed_p12;
gpio_t gpio_mbed_p13;
gpio_t gpio_mbed_p14;
gpio_t gpio_mbed_p15;
gpio_t gpio_mbed_p16;
gpio_t gpio_mbed_p17;
gpio_t gpio_mbed_p18;
gpio_t gpio_mbed_p19;
gpio_t gpio_mbed_p20;
gpio_t gpio_mbed_p21;
gpio_t gpio_mbed_p22;
gpio_t gpio_mbed_p23;
gpio_t gpio_mbed_p24;
gpio_t gpio_mbed_p25;
gpio_t gpio_mbed_p26;
gpio_t gpio_mbed_p27;
gpio_t gpio_mbed_p28;
gpio_t gpio_mbed_p29;
gpio_t gpio_mbed_p30;

/* LEDs, public */
gpio_t gpio_mbed_led1;
gpio_t gpio_mbed_led2;
gpio_t gpio_mbed_led3;
gpio_t gpio_mbed_led4;

void
gpio_init(void)
{
	// Enable GPIO power
	LPC_SC->PCONP |= PCONP_PCGPIO;
	
	// Initialise ports
	gpio_port_0 = gpio_init_port_t(LPC_GPIO0);
	gpio_port_1 = gpio_init_port_t(LPC_GPIO1);
	gpio_port_2 = gpio_init_port_t(LPC_GPIO2);
	gpio_port_3 = gpio_init_port_t(LPC_GPIO3);
	gpio_port_4 = gpio_init_port_t(LPC_GPIO4);
	
	// Initialise mbed pins
	gpio_mbed_p5  = gpio_init_gpio_t(&gpio_port_0,  9);
	gpio_mbed_p6  = gpio_init_gpio_t(&gpio_port_0,  8);
	gpio_mbed_p7  = gpio_init_gpio_t(&gpio_port_0,  7);
	gpio_mbed_p8  = gpio_init_gpio_t(&gpio_port_0,  6);
	gpio_mbed_p9  = gpio_init_gpio_t(&gpio_port_0,  0);
	gpio_mbed_p10 = gpio_init_gpio_t(&gpio_port_0,  1);
	gpio_mbed_p11 = gpio_init_gpio_t(&gpio_port_0, 18);
	gpio_mbed_p12 = gpio_init_gpio_t(&gpio_port_0, 17);
	gpio_mbed_p13 = gpio_init_gpio_t(&gpio_port_0, 15);
	gpio_mbed_p14 = gpio_init_gpio_t(&gpio_port_0, 16);
	gpio_mbed_p15 = gpio_init_gpio_t(&gpio_port_0, 23);
	gpio_mbed_p16 = gpio_init_gpio_t(&gpio_port_0, 24);
	gpio_mbed_p17 = gpio_init_gpio_t(&gpio_port_0, 25);
	gpio_mbed_p18 = gpio_init_gpio_t(&gpio_port_0, 26);
	gpio_mbed_p19 = gpio_init_gpio_t(&gpio_port_1, 30);
	gpio_mbed_p20 = gpio_init_gpio_t(&gpio_port_1, 31);
	gpio_mbed_p21 = gpio_init_gpio_t(&gpio_port_2,  5);
	gpio_mbed_p22 = gpio_init_gpio_t(&gpio_port_2,  4);
	gpio_mbed_p23 = gpio_init_gpio_t(&gpio_port_2,  3);
	gpio_mbed_p24 = gpio_init_gpio_t(&gpio_port_2,  2);
	gpio_mbed_p25 = gpio_init_gpio_t(&gpio_port_2,  1);
	gpio_mbed_p26 = gpio_init_gpio_t(&gpio_port_2,  0);
	gpio_mbed_p27 = gpio_init_gpio_t(&gpio_port_0, 11);
	gpio_mbed_p28 = gpio_init_gpio_t(&gpio_port_0, 10);
	gpio_mbed_p29 = gpio_init_gpio_t(&gpio_port_0,  5);
	gpio_mbed_p30 = gpio_init_gpio_t(&gpio_port_0,  4);
	
	// Initialise LEDs
	gpio_mbed_led1 = gpio_init_gpio_t(&gpio_port_1, 18);
	gpio_mbed_led2 = gpio_init_gpio_t(&gpio_port_1, 20);
	gpio_mbed_led3 = gpio_init_gpio_t(&gpio_port_1, 21);
	gpio_mbed_led4 = gpio_init_gpio_t(&gpio_port_1, 23);
	
	// Set LEDs to outputs
	gpio_set_mode(&gpio_mbed_led1, GPIO_OUTPUT);
	gpio_set_mode(&gpio_mbed_led2, GPIO_OUTPUT);
	gpio_set_mode(&gpio_mbed_led3, GPIO_OUTPUT);
	gpio_set_mode(&gpio_mbed_led4, GPIO_OUTPUT);
}


gpio_port_t
gpio_init_port_t(LPC_GPIO_TypeDef *lpc_port)
{
	gpio_port_t port;
	
	port.lpc_port = lpc_port;
	port.mutex    = xSemaphoreCreateMutex();
	
	return port;
}


gpio_t
gpio_init_gpio_t(gpio_port_t *port, unsigned int pin)
{
	gpio_t gpio;
	
	gpio.port = port;
	gpio.pin  = pin;
	
	return gpio;
}


void
gpio_set_mode(gpio_t *gpio, gpio_mode_t mode)
{
	// Get the port mutex (block forever until it is available)
	xSemaphoreTake(gpio->port->mutex, portMAX_DELAY);
	
	// Clear the IO mask bit (enabling control of the pin)
	gpio->port->lpc_port->FIOMASK & ~((uint32_t)(1<<gpio->pin));
	
	// Get the current mode register for this port and clear the target pin bit
	uint32_t cur_mode = gpio->port->lpc_port->FIODIR & ~((uint32_t)(1<<gpio->pin));
	
	// Set the register with the mode bit set as requested
	gpio->port->lpc_port->FIODIR = cur_mode | ((uint32_t)mode << (gpio->pin));
	
	// Release the mutex
	xSemaphoreGive(gpio->port->mutex);
} // gpio_set_mode


void
gpio_disable(gpio_t *gpio)
{
	// Just set the pin back to input mode again
	gpio_set_mode(gpio, GPIO_INPUT);
}


gpio_value_t
gpio_read(gpio_t *gpio)
{
	// Get the port mutex (block forever until it is available)
	xSemaphoreTake(gpio->port->mutex, portMAX_DELAY);
	
	// Fetch the pin value
	gpio_value_t value = (gpio->port->lpc_port->FIOPIN >> gpio->pin) & 1;
	
	// Release the mutex
	xSemaphoreGive(gpio->port->mutex);
	
	return value;
}


void
gpio_write(gpio_t *gpio, gpio_value_t value)
{
	// Get the port mutex (block forever until it is available)
	xSemaphoreTake(gpio->port->mutex, portMAX_DELAY);
	
	// Set the pin value
	if (value == GPIO_HIGH)
		gpio->port->lpc_port->FIOSET = 1 << (gpio->pin);
	else
		gpio->port->lpc_port->FIOCLR = 1 << (gpio->pin);
	
	
	// XXX: Debugging
	VCOM_putstr("gpio_write:\r\n");
	
	VCOM_putstr("  port = 0x");
	VCOM_putuint_base((unsigned int) &(gpio->port->lpc_port), 16);
	VCOM_putstr("\r\n");
	
	VCOM_putstr("  pin = ");
	VCOM_putuint((unsigned int) gpio->pin);
	VCOM_putstr("\r\n");
	
	VCOM_putstr("  value = ");
	VCOM_putuint((unsigned int) value);
	VCOM_putstr("\r\n");
	
	VCOM_putstr("  FIOPIN = ");
	VCOM_putuint_base((unsigned int) gpio->port->lpc_port->FIOPIN, 2);
	VCOM_putstr("\r\n");
	VCOM_putstr("           ........^.^^.^..................");
	VCOM_putstr("\r\n");
	
	VCOM_putstr("  FIOMASK = ");
	VCOM_putuint_base((unsigned int) gpio->port->lpc_port->FIOMASK, 2);
	VCOM_putstr("\r\n");
	VCOM_putstr("            ........^.^^.^..................");
	VCOM_putstr("\r\n");
	
	VCOM_putstr("  FIODIR = ");
	VCOM_putuint_base((unsigned int) gpio->port->lpc_port->FIODIR, 2);
	VCOM_putstr("\r\n");
	VCOM_putstr("           ........^.^^.^..................");
	VCOM_putstr("\r\n");
	
	VCOM_putstr("  Mask = ");
	VCOM_putuint_base((unsigned int) (1u << (gpio->pin)), 2);
	VCOM_putstr("\r\n");
	VCOM_putstr("         ........^.^^.^..................");
	VCOM_putstr("\r\n");
	
	VCOM_putstr("  PINSEL3 = ");
	VCOM_putuint_base((unsigned int) LPC_PINCON->PINSEL3, 2);
	VCOM_putstr("\r\n");
	VCOM_putstr("         ..........................^^....");
	VCOM_putstr("\r\n");
	
	// Release the mutex
	xSemaphoreGive(gpio->port->mutex);
}
