
#include "Task.h"



#ifdef __cplusplus
extern "C" {
#endif

void TaskInit(Task *task, float percent, integer total1, integer total2)
{
	task->Subtask1_Total = task->Subtask2_Total = 0;
	task->Step = 0;
	task->Subtask1_First = task->Subtask2_First = 0;
	task->IsFinished = True;
	task->Percent = 0;
	if ((percent > 0)&&(total1 >= 0)&&(total2 >= 0))
	{
		task->Percent = percent;
		task->Subtask1_Total = total1;
		task->Subtask2_Total = total2 ? total2 : 1;
		task->Step = (integer)(percent * task->Subtask1_Total * task->Subtask2_Total) + 1;
		task->IsFinished = False;
	}
}	
	
	
float TaskCompleted(const Task *task)
{
	if (task->Subtask1_Total > 0)
	{		
		return (float)(task->Subtask1_First * task->Subtask2_Total + task->Subtask2_First) / (task->Subtask1_Total * task->Subtask2_Total) * 100;
	}
	else return 0;
}


void TaskInc(Task *task)
{	
	task->Subtask1_First = (task->Subtask1_First * task->Subtask2_Total + task->Subtask2_First + task->Step) / task->Subtask2_Total;
	task->Subtask2_First = (task->Subtask1_First * task->Subtask2_Total + task->Subtask2_First + task->Step) % task->Subtask2_Total;
	if (task->Subtask1_First * task->Subtask2_Total + task->Subtask2_First >= task->Subtask1_Total * task->Subtask2_Total) task->IsFinished = True;	
}




#ifdef __cplusplus
}
#endif
