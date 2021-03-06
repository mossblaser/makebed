#******************************************************************************
#
# Makefile - Rules for building the driver library and examples.
#
# Copyright (c) 2005,2006 Luminary Micro, Inc.  All rights reserved.
#
# Software License Agreement
#
# Luminary Micro, Inc. (LMI) is supplying this software for use solely and
# exclusively on LMI's Stellaris Family of microcontroller products.
#
# The software is owned by LMI and/or its suppliers, and is protected under
# applicable copyright laws.  All rights are reserved.  Any use in violation
# of the foregoing restrictions may subject the user to criminal sanctions
# under applicable laws, as well as to civil liability for the breach of the
# terms and conditions of this license.
#
# THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
# OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
# LMI SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR
# CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
#
#******************************************************************************

include makedefs

RTOS_SOURCE_DIR=/home/jonathan/Programing/mbed/FreeRTOSv7.0.2/Source

CFLAGS+=-I LPCUSB -I float_parsing -I gcode -I makerbot -I pid -I thermistor -I watchdog -I GPIO -I analog -I stepper -I network
CFLAGS+=-I . -I ${RTOS_SOURCE_DIR}/include -I ${RTOS_SOURCE_DIR}/portable/GCC/ARM_CM3 -D GCC_ARMCM3_LM3S102 -D inline=

VPATH=${RTOS_SOURCE_DIR}:${RTOS_SOURCE_DIR}/portable/MemMang:${RTOS_SOURCE_DIR}/portable/GCC/ARM_CM3:$init:network:LPCUSB:GPIO:analog:stepper:watchdog:thermistor:pid:makerbot:gcode:float_parsing

OBJS=${COMPILER}/main.o	\
	  ${COMPILER}/mbed_boot.o    \
	  ${COMPILER}/freertos_hooks.o    \
	  ${COMPILER}/list.o    \
	  ${COMPILER}/queue.o   \
	  ${COMPILER}/tasks.o   \
	  ${COMPILER}/port.o    \
	  ${COMPILER}/heap_1.o  \
	  ${COMPILER}/emac.o \
	  ${COMPILER}/syscalls.o \
	  ${COMPILER}/printf-stdarg.o \
	  ${COMPILER}/psock.o \
	  ${COMPILER}/uip_arp.o \
	  ${COMPILER}/uip_task.o \
	  ${COMPILER}/timer.o \
	  ${COMPILER}/uip.o \
	  ${COMPILER}/network.o \
	  ${COMPILER}/network_gcode.o \
	  ${COMPILER}/network_debug.o \
	  ${COMPILER}/usbcontrol.o \
	  ${COMPILER}/USB_CDC.o \
	  ${COMPILER}/usbhw_lpc.o \
	  ${COMPILER}/usbinit.o \
	  ${COMPILER}/usbstdreq.o \
	  ${COMPILER}/gpio.o \
	  ${COMPILER}/analog_in.o \
	  ${COMPILER}/watchdog.o \
	  ${COMPILER}/thermistor.o \
	  ${COMPILER}/pid.o \
	  ${COMPILER}/makerbot.o \
	  ${COMPILER}/gcode.o \
	  ${COMPILER}/stepper.o \
	  ${COMPILER}/strtod.o



INIT_OBJS= ${COMPILER}/cr_startup_lpc17.o


#
# The default rule, which causes init to be built.
#
all: ${COMPILER}           \
     ${COMPILER}/RTOSDemo.axf \
	 
#
# The rule to clean out all the build products
#

clean:
	@rm -rf ${COMPILER} ${wildcard *.bin} RTOSDemo.axf
	
#
# The rule to create the target directory
#
${COMPILER}:
	@mkdir ${COMPILER}

${COMPILER}/RTOSDemo.axf: ${INIT_OBJS} ${OBJS} ${LIBS}
SCATTER_RTOSDemo=rtosdemo_rdb1768_Debug.ld
ENTRY_RTOSDemo=ResetISR

install: all
	@cp ${COMPILER}/RTOSDemo.bin /media/MBED/freertos-lpc1768.bin

#
#
# Include the automatically generated dependency files.
#
-include ${wildcard ${COMPILER}/*.d} __dummy__


	 



