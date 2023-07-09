/*
 * FreeRTOS Kernel V10.2.0
 * Copyright (C) 2019 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */

/* 
	NOTE : Tasks run in system mode and the scheduler runs in Supervisor mode.
	The processor MUST be in supervisor mode when vTaskStartScheduler is 
	called.  The demo applications included in the FreeRTOS.org download switch
	to supervisor mode prior to main being called.  If you are not using one of
	these demo application projects then ensure Supervisor mode is used.
*/


/*
 * Creates all the demo application tasks, then starts the scheduler.  The WEB
 * documentation provides more details of the demo application tasks.
 * 
 * Main.c also creates a task called "Check".  This only executes every three 
 * seconds but has the highest priority so is guaranteed to get processor time.  
 * Its main function is to check that all the other tasks are still operational.
 * Each task (other than the "flash" tasks) maintains a unique count that is 
 * incremented each time the task successfully completes its function.  Should 
 * any error occur within such a task the count is permanently halted.  The 
 * check task inspects the count of each task to ensure it has changed since
 * the last time the check task executed.  If all the count variables have 
 * changed all the tasks are still executing error free, and the check task
 * toggles the onboard LED.  Should any task contain an error at any time 
 * the LED toggle rate will change from 3 seconds to 500ms.
 *
 */

/* Standard includes. */
#include <stdlib.h>
#include <stdio.h>

/* Scheduler includes. */

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"

#include "task.h"
#include "lpc21xx.h"
#include "queue.h"
/* Peripheral includes. */
#include "serial.h"
#include "GPIO.h"

 
/*-----------------------------------------------------------*/

/* Constants to setup I/O and processor. */
#define mainBUS_CLK_FULL	( ( unsigned char ) 0x01 )

/* Constants for the ComTest demo application tasks. */
#define mainCOM_TEST_BAUD_RATE	( ( unsigned long ) 115200 )
	

/*Macros for application*/

#define MESSAGE_SIZE      26
#define QUEUE_LENGTH      10
#define COMM_PER          30
#define UART_PER          100
#define CONSUMER_PER      20



/*
 * Configure the processor for use with the Keil demo board.  This is very
 * minimal as most of the setup is managed by the settings in the project
 * file.
 */
static void prvSetupHardware( void );
/*-----------------------------------------------------------*/
 QueueHandle_t   xAppQueue                = NULL;
 TaskHandle_t    task1_btn1_Handler       = NULL;
 TaskHandle_t    task2_btn2_Handler       = NULL;
 TaskHandle_t    task3_100_ms_Handler     = NULL;
 TaskHandle_t    task4_consumer_Handler   = NULL;
 int8_t          task1_btn1_Rising_msg[]  = "-Button 1 Rising Event \r\n";
 int8_t          task1_btn1_Falling_msg[] = "-Button 1 Falling Event\r\n";
 int8_t          task2_btn2_Rising_msg[]  = "-Button 2 Rising Event \r\n";
 int8_t          task2_btn2_Falling_msg[] = "-Button 2 Falling Event\r\n";
 int8_t          task3_msg[]              = "-task3 message 100 ms  \r\n";
 int8_t          *pxRxedPointer           = NULL;

void     task1_btn1(void* pvParameters)
{
	uint8_t u8_loc_rising_sense  = pdFALSE;
	int8_t  *pc                  = NULL;

	for(;;)
	{
		if(GPIO_read(PORT_0,PIN0) == PIN_IS_HIGH && u8_loc_rising_sense == pdFALSE)
		{
			pc = task1_btn1_Rising_msg;
			if(xQueueSend( xAppQueue, &pc, portMAX_DELAY ) == pdPASS)
			{
				u8_loc_rising_sense = pdTRUE;
			}
			else
			{
				//do nothing
			}
		}
		else if (GPIO_read(PORT_0,PIN0) == PIN_IS_LOW && u8_loc_rising_sense == pdTRUE)
		{
			pc = task1_btn1_Falling_msg;
			if(xQueueSend( xAppQueue, &pc, portMAX_DELAY ) == pdPASS)
			{
				u8_loc_rising_sense = pdFALSE;
			}
			else
			{
				//do nothing
			}
		}
		else
		{
			//do nothing
		}

		vTaskDelay(COMM_PER);
	}
	
}

void     task2_btn2(void* pvParameters)
{
	uint8_t u8_loc_rising_sense  = pdFALSE;
	int8_t  *pc                  = NULL;

	for(;;)
	{
		if(GPIO_read(PORT_0,PIN1) == PIN_IS_HIGH && u8_loc_rising_sense == pdFALSE)
		{
			pc = task2_btn2_Rising_msg;
			if(xQueueSend( xAppQueue, &pc, portMAX_DELAY ) == pdPASS)
			{
				u8_loc_rising_sense = pdTRUE;
			}
			else
			{
				//do nothing
			}
		}
		else if (GPIO_read(PORT_0,PIN1) == PIN_IS_LOW && u8_loc_rising_sense == pdTRUE)
		{
			pc = task2_btn2_Falling_msg;
			if(xQueueSend( xAppQueue, &pc, portMAX_DELAY ) == pdPASS)
			{
				u8_loc_rising_sense = pdFALSE;
			}
			else
			{
				//do nothing
			}
		}
		else
		{
			//do nothing
		}

		vTaskDelay(COMM_PER);
	}
	
	
}

void task3_100_ms(void* pvParameters)
{
	volatile uint8_t counter = pdFALSE;
	int8_t  *pc              = task3_msg;
	for(;;)
	{
		// IT WILL ALWAYS SEND SAME MESSAGE
    if(xQueueSend( xAppQueue, &pc, portMAX_DELAY ) == pdTRUE)
		{
			//NOTHING
		}
		else
		{
			//NOTHING
		}
		vTaskDelay(UART_PER);
	}
	
}

void task4_consumer(void* pvParameters)
{

	for(;;)
	{
		if( xQueueReceive( xAppQueue,
                          &pxRxedPointer,
                         ( TickType_t ) 10 ) == pdPASS )
      {
         /* *pxRxedPointer now points to xMessage. */
				vSerialPutString((const signed char*)pxRxedPointer,MESSAGE_SIZE);
      }
			else
			{
				//do nothing
			}
		vTaskDelay(CONSUMER_PER);
	}
	
}




/*######################################################################################################################*/
/*######################################################################################################################*/
/*
 * Application entry point:
 * Starts all the other tasks, then starts the scheduler. 
 */
 
		
int main( void )

{
	
	prvSetupHardware();
	xAppQueue = xQueueCreate( QUEUE_LENGTH, sizeof( signed char * ));
	xTaskCreate( task1_btn1, /* Pointer to the function that implements the task. */
							 "task1_btn1",/* Text name for the task. This is to facilitate debugging only. */
							 configMINIMAL_STACK_SIZE, /* Stack depth - small microcontrollers will use much less stack than this. */
							 NULL, /* This example does not use the task parameter. */
							 1, 		/* This task will run at priority 1. */
							 &task1_btn1_Handler ); /* This example does not use the task handle. */
						
	xTaskCreate( task2_btn2, /* Pointer to the function that implements the task. */
							 "task2_btn2",/* Text name for the task. This is to facilitate debugging only. */
							 configMINIMAL_STACK_SIZE, /* Stack depth - small microcontrollers will use much less stack than this. */
							 NULL, /* This example does not use the task parameter. */
							 1, 		/* This task will run at priority 1. */
							 &task2_btn2_Handler ); /* This example does not use the task handle. */
	xTaskCreate( task3_100_ms, /* Pointer to the function that implements the task. */
							 "task3_100_ms",/* Text name for the task. This is to facilitate debugging only. */
							 configMINIMAL_STACK_SIZE, /* Stack depth - small microcontrollers will use much less stack than this. */
							 NULL, /* This example does not use the task parameter. */
							 1, 		/* This task will run at priority 1. */
							 &task3_100_ms_Handler ); /* This example does not use the task handle. */
							 
	xTaskCreate( task4_consumer, /* Pointer to the function that implements the task. */
							 "task4_consumer",/* Text name for the task. This is to facilitate debugging only. */
							 configMINIMAL_STACK_SIZE, /* Stack depth - small microcontrollers will use much less stack than this. */
							 NULL, /* This example does not use the task parameter. */
							 1, 		/* This task will run at priority 1. */
							 &task4_consumer_Handler ); /* This example does not use the task handle. */							 

	vTaskStartScheduler();

	for( ;; );
}
/*-----------------------------------------------------------*/
/*######################################################################################################################################*/
/*######################################################################################################################################*/
static void prvSetupHardware( void )
{
	/* Perform the hardware setup required.  This is minimal as most of the
	setup is managed by the settings in the project file. */

	/* Configure UART */
	xSerialPortInitMinimal(mainCOM_TEST_BAUD_RATE);

	/* Configure GPIO */
	GPIO_init();

	/* Setup the peripheral bus to be the same as the PLL output. */
	VPBDIV = mainBUS_CLK_FULL;
}
/*-----------------------------------------------------------*/

