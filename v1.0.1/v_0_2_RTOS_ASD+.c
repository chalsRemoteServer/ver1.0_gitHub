/**
  @file
  @brief
  Free RTOS Example 3

  An example with FreeRTOS illustrating the use of time-slicing, preemption and mutex based synchronization.
*/
/*
 * ============================================================================
 * History
 * =======
 * 29 Dec 2015 : Created
 *
 * Copyright (C) Bridgetek Pte Ltd
 * ============================================================================
 *
 * This source code ("the Software") is provided by Bridgetek Pte Ltd 
 * ("Bridgetek") subject to the licence terms set out
 * http://brtchip.com/BRTSourceCodeLicenseAgreement/ ("the Licence Terms").
 * You must read the Licence Terms before downloading or using the Software.
 * By installing or using the Software you agree to the Licence Terms. If you
 * do not agree to the Licence Terms then do not download or use the Software.
 *
 * Without prejudice to the Licence Terms, here is a summary of some of the key
 * terms of the Licence Terms (and in the event of any conflict between this
 * summary and the Licence Terms then the text of the Licence Terms will
 * prevail).
 *
 * The Software is provided "as is".
 * There are no warranties (or similar) in relation to the quality of the
 * Software. You use it at your own risk.
 * The Software should not be used in, or for, any medical device, system or
 * appliance. There are exclusions of Bridgetek liability for certain types of loss
 * such as: special loss or damage; incidental loss or damage; indirect or
 * consequential loss or damage; loss of income; loss of business; loss of
 * profits; loss of revenue; loss of contracts; business interruption; loss of
 * the use of money or anticipated savings; loss of information; loss of
 * opportunity; loss of goodwill or reputation; and/or loss of, damage to or
 * corruption of data.
 * There is a monetary cap on Bridgetek's liability.
 * The Software may have subsequently been amended by another user and then
 * distributed by that other user ("Adapted Software").  If so that user may
 * have additional licence terms that apply to those amendments. However, Bridgetek
 * has no liability in relation to those amendments.
 * ============================================================================
 */

#include <stdint.h>
#include <stdio.h>
#include "ft900.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "TFT.h"
#include "system.h"


#define STACK_DISP	 128
#define STACK_TXSER  256
#define STACK_MONSTK 128
TaskHandle_t xTareaDisplayHandle = NULL;
TaskHandle_t xTareaTxHandle = NULL;      // Nueva tarea

void vStart_Priority_Tasks( void );
//static void vDispTask(void *pvParam);
static void vTxSerTask(void *pvParam);
static void vMonTask(void *pvParam);
static SemaphoreHandle_t xMutex = NULL;
int delay = portMAX_DELAY;
void UART_1_ISR(void);



int main(void)
{
	sys_reset_all(); /****** Peripheral reset ******/
	sys_enable(sys_device_uart0);/* enable uart */
	sys_enable(sys_device_uart1);/* enable uart */

	sys_enable(sys_device_timer_wdt);
    gpio_function(48, pad_uart0_txd); /* UART0 TXD */ /* Set UART0 GPIO functions to UART0_TXD and UART0_RXD... */
    gpio_function(49, pad_uart0_rxd); /* UART0 RXD */
    gpio_function(52, pad_uart1_txd); /* UART1 TXD */ /* Set UART0 GPIO functions to UART0_TXD and UART0_RXD... */
	gpio_function(53, pad_uart1_rxd); /* UART1 RXD */

	uart_open(UART0, 1,UART_DIVIDER_19200_BAUD, uart_data_bits_8, uart_parity_none,uart_stop_bits_1);
	uart_open(UART1, 1,UART_DIVIDER_19200_BAUD, uart_data_bits_8, uart_parity_odd,uart_stop_bits_2);

	uart_puts(UART0,/* Print out a welcome message... */
			"Copyright (c) Gracida Sistemas \r\n"
			"--------------------------------------------------------------------- \r\n"
			"Welcome to Free RTOS Test ASD+ \r\n"
			"version "VERSION
			"\r\n"
			"Communication tasks which will send and receive data over UART1 using FreeRTOS Queues\r\n"
			"Please ensure that UART1 RX and TX lines are connected to each other.\r\n"
			"--------------------------------------------------------------------- \r\n"
			 );//fin mensaje de inicio del uart0
	uart_puts(UART1,"Uart1 iniciado");/* Print out a welcome message... */


	    interrupt_attach(interrupt_uart1, (uint8_t) interrupt_uart1, UART_1_ISR);/* Attach the interrupt so it can be called... */
	    uart_enable_interrupt(UART1, uart_interrupt_rx);/* Enable the UART to fire interrupts when receiving data... */
	    uart_enable_interrupts_globally(UART1);/* Enable interrupts to be fired... */
	    interrupt_enable_globally();

	//vStart_Pantalla_TFT();//_Encender Pantalla TFT


    TFT_init();
    vStart_Priority_Tasks();


    /*while(1)
    {TFT_display();
    	delayms(5);}*/

    vTaskStartScheduler();/*RTOS start Now all the tasks have been started - start the scheduler. */
}//fin main +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


void UART_1_ISR(){
static uint8_t c;

    if (uart_is_interrupted(UART1, uart_interrupt_rx)){
    	xSemaphoreTake(xMutex, delay);
        uart_read(UART1, &c);
        uart_write(UART1, c);
    	xSemaphoreGive(xMutex);}
}//ifin interrupt------------------------------------------



void vStart_Priority_Tasks( void ){
#if( ( configUSE_MUTEXES == 1 ) && ( configSUPPORT_DYNAMIC_ALLOCATION == 1 ) )
	xMutex = xSemaphoreCreateMutex();
#else
#error "error: no esta active USE MUTEX y SUPP DYNAMC ALLOCATION"
#endif
#if( configSUPPORT_DYNAMIC_ALLOCATION == 1 )
//	xTaskCreate( vDispTask,"Disp_Man",STACK_DISP, NULL,3,&xTareaDisplayHandle );
	xTaskCreate( vTxSerTask,"Ser_Man",STACK_TXSER, NULL,4,&xTareaTxHandle );
	xTaskCreate( vMonTask,"Mon_tsk",256, NULL, 2, NULL );
#else
#error "no esta activado SUPP DYNAMIC ALLOC"
#endif
}//vStart_Priority_Tasks------------------------------------------------


static void vTxSerTask(void *pvParam){
//UBaseType_t uxHighWaterMark;
uint32_t count=0;
  for(;;){
	//uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
	xSemaphoreTake(xMutex, delay);
	printf("\nserial task: %lu",count++);//Recibido
	uart_puts(UART1,"Uart1 transmitiendo");/* Print out a welcome message... */
	xSemaphoreGive(xMutex);
	vTaskDelay(650);}//wait 5 ms
}//fin task serial task--------------------------------

static void vMonTask(void *pvParam){
//UBaseType_t uxStackdisplay;
UBaseType_t uxStacktxserial;
    for(;;){
    	//if(xTareaDisplayHandle != NULL) {
    	//	uxStackdisplay = uxTaskGetStackHighWaterMark(xTareaDisplayHandle);
			//printf("\nMon Disp:%lu bytes)", uxStackdisplay * 4);}
    	if(xTareaTxHandle != NULL) {
    		xSemaphoreTake(xMutex, delay);
    		uxStacktxserial = uxTaskGetStackHighWaterMark(xTareaTxHandle);
			printf("\nMon TX:%lu words)",STACK_TXSER- uxStacktxserial);
			xSemaphoreGive(xMutex);}
    	vTaskDelay(1050);//wait mseg
      }//fin for
}//vstack mon task------------------------------------
