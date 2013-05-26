
#include "Task.h"



#ifdef __cplusplus
extern "C" {
#endif

void TaskInit(Task *task, integer total1, integer total2)
{
	if (task->Subtask1_Percent > 0)
	{
		task->Subtask1_Step = (integer) (total1 * task->Subtask1_Percent) + 1;
	}
	else
	{
		task->Subtask1_Step = 0;
	}	
	
	if (task->Subtask2_Percent > 0)
	{
		task->Subtask2_Step = (integer) (total2 * task->Subtask2_Percent) + 1;
	}
	else
	{
		task->Subtask2_Step = 0;
	}

	task->Subtask1_First = 0;
	task->Subtask2_First = 0;
	task->IsFinished = False;
	
}	
	
	
float TaskCompletedPercentAfter(const Task *task)
{
	real total1, total2;
	if ((task->Subtask1_Percent > 0) && (task->Subtask2_Percent > 0))
	{
		total1 = task->Subtask1_Step / task->Subtask1_Percent;
		total2 = task->Subtask2_Step / task->Subtask2_Percent;
		return 0; //(((float)task->Subtask1_First * total2 + task->Subtask2_First)*100.0 / (total1 * total2));
		 
	}
	else if (task->Subtask1_Percent > 0)
	{
		total1 = task->Subtask1_Step / task->Subtask1_Percent;		
		return (((float)task->Subtask1_First + task->Subtask1_Step)*100.0 / total1);
	}
}


void TaskIncSubtask1(Task *task)
{
	if (task->Subtask1_Percent > 0)
	{	
		task->Subtask1_First += task->Subtask1_Step;
		real total1 = task->Subtask1_Step / task->Subtask1_Percent;
		if (task->Subtask1_First >= (integer)total1) task->IsFinished = True;
	}	
}

void TaskIncSubtask2(Task *task)
{
	if ( (task->Subtask1_Percent > 0) && (task->Subtask2_Percent > 0) )
	{
		real total1 = task->Subtask1_Step / task->Subtask1_Percent;
		real total2 = task->Subtask2_Step / task->Subtask2_Percent;
		task->Subtask2_First += task->Subtask2_Step;
		//if (task.Subtask2_First >= (integer)total2) 	
		//доделать
	}
	
	
	
}


#ifdef __cplusplus
}
#endif
