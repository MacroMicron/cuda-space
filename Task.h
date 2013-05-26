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
	//percent for implement
	float 		Subtask1_Percent;
	float		Subtask2_Percent;

	//private read-only
	integer 	Subtask1_First;
	integer		Subtask1_Step;
	integer 	Subtask2_First;
	integer		Subtask2_Step;	
	
	//result
	Bool		IsFinished;
	
} Task;

void TaskInit(Task *task, integer total1, integer total2);
float TaskCompletedPercentAfter(const Task *task);
void TaskIncSubtask1(Task *task);
void TaskIncSubtask2(Task *task);

#ifdef __cplusplus
}
#endif

#endif
