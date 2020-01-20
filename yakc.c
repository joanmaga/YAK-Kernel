#include "yakk.h"
#include "yaku.h"
#include "clib.h"

#define FIRST_DISPATCH 0
#define ISR_DISPATCH 1
#define BLOCK_DISPATCH 2

TCB tcb_array[MAX_NUMBER_OF_TASKS + 1];
int next_available_tcb = 0;
YKSEM sem_array[MAX_NUMBER_OF_SEMAPHORES];
int next_available_semaphore = 0;
YKQ queue_array[MAX_NUMBER_OF_QUEUES];
int next_available_queue = 0;
YKEVENT event_array[MAX_NUMBER_OF_EVENTS];
int next_available_event = 0;

int debug = 0;

TCBptr YKList;
TCBptr running_task;
TCBptr old_task;
int YK_running = 0;
int YKCtxSwCount;
int YKIdleCount;
int YKTickNum;
int YK_Depth;
int FirstTime;
int IdleTskStk[IDLE_TASK_SIZE];

void disable_interrupts();
void enable_interrupts();
void save_context();
void restore_context();

extern void YKDispHandler();
extern void YKFirst();
extern void YKISR();
extern void YKSecond();

void YKInitialize(void)
{
	YKNewTask(YKIdleTask, (void *)&IdleTskStk[IDLE_TASK_SIZE], IDLE_TASK_PRIORITY);
	YKCtxSwCount = 0;
	YKIdleCount = 0;
	YKTickNum = 1;
	YK_Depth = 0;
	FirstTime = 1;
}

void YKEnterMutex(void) {
	disable_interrupts();
}

void YKExitMutex(void) {
	enable_interrupts();
}

void YKIdleTask(void)
{
	while (1)
	{
		++YKIdleCount;
		--YKIdleCount;
		++YKIdleCount;
		//read disassemble and ensure there are at least four instruction here
	}
}

void YKNewTask(void(*task)(void), void *taskStack, unsigned char priority)
{
	int index;
	
	TCBptr tempHead;
	//YKEnterMutex();		//BOOLEAN VALUE BOOK
	index = next_available_tcb;
	tempHead = YKList;
	tcb_array[index].delay_counter = 0;
	tcb_array[index].id = next_available_tcb;
	tcb_array[index].priority = priority;
	tcb_array[index].task = (void*)task;
	tcb_array[index].pInst = (void*)(task);
	tcb_array[index].pStck = (int*)(taskStack);
	tcb_array[index].state = READY;
	tcb_array[index].firstTime = 1;
	tcb_array[index].event_wait_type = 0;
	tcb_array[index].event_mask = 0;
	next_available_tcb++;

	if (index == 0) YKList = &(tcb_array[index]);

	while (tempHead != NULL) {
		if (priority < tempHead->priority) {	//LOCATION FOR NEW TASK IS FOUND

			if (YKList == tempHead) {		//SPECIAL CASE TO MODIFY HEAD
				YKList->prev = &(tcb_array[index]);
				tcb_array[index].next = YKList;
				YKList = &(tcb_array[index]);
			}
			else {
				tcb_array[index].next = tempHead;
				tcb_array[index].prev = tempHead->prev;
				tempHead->prev->next = &(tcb_array[index]);
				tempHead->prev = &(tcb_array[index]);
			}
			tempHead = NULL;
			//break;
		}
		else {
			tempHead = tempHead->next;
		}
	}
	//YKExitMutex();
	if (YK_running) {
		YKScheduler(BLOCK_DISPATCH);
	}

	//MAYBE:EXIT MUTEX
}

void YKRun(void)
{
	if (YKList != &(tcb_array[0])) { //If idle task is not at the head of list (ie there are user-defined tasks)
		YK_running = 1;
		YKScheduler(FIRST_DISPATCH);
	}
}

void YKScheduler(int dispatcher_type)        //Determines the highest priority ready taskvoid YKScheduler(void)
{
	TCBptr currentTCB;
	YKEnterMutex();
	currentTCB = YKList;
	while (currentTCB != NULL) {
		if (currentTCB->state == RUNNING) {
																	
			break;
		}
		else if (currentTCB->state == READY) {
														
			YKDispatcher(currentTCB, dispatcher_type);
			break;
		}
														
		currentTCB = currentTCB->next;
	}
	YKExitMutex();
}

void YKDispatcher(TCBptr task, int dispatcher_type)
{
	int bool_return = 0;
	int firstForTask = task->firstTime;
	int isFirst = FirstTime;

	old_task = running_task;

	if(FirstTime) {		//FIRST TIME
		task->firstTime = 0;
		FirstTime = 0;
		++YKCtxSwCount;
		running_task = task;
		//bool_return = 2;save_context
	}
	else if (running_task->state == BLOCKED) {
		running_task = task;	//SETS NEW RUNNING TASK
		if (!(task->firstTime)) {
			task->firstTime = 0;
			//++YKCtxSwCount;
			//bool_return = 2;
			//bool_return = 1;
		}
		else {
			task->firstTime = 0;
			task->state = RUNNING;
			++YKCtxSwCount;
			//bool_return = 3;
		}
	}
	else {	
		task->firstTime = 0;
		++YKCtxSwCount;
		old_task->state = READY;
		task->state = RUNNING;
		running_task = task;
		//bool_return = 3;
	}

	//if (enable) enable_interrupts();
	if (isFirst || dispatcher_type == FIRST_DISPATCH) {
		YKFirst();
	}
	else if (dispatcher_type == BLOCK_DISPATCH) {
		if(firstForTask){			
				YKSecond();
		}
		else {
			YKDispHandler(); 
		}
	}
	else {
		YKISR();
	}
}

void YKDelayTask(unsigned count)
{
	//MAYBE: SAVE CONTEXT
	if (count == 0) return;
	YKEnterMutex();
	running_task->delay_counter = count;
	running_task->state = BLOCKED;			//BLOCKS DELAYED TASK
	YKExitMutex();
	//MAYBE: SAVE CONTEXT										//PRINT STRING HERE
	YKScheduler(BLOCK_DISPATCH);
	//MAYBE: RESTORE CONTEXT
	//CALLS SPECIAL SCHEDULER AND SAVES CONTEXT
	//MAYBE: RESTORE CONTEXT AND GO BACK TO FUNCTION
}

void YKEnterISR()
{
	++YK_Depth;
}

void YKExitISR()
{
	--YK_Depth;
	if (YK_Depth == 0){ 
		YKScheduler(ISR_DISPATCH);
	}
}

//CALLED BY TICK ISR
//May also call a user tick handler if the user code requires actions to be taken on each clock tick
void YKTickHandler()
{
	TCBptr currentTCB = YKList;
	YKEnterMutex();
	++YKTickNum;
	while (currentTCB != NULL) {

		if (currentTCB->task == YKIdleTask) break;	//MIGHT NEED TO CHANGE THIS IN THE FUTURE. 
											//BUT IDLE TASK SHOULDN'T HAVE A DELAY RIGHT NOW
		if (currentTCB->delay_counter != 0) {
			--(currentTCB->delay_counter);
			if (currentTCB->delay_counter == 0) {
				currentTCB->state = READY;
			}
		}
		currentTCB = currentTCB->next;
	}
	YKExitMutex();
}

YKSEM* YKSemCreate(int initialValue) {
    YKSEM* sem_ptr = &(sem_array[next_available_semaphore]);
	next_available_semaphore++;
	sem_ptr->value = initialValue;
	sem_ptr->number_of_waiting_tasks = 0;
	return sem_ptr;
}

void YKSemPend(YKSEM *semaphore) {
	TCBptr current_tcb;     
	YKEnterMutex();
	(semaphore->value)--;
    if (semaphore->value >= 0) {
		YKExitMutex();
    	return;
    }
    else {
																		
    	running_task->state = BLOCKED;
		semaphore->waiting_task_list[semaphore->number_of_waiting_tasks] = running_task;
		semaphore->number_of_waiting_tasks++;
		YKScheduler(BLOCK_DISPATCH);
    }
	YKExitMutex();
}

void YKSemPost(YKSEM *semaphore) {
	int i;
	int highest_priority = IDLE_TASK_PRIORITY + 1;
	int index_of_highest_priority_task;
	
	YKEnterMutex();		
	(semaphore->value)++;

	if (semaphore->number_of_waiting_tasks > 0) {
		for (i = 0; i < semaphore->number_of_waiting_tasks; i++) {
			if (semaphore->waiting_task_list[i]->priority < highest_priority) {
				highest_priority = semaphore->waiting_task_list[i]->priority;
				index_of_highest_priority_task = i;
			}
		}
																				
		semaphore->waiting_task_list[index_of_highest_priority_task]->state = READY;
		for (i = 0; i + index_of_highest_priority_task < semaphore->number_of_waiting_tasks; i++) { //scoot the members of the array to the left to fill the void
			semaphore->waiting_task_list[index_of_highest_priority_task + i] = semaphore->waiting_task_list[index_of_highest_priority_task + i + 1];
		}
		semaphore->number_of_waiting_tasks--;
	}

    if (YK_Depth == 0) { //If called from task code
		YKScheduler(BLOCK_DISPATCH);
    }
	YKExitMutex();
}

YKQ *YKQCreate(void **start, unsigned size)
{

	YKQ* q_ptr = &(queue_array[next_available_queue]);
	next_available_queue++;
	q_ptr->q_array = start;
	q_ptr->number_of_messages = 0;
	q_ptr->indexIn = 0;
	q_ptr->indexOut = 0;
	q_ptr->size = size;
	q_ptr->number_of_waiting_tasks = 0;
	return q_ptr;
}

void *YKQPend(YKQ *queue)
{
	
	TCBptr current_tcb;   
	void* return_message;  
	YKEnterMutex();
    if (queue->number_of_messages == 0) {
		running_task->state = BLOCKED;
		queue->waiting_task_list[queue->number_of_waiting_tasks] = running_task;
		queue->number_of_waiting_tasks++;
		YKExitMutex(); 	//CRAZY STUFF, THEN DELETE THAT
		YKScheduler(BLOCK_DISPATCH);
    }
	
	return_message = queue->q_array[queue->indexOut];
	queue->number_of_messages--;
	if ((queue->size - 1) == queue->indexOut){
		queue->indexOut = 0;
	}
	else{
		queue->indexOut++;
	}
	
	YKExitMutex();
	return return_message;

}

int YKQPost(YKQ *queue, void *msg)
{
	int i;
	int highest_priority = IDLE_TASK_PRIORITY + 1;
	int index_of_highest_priority_task;
	int returned_value = 0;
	
	YKEnterMutex();
	if(queue->size == queue->number_of_messages){
		//queue->indexIn = 0;
		YKExitMutex();	//CRAZY STUFF, THEN DELETE THAT
		return returned_value;
	}

	//PUSH MESSAGE
	queue->q_array[queue->indexIn] = msg;
	queue->number_of_messages++;

	if((queue->size - 1) == queue->indexIn){
		queue->indexIn = 0;
	}
	else{
		queue->indexIn++;
	}
	
	
	if (queue->number_of_waiting_tasks > 0) {
		
		
		for (i = 0; i < queue->number_of_waiting_tasks; i++) {
			
			if (queue->waiting_task_list[i]->priority < highest_priority) {
				highest_priority = queue->waiting_task_list[i]->priority;
				index_of_highest_priority_task = i;
			}
		}
																				
		queue->waiting_task_list[index_of_highest_priority_task]->state = READY;
		for (i = 0; i + index_of_highest_priority_task < queue->number_of_waiting_tasks; i++) { //scoot the members of the array to the left to fill the void
			//***THERE MIGHT BE BUGS HERE***
			
		}
		queue->number_of_waiting_tasks--;

		
		
	}
	
	YKExitMutex();
    if (YK_Depth == 0) { //If called from task code
		YKScheduler(BLOCK_DISPATCH);
    }
	
	returned_value = 1;
	return returned_value;
}

YKEVENT *YKEventCreate(unsigned initialValue)
{
	YKEVENT* event_ptr = &(event_array[next_available_event]);
	next_available_event++;
	event_ptr->value = initialValue;
	event_ptr->number_of_waiting_tasks = 0;
	return event_ptr;
}

unsigned YKEventPend(YKEVENT *event, unsigned eventMask, int waitMode)
{
	
	YKEnterMutex();
	if (waitMode == EVENT_WAIT_ALL) {
		if (eventMask == event->value & eventMask) {
			YKExitMutex();
			return event->value;	
		}
	}
	else{
		if (event->value & eventMask) {
			YKExitMutex();
			return event->value;
		}
	}
	
	event->waiting_task_list[event->number_of_waiting_tasks] = running_task;
	event->waiting_task_list[event->number_of_waiting_tasks]->event_mask = eventMask;
	event->waiting_task_list[event->number_of_waiting_tasks]->event_wait_type = waitMode;
	event->number_of_waiting_tasks++;
	running_task->state = BLOCKED;			//BLOCKS DELAYED TASK
	YKExitMutex();
										//PRINT STRING HERE
	YKScheduler(BLOCK_DISPATCH);

	return event->value;
}


void YKEventSet(YKEVENT *event, unsigned eventMask)
{
	int i;
	int ready_tasks_removed = 0;
		
	YKEnterMutex();
	event->value |= eventMask;
	
	if (event->number_of_waiting_tasks > 0) {
		for (i = 0; i < event->number_of_waiting_tasks; i++) {
			if (event->waiting_task_list[i]->event_wait_type == EVENT_WAIT_ALL) {
				if (event->waiting_task_list[i]->event_mask == (event->value & event->waiting_task_list[i]->event_mask)) {
					event->waiting_task_list[i]->state = READY;
					//printString("SET ALL EVENT ");
					//printInt(event->waiting_task_list[i]->id);
					//printString(" to ready\n");
				}
			}
			else{
				if (event->value & event->waiting_task_list[i]->event_mask) {
					event->waiting_task_list[i]->state = READY;
					//printString("SET ANY EVENT ");
					//printInt(event->waiting_task_list[i]->id);
					//printString(" to ready\n");
				}
			}
		}
		
		
		for(i = 0; i < event->number_of_waiting_tasks;i++) {
			if (event->waiting_task_list[i]->state != READY) {
				event->waiting_task_list[i-ready_tasks_removed] = event->waiting_task_list[i];
			}
			else {
				ready_tasks_removed++;
			}
		}
			

		event->number_of_waiting_tasks = event->number_of_waiting_tasks - ready_tasks_removed;
		
		
	}
	
	
	YKExitMutex();

	if (YK_Depth == 0) { //If called from task code
		YKScheduler(BLOCK_DISPATCH);
    }
}


void YKEventReset(YKEVENT *event, unsigned eventMask)
{
	event->value &= ~eventMask;
}




