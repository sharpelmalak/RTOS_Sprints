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
#include "semphr.h"
/* Peripheral includes. */
#include "serial.h"
#include "GPIO.h"

 
/*-----------------------------------------------------------*/

/* Constants to setup I/O and processor. */
#define mainBUS_CLK_FULL	( ( unsigned char ) 0x01 )

/* Constants for the ComTest demo application tasks. */
#define mainCOM_TEST_BAUD_RATE	( ( unsigned long ) 115200 )
	

/*Macros for application*/

#define  END_MESSAGE_SIZE        19
#define  MESSAGE_SIZE            22
#define  MESSAGES_PER_CYCLE      10
#define  TASK_2_CPU_LOAD         100000
#define  TASK_1_HANDLER          5000
#define  TASK_1_PER              100
#define  TASK_2_PER              500


/*
 * Configure the processor for use with the Keil demo board.  This is very
 * minimal as most of the setup is managed by the settings in the project
 * file.
 */
static void prvSetupHardware( void );
/*-----------------------------------------------------------*/

         TaskHandle_t      task1_100_ms_Handler = NULL;
         TaskHandle_t      task2_500_ms_Handler = NULL;
         SemaphoreHandle_t UART_Semaphore       = NULL;
const    int8_t            task_1_msg[]         = "task1 message 100 ms\n";
const    int8_t            task_2_msg[]         = "task2 message 500 ms\n";
volatile int32_t           i32_gl_counter       = pdFALSE;
void     task2_500_ms(void* pvParameters)
{
	volatile uint8_t counter = pdFALSE;
	for(;;)
	{

		
		if(xSemaphoreTake(UART_Semaphore,portMAX_DELAY) == pdTRUE)
		{
			for(counter = pdFALSE ; counter < MESSAGES_PER_CYCLE ; counter ++)
			{
				vSerialPutString(task_2_msg,MESSAGE_SIZE);
				for(i32_gl_counter=pdFALSE;i32_gl_counter<TASK_2_CPU_LOAD;)
				{
					i32_gl_counter++;
				}
			}
			
			vSerialPutString((const signed char*)"-----endtask2-----\n",END_MESSAGE_SIZE);
			xSemaphoreGive(UART_Semaphore);
		}
		else
		{
			//do nothing
		}
		
		vTaskDelay(TASK_2_PER);
	}
	
}

void task1_100_ms(void* pvParameters)
{
	volatile uint8_t counter = pdFALSE;
	for(;;)
	{


		if(xSemaphoreTake(UART_Semaphore,portMAX_DELAY) == pdTRUE)
		{
			for(counter = pdFALSE ; counter < MESSAGES_PER_CYCLE ; counter ++)
			{
				vSerialPutString(task_1_msg,MESSAGE_SIZE);
				for(i32_gl_counter=pdFALSE;i32_gl_counter<TASK_1_HANDLER;)
				{
					i32_gl_counter++;
				}
			}
			vSerialPutString((const signed char*)"-----endtask1-----\n",END_MESSAGE_SIZE);
			xSemaphoreGive(UART_Semaphore);
		}
		else
		{
			//do nothing
		}
		
		vTaskDelay(TASK_1_PER);
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
	
	UART_Semaphore =xSemaphoreCreateBinary();	//creat semaphore for protecting the shared uart
	
	xSemaphoreGive(UART_Semaphore); // initial value
	
	xTaskCreate( task1_100_ms, /* Pointer to the function that implements the task. */
							 "task1_100_ms",/* Text name for the task. This is to facilitate debugging only. */
							 configMINIMAL_STACK_SIZE, /* Stack depth - small microcontrollers will use much less stack than this. */
							 NULL, /* This example does not use the task parameter. */
							 2, 		/* This task will run at priority 2. */
							 &task1_100_ms_Handler ); /* This example does not use the task handle. */
						
	xTaskCreate( task2_500_ms, /* Pointer to the function that implements the task. */
							 "task2_500_ms",/* Text name for the task. This is to facilitate debugging only. */
							 configMINIMAL_STACK_SIZE, /* Stack depth - small microcontrollers will use much less stack than this. */
							 NULL, /* This example does not use the task parameter. */
							 1, 		/* This task will run at priority 1. */
							 &task2_500_ms_Handler ); /* This example does not use the task handle. */							

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

