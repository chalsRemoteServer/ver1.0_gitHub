/*
 * RTOS.h
 *
 *  Created on: Nov 26, 2025
 *      Author: desarrollo_1
 */

#ifndef ASD_APP_INCLUDE_RTOS_H_
#define ASD_APP_INCLUDE_RTOS_H_

//prioridades de las tareas----------------------------
#define PR_TFT_EVE tskIDLE_PRIORITY+1 //tarea que constrola y enciende el TFT EVE

//RTOS STACK SIZE TASK
#define ST_TFT_EVE ( ( unsigned short ) 256 )  //tarea de control de TFT EVE


void vDisplayTFT_EVE_Task1(void *pvParam);


#endif /* ASD_APP_INCLUDE_RTOS_H_ */
