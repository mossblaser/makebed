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
DEMO_SOURCE_DIR=/home/jonathan/Programing/mbed/FreeRTOSv7.0.2/Demo/Common/Minimal

CFLAGS+= -I /home/jonathan/Programing/mbed/FreeRTOSv7.0.2/Demo/Common/include -I webserver
CFLAGS+=-I . -I ${RTOS_SOURCE_DIR}/include -I ${RTOS_SOURCE_DIR}/portable/GCC/ARM_CM3 -I /home/jonathan/Programing/mbed/FreeRTOSv7.0.2/Demo/Common/include -D GCC_ARMCM3_LM3S102 -D inline=

VPATH=${RTOS_SOURCE_DIR}:${RTOS_SOURCE_DIR}/portable/MemMang:${RTOS_SOURCE_DIR}/portable/GCC/ARM_CM3:${DEMO_SOURCE_DIR}:init:webserver:LPCUSB:GPIO

OBJS=${COMPILER}/main.o	\
	  ${COMPILER}/list.o    \
	  ${COMPILER}/queue.o   \
	  ${COMPILER}/tasks.o   \
	  ${COMPILER}/port.o    \
	  ${COMPILER}/heap_1.o  \
	  ${COMPILER}/BlockQ.o	\
	  ${COMPILER}/PollQ.o	\
	  ${COMPILER}/QPeek.o \
	  ${COMPILER}/GenQTest.o \
	  ${COMPILER}/blocktim.o \
	  ${COMPILER}/flash.o \
	  ${COMPILER}/recmutex.o \
	  ${COMPILER}/integer.o	\
	  ${COMPILER}/semtest.o \
	  ${COMPILER}/emac.o \
	  ${COMPILER}/ParTest.o \
	  ${COMPILER}/syscalls.o \
	  ${COMPILER}/printf-stdarg.o \
	  ${COMPILER}/httpd-cgi.o ${COMPILER}/psock.o ${COMPILER}/uip_arp.o ${COMPILER}/uIP_Task.o \
	  ${COMPILER}/httpd.o ${COMPILER}/httpd-fs.o ${COMPILER}/http-strings.o ${COMPILER}/timer.o ${COMPILER}/uip.o \
	  ${COMPILER}/usbcontrol.o \
	  ${COMPILER}/USB_CDC.o \
	  ${COMPILER}/usbhw_lpc.o \
	  ${COMPILER}/usbinit.o \
	  ${COMPILER}/usbstdreq.o



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
	@cp ${COMPILER}/RTOSDemo.bin /media/mbed/freertos-lpc1768.bin

#
#
# Include the automatically generated dependency files.
#
-include ${wildcard ${COMPILER}/*.d} __dummy__


	 



