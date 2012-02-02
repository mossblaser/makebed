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
	//stepper_set_action(0, STEPPER_FORWARD, 100, 2);
	//stepper_wait_until_idle();
	//stepper_set_enabled(0, false);
	//
	//stepper_set_action(1, STEPPER_FORWARD, 100, 2);
	//stepper_wait_until_idle();
	//stepper_set_enabled(1, false);
	//
	//stepper_set_action(2, STEPPER_FORWARD, 300, 2);
	//stepper_wait_until_idle();
	//stepper_set_enabled(2, false);
	
	// Power down PSU
	//gpio_write(&gpio_mbed_p24, GPIO_LOW);
	
	portTickType last_flash = xTaskGetTickCount();
	portTickType delay      = 500 / portTICK_RATE_MS;
	
	thermistor_t thermistor;
	thermistor_init(&thermistor, 4400.0, 100000.0, 4072.0, 25.0 + 273.15);
	
	/* FreeRTOS Flashing */
	gpio_set_mode(&gpio_mbed_led4, GPIO_OUTPUT);
	gpio_write(&gpio_mbed_led4, GPIO_HIGH);
	for (;;) {
		watchdog_feed();
		
		vTaskDelayUntil(&last_flash, delay);
		gpio_write(&gpio_mbed_led4, !gpio_read(&gpio_mbed_led4));
	}
}

extern bool enable_heater;

void
pid_task(void *pvParameters)
{
	portTickType last_update = xTaskGetTickCount();
	portTickType delay      = 500 / portTICK_RATE_MS;
	
	thermistor_t thermistor;
	thermistor_init(&thermistor, 4400.0, 100000.0, 4072.0, 25.0 + 273.15);
	
	pid_state_t pid;
	//pid_init(&pid, 7.0, 0.3, 36.0, 0.0); // pid0
	//pid_init(&pid, 6.25, 0.4140625, 100.0, 0.0); // pid1
	//pid_init(&pid, 6.25, 0.4140625, 140.0, 0.0); // pid2
	pid_init(&pid, 6.25, 0.4140625, 200.0, 0.0);
	
	// LED
	gpio_set_mode(&gpio_mbed_led3, GPIO_OUTPUT);
	gpio_write(&gpio_mbed_led3, GPIO_LOW);
	
	// Heater
	gpio_set_mode(PIN_HEATER_EXTRUDER, GPIO_OUTPUT);
	gpio_write(PIN_HEATER_EXTRUDER, GPIO_LOW);
	
	// Extruder
	gpio_set_mode(&gpio_mbed_p29, GPIO_OUTPUT);
	gpio_write(&gpio_mbed_p29, GPIO_LOW);
	
	for (;;) {
		vTaskDelayUntil(&last_update, delay);
		
		if (enable_heater) {
			double temperature = thermistor_convert(&thermistor,
			                                        (double)analog_in_read(PIN_THERMISTOR_EXTRUDER) / 4086.0) - 273.0;
			double control = pid_update(&pid, 210.0, temperature, 500.0);
			
			sprintf(XXX_glbl_msg, "p19: %d, PID: %d, P: %d, I: %d, D:%d, Integral: %d",
			        (int)(temperature*10),
			        (int)control,
			        (int)(pid.Kp * 100.0),
			        (int)(pid.Ki * 100.0),
			        (int)(pid.Kd * 100.0),
			        (int)(pid.integral * 100.0)
			);
			
			gpio_write(&gpio_mbed_led3, control > 0.0);
			gpio_write(PIN_HEATER_EXTRUDER, control > 0.0);
			gpio_write(&gpio_mbed_p29, control < 0.0);
			gpio_write(&gpio_mbed_p24, GPIO_HIGH);
		} else {
			gpio_write(&gpio_mbed_led3, GPIO_LOW);
			gpio_write(PIN_HEATER_EXTRUDER, GPIO_LOW);
			gpio_write(&gpio_mbed_p29, GPIO_LOW);
			gpio_write(&gpio_mbed_p24, GPIO_LOW);
		}
		
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
	//usb_init();
	
	// Make sure the PSU is off...
	gpio_set_mode(&gpio_mbed_p24, GPIO_OUTPUT);
	gpio_write(&gpio_mbed_p24, GPIO_LOW);
	
	enable_heater = false;
	
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
	
	xTaskCreate(pid_task,
	            (signed char *) "pid",
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

