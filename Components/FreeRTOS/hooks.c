#include "FreeRTOS.h"
#include "task.h"
#include "croutine.h"

#include <stdio.h>

void vApplicationIdleHook( void )
{
	printf("idle\n");
	/* Schedule the co-routines from within the idle task hook. */
	vCoRoutineSchedule();
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{
	( void ) pcTaskName;
	( void ) pxTask;

	/* Run time stack overflow checking is performed if
	configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
	function is called if a stack overflow is detected. */
	taskDISABLE_INTERRUPTS();

	printf("FATAL: Task Stack Overflow: handle=%p, name=`%s'\n", pxTask, pcTaskName);

	for( ;; );
}