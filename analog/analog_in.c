#include "analog_in.h"

/* A mutex which must be held to use the ADC */
xSemaphoreHandle analog_in_adc_mutex;

/* Device Ports, private */
analog_in_port_t analog_in_port_0l;
analog_in_port_t analog_in_port_0h;
analog_in_port_t analog_in_port_1l;
analog_in_port_t analog_in_port_1h;

/* Analog-In Pins, public */
analog_in_t analog_in_mbed_p15;
analog_in_t analog_in_mbed_p16;
analog_in_t analog_in_mbed_p17;
analog_in_t analog_in_mbed_p18;
analog_in_t analog_in_mbed_p19;
analog_in_t analog_in_mbed_p20;


void
analog_in_init(void)
{
	// Power up the ADC
	LPC_SC->PCONP |= PCONP_PCADC;
	
	// Enable the ADC
	LPC_ADC->ADCR |= ADCR_PDN;
	
	// Set up the input clock divider for the ADC
	LPC_SC->PCLKSEL0 = (LPC_SC->PCLKSEL0 & ~(0x3<<24)) | (0x1<<24);
	
	// Set up the clock divider for the ADC
	LPC_ADC->ADCR |= ADCR_CLKDIV;
	
	// Initialise ports
	analog_in_port_0l = analog_in_init_port_t(&(LPC_PINCON->PINSEL0), 0x1);
	analog_in_port_0h = analog_in_init_port_t(&(LPC_PINCON->PINSEL1), 0x1);
	analog_in_port_1l = analog_in_init_port_t(&(LPC_PINCON->PINSEL2), 0x3);
	analog_in_port_1h = analog_in_init_port_t(&(LPC_PINCON->PINSEL3), 0x3);
	
	// Initialise mbed pins
	analog_in_mbed_p15 = analog_in_init_analog_in_t(&analog_in_port_0h, 23, 0);
	analog_in_mbed_p16 = analog_in_init_analog_in_t(&analog_in_port_0h, 24, 1);
	analog_in_mbed_p17 = analog_in_init_analog_in_t(&analog_in_port_0h, 25, 2);
	analog_in_mbed_p18 = analog_in_init_analog_in_t(&analog_in_port_0h, 26, 3);
	analog_in_mbed_p19 = analog_in_init_analog_in_t(&analog_in_port_1h, 30, 4);
	analog_in_mbed_p20 = analog_in_init_analog_in_t(&analog_in_port_1h, 31, 5);
	
	// Initialise mutex
	analog_in_adc_mutex = xSemaphoreCreateMutex();
}


analog_in_port_t
analog_in_init_port_t(__IO uint32_t *pinsel, uint32_t mask)
{
	analog_in_port_t port;
	
	port.pinsel = pinsel;
	port.mask   = mask;
	
	return port;
}


analog_in_t
analog_in_init_analog_in_t(analog_in_port_t *port,
                           unsigned int      pin,
                           unsigned int      analog_ch)
{
	analog_in_t analog_in;
	
	analog_in.port      = port;
	analog_in.pin       = pin;
	analog_in.analog_ch = analog_ch;
	
	return analog_in;
}


void
analog_in_set_mode(analog_in_t *analog_in)
{
	portENTER_CRITICAL();
	
	*(analog_in->port->pinsel) |= (analog_in->port->mask)
	                              << ((analog_in->pin * 2ul) % 32ul);
	
	portEXIT_CRITICAL();
}


int
analog_in_read(analog_in_t *analog_in)
{
	xSemaphoreTake(analog_in_adc_mutex, portMAX_DELAY);
	
	// Load the control register
	uint32_t adcr = LPC_ADC->ADCR;
	
	// Clear the input selection field (bottom 8 bits)
	adcr &= ~0xFFul;
	
	// Select the ADC input to read
	adcr |= 1 << analog_in->analog_ch;
	
	// Put the ADC in "start" mode
	adcr |= 1ul<<24;
	
	// Write back to register
	LPC_ADC->ADCR = adcr;
	
	// Wait for the read to complete...
	portTickType last_tick = xTaskGetTickCount();
	uint32_t result;
	do {
		vTaskDelayUntil(&last_tick, ADC_READ_DELAY);
		result = LPC_ADC->ADGDR;
	} while ((result & (1<<31)) == 0);
	
	xSemaphoreGive(analog_in_adc_mutex);
	
	return (result >> 4) & ADC_MAX;
}
