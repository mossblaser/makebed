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
		
		vTaskDelayUntil(&last_update, delay);
		
		const double pi = 3.14159265358979323846;
		const double speed = 63.0;
		const double radius = 20.0;
		const double height = -5.0;
		const int segments = 10;
		const int its = 10;
		
		const double x_off = -50.0;
		const double y_off = 50.0;
		
		double x;
		double y;
		
		makerbot_move_to((double[3]){0.0,0.0,0.0}, 2.0);
		makerbot_move_to((double[3]){0.0,0.0,height}, 2.0);
		makerbot_move_to((double[3]){x_off,y_off,height}, speed);
		
		int segment;
		int i;
		for (i = 0; i < its; i++) {
			for (segment = 0; segment < segments; segment++) {
				double theta = ((2.0*pi) / ((double)(segments))) * ((double)(segment));
				x = radius * cos(theta);
				y = radius * sin(theta);
				makerbot_move_to((double[3]){x+x_off,y+y_off,height}, speed);
			}
		}
		
		makerbot_move_to((double[3]){x_off,y_off,height}, speed);
		makerbot_move_to((double[3]){0.0,0.0,height}, speed);
		makerbot_move_to((double[3]){0.0,0.0,0.0}, 2.0);
		
		makerbot_set_power(false);
		
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
	            tskIDLE_PRIORITY,
	            NULL);
	
	/* Start the scheduler. */
	vTaskStartScheduler();
	
	/* Will only get here if there was insufficient memory to create the idle
	task.  The idle task is created within vTaskStartScheduler(). */
	for( ;; );
}

