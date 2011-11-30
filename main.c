/**
 * Makebed -- A Makerbot control firmware for the mbed
 * (C) 2011 Jonathan Heathcote, University of Manchester
 */

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"

/* Peripheral support includes */
#include "usb_cdc.h"
#include "watchdog.h"
#include "gpio.h"
#include "analog_in.h"
#include "stepper.h"

/* Task priorities. */
#define mainUIP_TASK_PRIORITY (tskIDLE_PRIORITY + 3)

/* The WEB server has a larger stack as it utilises stack hungry string
handling library calls. */
#define mainBASIC_WEB_STACK_SIZE ( configMINIMAL_STACK_SIZE * 4 )

/*
 * The task that handles the uIP stack.  All TCP/IP processing is performed in
 * this task.
 */
extern void vuIP_Task( void *pvParameters );

extern char XXX_glbl_msg[64];

void
flash_task(void *pvParameters)
{
	// XXX: Check with delay of 1
	stepper_set_action(0, STEPPER_FORWARD, 100, 2);
	stepper_wait_until_idle();
	stepper_set_enabled(0, false);
	
	stepper_set_action(1, STEPPER_FORWARD, 100, 2);
	stepper_wait_until_idle();
	stepper_set_enabled(1, false);
	
	stepper_set_action(2, STEPPER_FORWARD, 300, 2);
	stepper_wait_until_idle();
	stepper_set_enabled(2, false);
	
	// Power down PSU
	//gpio_write(&gpio_mbed_p24, GPIO_LOW);
	
	portTickType last_flash = xTaskGetTickCount();
	portTickType delay      = 500 / portTICK_RATE_MS;
	
	/* FreeRTOS Flashing */
	gpio_set_mode(&gpio_mbed_led4, GPIO_OUTPUT);
	gpio_write(&gpio_mbed_led4, GPIO_HIGH);
	for (;;) {
		watchdog_feed();
		
		vTaskDelayUntil(&last_flash, delay);
		gpio_write(&gpio_mbed_led4, !gpio_read(&gpio_mbed_led4));
		
		sprintf(XXX_glbl_msg, "p20: %d, p19: %d, %x, %x",
		        analog_in_read(&analog_in_mbed_p20),
		        analog_in_read(&analog_in_mbed_p19),
		        analog_in_mbed_p20.port->mask<< ((analog_in_mbed_p20.pin * 2ul) % 32ul),
		        *(analog_in_mbed_p20.port->pinsel));
		
	}
}


int
main(void)
{
	/* Configure the hardware for use by this demo. */
	mbed_boot();
	watchdog_init();
	gpio_init();
	analog_in_init();
	stepper_init();
	//usb_init();
	
	// Make sure the PSU is off...
	gpio_set_mode(&gpio_mbed_p24, GPIO_OUTPUT);
	gpio_write(&gpio_mbed_p24, GPIO_HIGH);
	
	analog_in_set_mode(&analog_in_mbed_p20);
	analog_in_set_mode(&analog_in_mbed_p19);
	
	stepper_init_motor(0, &gpio_mbed_p10, &gpio_mbed_p9,  &gpio_mbed_p8);
	stepper_init_motor(1, &gpio_mbed_p15, &gpio_mbed_p14, &gpio_mbed_p13);
	stepper_init_motor(2, &gpio_mbed_p23, &gpio_mbed_p22, &gpio_mbed_p21);
	
	
	
	//xTaskCreate(vUSBTask,                 // Task entry function
	//            (signed char *) "USB",    // Task name
	//            configMINIMAL_STACK_SIZE, // Stack space for task
	//            (void *) NULL,            // Parameter to an argument
	//            tskIDLE_PRIORITY,         // Priority
	//            NULL);                    // Write refrence to a task handle
	
	xTaskCreate(vuIP_Task,
	            (signed char *) "uIP",
	            mainBASIC_WEB_STACK_SIZE,
	            (void *) NULL,
	            mainUIP_TASK_PRIORITY,
	            NULL);
	
	xTaskCreate(flash_task,
	            (signed char *) "flash",
	            configMINIMAL_STACK_SIZE,
	            (void *) NULL,
	            tskIDLE_PRIORITY,
	            NULL);
	
	/* Start the scheduler. */
	vTaskStartScheduler();
	
	/* Will only get here if there was insufficient memory to create the idle
	task.  The idle task is created within vTaskStartScheduler(). */
	for( ;; );
}

