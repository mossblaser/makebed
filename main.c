/**
 * Makebed -- A Makerbot control firmware for the mbed
 * (C) 2011 Jonathan Heathcote, University of Manchester
 */

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"

/* Peripheral support includes */
#include "usb_cdc.h"
#include "gpio.h"

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

int value = 0;

void
TIMER0_IRQHandler(void)
{
	gpio_write(&gpio_mbed_led3, !gpio_read(&gpio_mbed_led3));
	
	LPC_TIM0->IR = LPC_TIM0->IR;
	NVIC_ClearPendingIRQ(TIMER0_IRQn);
}



void
flash_task(void *pvParameters)
{
	portTickType last_flash = xTaskGetTickCount();
	portTickType delay      = 1000 / portTICK_RATE_MS;
	
	/* Timer flashing */
	gpio_set_mode(&gpio_mbed_led3, GPIO_OUTPUT);
	LPC_TIM0->TCR  = 2;         // Reset & disable counter/timer
	LPC_TIM0->PR   = 99999;     // For 1kHz when running at 100mHz
	LPC_TIM0->MR0  = 999;       // Every 100ms
	LPC_TIM0->MCR  = 3;         // Reset and interrupt on MR0 match
	LPC_TIM0->TCR  = 1;         // Enable and remove reset
	NVIC_EnableIRQ(TIMER0_IRQn); // Enable interupt
	
	
	/* FreeRTOS Flashing */
	gpio_set_mode(&gpio_mbed_led1, GPIO_OUTPUT);
	for (;;) {
		vTaskDelayUntil(&last_flash, delay);
		gpio_write(&gpio_mbed_led1, !gpio_read(&gpio_mbed_led1));
	}
}


int
main(void)
{
	/* Configure the hardware for use by this demo. */
	mbed_boot();
	gpio_init();
	usb_init();
	
	xTaskCreate(vUSBTask,                 // Task entry function
	            (signed char *) "USB",    // Task name
	            configMINIMAL_STACK_SIZE, // Stack space for task
	            (void *) NULL,            // Parameter to an argument
	            tskIDLE_PRIORITY,         // Priority
	            NULL);                    // Write refrence to a task handle
	
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

