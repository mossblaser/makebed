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
#include "thermistor.h"
#include "pid.h"
#include "makerbot.h"

/* Task priorities. */
#define mainUIP_TASK_PRIORITY (tskIDLE_PRIORITY + 3)
#define mainMAKEBED_TASK_PRIORITY (tskIDLE_PRIORITY + 0)

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
	portTickType last_flash = xTaskGetTickCount();
	portTickType delay      = 500 / portTICK_RATE_MS;
	
	/* FreeRTOS Flashing */
	gpio_set_mode(&gpio_mbed_led4, GPIO_OUTPUT);
	gpio_write(&gpio_mbed_led4, GPIO_HIGH);
	for (;;) {
		//watchdog_feed();
		
		double temp0 = makerbot_get_temperature(0);
		double temp1 = makerbot_get_temperature(1);
		
		//sprintf(XXX_glbl_msg, "Temps: %d%c, %d%c",
		//        (int)(temp0), makerbot_temperature_reached(0) ? '!' : ' ',
		//        (int)(temp1), makerbot_temperature_reached(1) ? '!' : ' ');
		
		vTaskDelayUntil(&last_flash, delay);
		gpio_write(&gpio_mbed_led4, !gpio_read(&gpio_mbed_led4));
	}
}

void
pid_task(void *pvParameters)
{
	portTickType last_update = xTaskGetTickCount();
	portTickType delay      = 500 / portTICK_RATE_MS;
	
	for (;;) {
		vTaskDelayUntil(&last_update, delay);
		
		makerbot_set_power(true);
		makerbot_set_origin((double[3]){0.0,0.0,0.0});
		
		//makerbot_set_temperature(0, 210);
		//makerbot_set_temperature(1, 0);
		makerbot_wait_heaters();
		
		//last_update = xTaskGetTickCount();
		//vTaskDelayUntil(&last_update, delay*120);
		//
		//makerbot_set_power(false);
		
		for (;;)
			;
	}
}

volatile double val = 10.5;

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
	//usb_init();
	
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
	
	xTaskCreate(pid_task,
	            (signed char *) "pid",
	            configMINIMAL_STACK_SIZE * 8,
	            (void *) NULL,
	            tskIDLE_PRIORITY,
	            NULL);
	
	xTaskCreate(makerbot_task,
	            (signed char *) "makerbot",
	            configMINIMAL_STACK_SIZE * 8,
	            (void *) NULL,
	            mainMAKEBED_TASK_PRIORITY,
	            NULL);
	
	/* Start the scheduler. */
	vTaskStartScheduler();
	
	/* Will only get here if there was insufficient memory to create the idle
	task.  The idle task is created within vTaskStartScheduler(). */
	for( ;; );
}

