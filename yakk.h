#ifndef yakk
#define yakk

#include <stdint.h>
#include "yaku.h"

#define NULL 0
#define IDLE_TASK_SIZE 256
#define IDLE_TASK_PRIORITY 100
#define EVENT_WAIT_ALL 1
#define EVENT_WAIT_ANY 0

extern int YK_running;
extern int YKCtxSwCount;
extern int YKIdleCount;
extern int YKTickNum;
extern int YK_Depth;
extern int FirstTime;

enum State {
	BLOCKED,
	READY,
	RUNNING,
};

//typedef void func(void);
struct taskblock;
typedef struct taskblock* TCBptr;

typedef struct semaphore
{
	int value;
	int number_of_waiting_tasks;
 	TCBptr waiting_task_list[MAX_NUMBER_OF_TASKS];
} YKSEM;

typedef struct event
{
	unsigned value;									//EVENT BITS
	int number_of_waiting_tasks;
 	TCBptr waiting_task_list[MAX_NUMBER_OF_TASKS];
} YKEVENT;

typedef struct taskblock
{
	int* pStck; 			//0
	void(*pInst);			//2
	int id;					//4	
	int priority;			//6
	enum State    state;
	char		context;
	int	delay_counter;
	TCBptr	prev; //points to next higher priority task
	TCBptr next; //Points to next lower priority task
	//CONTEXT
	void(*task)(void);	//Beginning of Function
	int firstTime;
	char event_wait_type;
	unsigned event_mask;
} TCB;

typedef struct queue
{
	void** q_array;	//MAYBE INITIALIZE VALUE
	int number_of_messages;
	int indexIn;
	int indexOut;
	unsigned size;
	int number_of_waiting_tasks;
 	TCBptr waiting_task_list[MAX_NUMBER_OF_TASKS];
} YKQ;

extern TCB tcb_array[MAX_NUMBER_OF_TASKS + 1];
extern int next_available_tcb;

extern YKSEM sem_array[MAX_NUMBER_OF_SEMAPHORES];
extern int next_available_semaphore;

extern YKQ queue_array[MAX_NUMBER_OF_QUEUES];
extern int next_available_queue;	//DANGER

extern YKEVENT event_array[MAX_NUMBER_OF_EVENTS];
extern int next_available_event;

extern TCBptr YKList;
extern TCBptr running_task;
extern TCBptr old_task;

extern void YKDispHandler();

void YKInitialize(void);       // Initializes all required kernel data structures
void YKEnterMutex(void);       //Disables interrupts
void YKExitMutex(void);        //Enables interrupts
void YKIdleTask(void);         //Kernel's idle task
void YKNewTask(void(*task)(void), void *taskStack, unsigned char priority);  //Creates a new task
void YKRun(void);              //Starts actual execution of user code
void YKScheduler(int dispatcher_type);         //Determines the highest priority ready task
void YKDispatcher(TCBptr task, int save_context);       //Begins or resumes execution of the next task
void YKDelayTask(unsigned count);
void YKEnterISR(void);
void YKExitISR(void);
void YKTickHandler(void);
YKSEM* YKSemCreate(int initialValue);
void YKSemPend(YKSEM *semaphore);
void YKSemPost(YKSEM *semaphore);
YKQ *YKQCreate(void **start, unsigned size);
void *YKQPend(YKQ *queue);
int YKQPost(YKQ *queue, void *msg);
YKEVENT *YKEventCreate(unsigned initialValue);
unsigned YKEventPend(YKEVENT *event, unsigned eventMask, int waitMode);
void YKEventSet(YKEVENT *event, unsigned eventMask);
void YKEventReset(YKEVENT *event, unsigned eventMask);





#endif
