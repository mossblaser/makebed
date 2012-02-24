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
	
	// Store and clear RESET
	int rsid = LPC_SC->RSID;
	LPC_SC->RSID = LPC_SC->RSID;
	
	for (;;) {
		//watchdog_feed();
		
		vTaskDelayUntil(&last_flash, delay);
		//gpio_write(&gpio_mbed_led4, !gpio_read(&gpio_mbed_led4));
	}
}

char test_gcode[] = "G4 P10000      \r\n" // Sleep for 10 seconds
                    "G21            \r\n" // Use mm
                    "G92 X0 Y0 Z0   \r\n" // Zero the positions
                    "G1 X10 F3000   \r\n" // Move to 10,0,0
                    "G4 P1000       \r\n" // Wait 1000ms
                    "G1 X0 F2000    \r\n" // Move back to 0,0,0
                    "M109 S50       \r\n" // Heat bed to 50*c
                    "M6             \r\n" // Wait for heaters
                    "G1 Y10 F3000   \r\n" // Move to 0,10,0
                    "G4 P1000       \r\n" // Wait 1000ms
                    "G1 Y0 F2000    \r\n" // Move back to 0,0,0
                    ;

void
my_task(void *pvParameters)
{
	portTickType last_update = xTaskGetTickCount();
	portTickType delay      = 500 / portTICK_RATE_MS;
	makerbot_set_power(true);
	
	for (;;) {
		vTaskDelayUntil(&last_update, delay);
		
		//gcode_interpret(test_gcode, sizeof(test_gcode));
		//
		//for (;;)
		//	;
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
	
	xTaskCreate(my_task,
	            (signed char *) "my",
	            configMINIMAL_STACK_SIZE * 8,
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

