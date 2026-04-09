/*
 * RTOS.c
 *
 *  Created on: Nov 26, 2025
 *      Author: desarrollo_1

 aqui se deben PONER TODAS LAS TAREAS QUE SE CREAN SOLAMENTE
 */
#include "FreeRTOS.h"  // DEBE ir primero
#include "task.h"
#include "queue.h"
#include <stdio.h>
#include "system.h"



void vDisplayTFT_EVE_Task1(void *pvParam){

  for(;;){
	printf(COLOR_YELLOW"%s"COLOR_RESET, "Estoy en tarea EVE!\n");
     }

}//fin display TFT EVE TASK1-------------------------------
