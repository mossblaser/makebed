#ifndef USB_CDC_H
#define USB_CDC_H

#include "FreeRTOS.h"
#include "queue.h"

extern xQueueHandle xRxedChars;
extern xQueueHandle xCharsForTx;

void usb_init(void);
void vUSBTask(void *pvParameters);

int VCOM_putchar(int c);
void VCOM_putstr(char *s);
void VCOM_putuint(unsigned int value);
void VCOM_putuint_base(unsigned int value, unsigned int base);

#endif
