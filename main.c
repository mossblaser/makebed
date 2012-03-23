/**
 * Makebed -- A Makerbot control firmware for the mbed
 * (C) 2011 Jonathan Heathcote, University of Manchester
 */

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"

#include "LPC17xx.h"

/* Peripheral support includes */
#include "usb_cdc.h"
#include "watchdog.h"
#include "gpio.h"
#include "analog_in.h"
#include "stepper.h"
#include "thermistor.h"
#include "pid.h"
#include "makerbot.h"
#include "gcode.h"
#include "network.h"
#include "network_debug.h"

/* Task priorities. */
#define mainUIP_TASK_PRIORITY (tskIDLE_PRIORITY + 2)
#define mainMAKEBED_TASK_PRIORITY (tskIDLE_PRIORITY + 3)

/* The WEB server has a larger stack as it utilises stack hungry string
handling library calls. */
#define mainBASIC_WEB_STACK_SIZE ( configMINIMAL_STACK_SIZE * 16 )


void
flash_task(void *pvParameters)
{
	portTickType last_flash = xTaskGetTickCount();
	portTickType delay      = 100 / portTICK_RATE_MS;
	
	/* FreeRTOS Flashing */
	gpio_set_mode(&gpio_mbed_led4, GPIO_OUTPUT);
	gpio_set_mode(PIN_ENDSTOP_2_MAX, GPIO_INPUT);
	gpio_write(&gpio_mbed_led4, GPIO_HIGH);
	
	for (;;) {
		vTaskDelayUntil(&last_flash, delay);
		gpio_write(&gpio_mbed_led4, !gpio_read(PIN_ENDSTOP_2_MAX));
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
	makerbot_init();
	gcode_init();
	network_debug_init();
	//usb_init();
	
	//xTaskCreate(vUSBTask,                 // Task entry function
	//            (signed char *) "USB",    // Task name
	//            configMINIMAL_STACK_SIZE, // Stack space for task
	//            (void *) NULL,            // Parameter to an argument
	//            tskIDLE_PRIORITY,         // Priority
	//            NULL);                    // Write refrence to a task handle
	
	xTaskCreate(uip_task,
	            (signed char *) "network",
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
	
	xTaskCreate(makerbot_main_task,
	            (signed char *) "mb_main",
	            configMINIMAL_STACK_SIZE * 8,
	            (void *) NULL,
	            mainMAKEBED_TASK_PRIORITY,
	            NULL);
	
	xTaskCreate(makerbot_pid_task,
	            (signed char *) "mb_pid",
	            configMINIMAL_STACK_SIZE * 8,
	            (void *) NULL,
	            mainMAKEBED_TASK_PRIORITY,
	            NULL);
	
	xTaskCreate(gcode_task,
	            (signed char *) "gcode",
	            configMINIMAL_STACK_SIZE * 2,
	            (void *) NULL,
	            mainUIP_TASK_PRIORITY,
	            NULL);
	
	/* Start the scheduler. */
	vTaskStartScheduler();
	
	/* Will only get here if there was insufficient memory to create the idle
	task.  The idle task is created within vTaskStartScheduler(). */
	for( ;; );
}

