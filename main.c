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

/* Task priorities. */
#define mainUIP_TASK_PRIORITY (tskIDLE_PRIORITY + 2)
#define mainMAKEBED_TASK_PRIORITY (tskIDLE_PRIORITY + 3)

/* The WEB server has a larger stack as it utilises stack hungry string
handling library calls. */
#define mainBASIC_WEB_STACK_SIZE ( configMINIMAL_STACK_SIZE * 4 )

/*
 * The task that handles the uIP stack.  All TCP/IP processing is performed in
 * this task.
 */
extern void vuIP_Task( void *pvParameters );

extern char XXX_glbl_msg[64];
extern makerbot_command_t *XXX_cur_cmd;

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
		
		double temp0 = makerbot_get_temperature(0);
		double temp1 = makerbot_get_temperature(1);
		
		sprintf(XXX_glbl_msg, "Size: %d, Temps: %d%c, %d%c, cmd.instr: %d, tmp:%d, rsid:%x",
		        sizeof(makerbot_command_t),
		        (int)(temp0), _makerbot_temperature_reached(0) ? '!' : ' ',
		        (int)(temp1), _makerbot_temperature_reached(1) ? '!' : ' ',
		        XXX_cur_cmd->instr,
		        XXX_cur_cmd->arg.set_temperature.heater_num,
		        rsid
		        );
		
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
		makerbot_set_origin((double[3]){0.0,0.0,-30.0});
		
		// Move up to a heating position
		makerbot_move_to((double[3]){0.0,0.0,-30.0}, 2.0);
		
		// Heat up
		makerbot_set_temperature(0, 210.0);
		makerbot_wait_heaters();
		
		// Move off to the side and squirt some plastic
		makerbot_move_to((double[3]){-50.0,0.0,-30.0}, 10.0);
		makerbot_set_extruder(true);
		makerbot_sleep(3000);
		makerbot_set_extruder(false);
		makerbot_sleep(5000);
		makerbot_move_to((double[3]){0.0,0.0,-30.0}, 10.0);
		
		// Descend onto the platform and squirt out a 2cm box.
		makerbot_move_to((double[3]){0.0,0.0,0.5}, 2.0);
		makerbot_set_extruder(true);
		makerbot_move_to((double[3]){-10.0,-10.0,0.5}, 10.0);
		makerbot_move_to((double[3]){ 10.0,-10.0,0.5}, 10.0);
		makerbot_move_to((double[3]){ 10.0, 10.0,0.5}, 10.0);
		makerbot_move_to((double[3]){-10.0, 10.0,0.5}, 10.0);
		makerbot_move_to((double[3]){-10.0,-10.0,0.5}, 10.0);
		makerbot_set_extruder(false);
		makerbot_move_to((double[3]){0.0,0.0,-30.0}, 2.0);
		
		// Eject the print
		makerbot_set_platform(true);
		makerbot_sleep(5000);
		makerbot_set_platform(false);
		
		// Turn off, done!
		makerbot_set_power(false);
		
		// Done!
		gpio_set_mode(&gpio_mbed_led1, GPIO_OUTPUT);
		gpio_write(&gpio_mbed_led1, GPIO_HIGH);
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
	
	/* Start the scheduler. */
	vTaskStartScheduler();
	
	/* Will only get here if there was insufficient memory to create the idle
	task.  The idle task is created within vTaskStartScheduler(). */
	for( ;; );
}

