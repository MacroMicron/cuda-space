/* This header implement tasks for wide countings.*/

#include "UserSettings.h"
#include "Bool.h"


#ifndef _TASK_
#define _TASK_

#ifdef __cplusplus
extern "C" {
#endif

//2 subtasks only in one task (maximum)
typedef struct
{
	//private fields
	float 		Percent;
	integer 	Step;
	integer 	Subtask1_First;
	integer		Subtask1_Total;
	integer 	Subtask2_First;
	integer		Subtask2_Total;
	
	//result
	Bool		IsFinished;
	
} Task;

void TaskInit(Task *task, float percent, integer total1, integer total2);
float TaskCompleted(const Task *task);
void TaskInc(Task *task);

#ifdef __cplusplus
}
#endif

#endif
