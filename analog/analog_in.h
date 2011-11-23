#ifndef ANALOG_IN_H
#define ANALOG_IN_H

/* Standard includes. */
#include "stdio.h"
#include "type.h"

/* FreeRTOS stuff */
#include "FreeRTOS.h"
#include "semphr.h"

/* LPC Register Definitions */
#include "LPC17xx.h"


/* Frequency of analog reads. */
#define ADC_READ_HZ 200000ul

/* Target/Max clock speed of ADC (13MHz) */
#define ADC_CLOCK_TARGET_HZ 13000000ul

/* Maximum value of an ADC reading */
#define ADC_MAX 0xFFF

/* Input clock speed of input to the ADC */
#define ADC_INPUT_CLOCK_SPEED_HZ (configCPU_CLOCK_HZ / 4ul)

/* Required devider setting shifted into the right area of the register. */
#define ADCR_CLKDIV ((uint8_t)(((ADC_INPUT_CLOCK_SPEED_HZ + ADC_CLOCK_TARGET_HZ - 1ul) \
                                / ADC_CLOCK_TARGET_HZ) \
                               - 1ul) << 8)

/* Delay before reading result register in scheduler ticks. */
#define ADC_READ_DELAY ((portTickType)((configTICK_RATE_HZ + ADC_READ_HZ - 1) \
                                       / ADC_READ_HZ))


/* Config Register Bit Masks */
#define PCONP_PCADC (1<<12)
#define ADCR_PDN (1<<21)


/**
 * A port on the device (not intended for external use) contianing the
 * pin-mode-select register and the mask to apply to that register to connect
 * the ADC.
 */
typedef struct analog_in_port {
	__IO uint32_t *pinsel;
	uint32_t       mask;
} analog_in_port_t;


/**
 * An Analog-In pin refrence.
 */
typedef struct analog_in {
	analog_in_port_t  *port;
	unsigned int       pin;
	unsigned int       analog_ch;
} analog_in_t;


/* A mutex which must be held to use the ADC */
extern xSemaphoreHandle analog_in_adc_mutex;


/* Analog In Pins available on the mbed (setup by analog_in_init()) */
extern analog_in_t analog_in_mbed_p15;
extern analog_in_t analog_in_mbed_p16;
extern analog_in_t analog_in_mbed_p17;
extern analog_in_t analog_in_mbed_p18;
extern analog_in_t analog_in_mbed_p19;
extern analog_in_t analog_in_mbed_p20;


/**
 * Initialise the analog input system. Call before using any functions in this
 * file. Powers on the ADC.
 */
void analog_in_init(void);

/**
 * Initialise a analog_in_port_t with the given port register address.
 */
analog_in_port_t analog_in_init_port_t(__IO uint32_t *pinsel, uint32_t mask);

/**
 * Initialise a analog_in_t with the given port and pin number
 */
analog_in_t analog_in_init_analog_in_t(analog_in_port_t *port,
                                       unsigned int      pin,
                                       unsigned int      analog_ch);


/**
 * Set the specified pin up as an analog input.
 */
void analog_in_set_mode(analog_in_t *analog_in);


/**
 * Read the value from the specified pin in the range from 0x0 to ADC_MAX.
 */
int analog_in_read(analog_in_t *analog_in);

#endif
