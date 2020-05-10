/*
    OSC Task4
    Author: Muyuan Chen  20031527 scymc1
            Pinyuan Feng 20028407 scypf1
*/
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>
#include "coursework.h"
#include "linkedlist.h"

typedef struct element Element;
typedef struct process Process;
typedef struct timeval Timeval;

// declaration
sem_t sync, full, empty;
int produced, consumed;
Element *qHead[MAX_PRIORITY];
Element *qTail[MAX_PRIORITY];
long int totalResponseTime;
long int totalTurnAroundTime;

void * consumer(void *ID);
void * producer(void *ID);
void addJob(int P_ID);
Process* removeJob(int C_ID);
void printTime();
void printCurProducer(int P_ID, Process * curProcess);
Process * processJob(int iConsumerId, struct process * pProcess, struct timeval oStartTime, struct timeval oEndTime);

int main(int argc, char * agrv[]){
    // initialization
    pthread_t prod[NUMBER_OF_PRODUCERS], cons[NUMBER_OF_CONSUMERS];   // create multiple threads 
    int P_IP[NUMBER_OF_PRODUCERS], C_ID[NUMBER_OF_CONSUMERS];         // use arrays to store producer and consumers' IDs.
    int s_sync, s_full, s_empty;                                      // declare value of semaphores
    sem_init(&sync, 0, 1);                                            // initialize binary semaphore
    sem_init(&empty, 0, MAX_BUFFER_SIZE);                             // initialize empty semaphore to record the number of empty places in the buffer
    sem_init(&full, 0, 0);                                            // initialize full semaphore to record the number of filled places

    // create jobs
    for(int i = 0; i < NUMBER_OF_PRODUCERS; i++){
        P_IP[i] = i + 1;
        pthread_create(&prod[i], NULL, producer, (P_IP + i)); 
    }

    for(int i = 0; i < NUMBER_OF_CONSUMERS; i++){
        C_ID[i] = i + 1;
        pthread_create(&cons[i], NULL, consumer, (C_ID + i));
    }

    // join jobs
    for(int i = 0; i < NUMBER_OF_PRODUCERS; i++){
        pthread_join(prod[i], NULL);
    }

    for(int i = 0; i < NUMBER_OF_CONSUMERS; i++){
        pthread_join(cons[i], NULL);   
    }

    /*
    // print out the values of semaphores
    sem_getvalue(&sync, &s_sync);           
    sem_getvalue(&empty, &s_empty);  
    sem_getvalue(&full, &s_full);         
    printf("sync = %d, empty = %d, full = %d\n", s_sync, s_empty, s_full);
    */

    // print result
    printTime();
    
    // destroy the semaphore in the end of the program
    sem_destroy(&sync);
    sem_destroy(&empty);
    sem_destroy(&full);

    return 0;
}

void * producer(void * ID){
    while(produced < NUMBER_OF_JOBS){
        sem_wait(&empty);
        sem_wait(&sync);
        
        // add an item to the buffer
        addJob(*((int*)ID));
        produced++;

        sem_post(&sync);
        sem_post(&full);

    }
    return ((void *)0);
}

void * consumer(void * ID){
    int result;
    int temp = 0;
    Timeval curStartTime;
    Timeval curEndTime;
    while(1){
        sem_wait(&full);
        sem_wait(&sync);

        // remove an item from the buffer according their priorities
        // this is run in the critical section
        Process * curProcess = removeJob(*((int*)ID));

        sem_post(&sync);

        // run jobs and print the information
        runJob(curProcess, &curStartTime, &curEndTime);
        Process * tempP = processJob(*((int*)ID), curProcess, curStartTime, curEndTime);
        
        sem_wait(&sync);
        if(tempP != NULL){
            // if job is not consumed entirely, add back to the buffer
            sem_post(&full);
            addLast(tempP, &(qHead[tempP -> iPriority]), &(qTail[tempP -> iPriority]));
        } else {
            // if job is consumed, number of consumed job is increased
            // and is assigned to temp
            sem_post(&empty);
            temp = ++ consumed;
        }
        sem_post(&sync);
        
        // let the consumers to quit one by one
        if (temp > NUMBER_OF_JOBS - NUMBER_OF_CONSUMERS)
        {
            return ((void *)0);
        }
    }
    
}

/*
 * The function is used by producer to add jobs;
 * The current status will be printed.
 */
void addJob(int P_ID){
    Process * curProcess = generateProcess();
    addLast(curProcess, &(qHead[curProcess -> iPriority]), &(qTail[curProcess -> iPriority]));

    // print information of Producer
    printCurProducer(P_ID, curProcess);

    return;
}

/*
 * The function is used by consumer with corresponding consumer ID.
 * if the job is not totally consumed in RR fashion, it will return 1; Otherwise it will rturn 0;
 */
Process* removeJob(int C_ID){
    for(int i = 0; i < MAX_PRIORITY; i++){
        if(qHead[i] != NULL){
          Process * curProcess = removeFirst(&(qHead[i]), &(qTail[i]));
          return curProcess;
        }
    }
    return NULL;
}

/*
 * The function is used to display the average response time and average turn around time of jobs.
 */
void printTime(){
    double avgResopnseTime = (double) totalResponseTime / NUMBER_OF_JOBS;
    double avgTurnAroundTime = (double) totalTurnAroundTime / NUMBER_OF_JOBS;
    printf("Average Response Time = %6f\n", avgResopnseTime);
    printf("Average Turn Around Time = %6f\n", avgTurnAroundTime);

    return;
}

/*
 * The function is used to display the current status of producer and consumer
 */
void printCurProducer(int P_ID, Process * curProcess){
    printf("Producer %d, ", P_ID);
    printf("Process Id = %d ", curProcess -> iProcessId);
    printf("(%s), ", curProcess -> iPriority < MAX_PRIORITY / 2 ? "FCFS" : "RR");
    printf("Priority = %d, ", curProcess -> iPriority);
    printf("Initial Burst Time %d\n", curProcess -> iInitialBurstTime);
    return;
}

/*
 * Prints output on the screen depending on the type of process (FCFS or RR) and calculates the turn-around/response times.
 * If the job has not finished completely, it is returned by the function, NULL otherwise.
 */
Process * processJob(int iConsumerId, struct process * pProcess, struct timeval oStartTime, struct timeval oEndTime)
{
	int iResponseTime;
	int iTurnAroundTime;
	if(pProcess->iPreviousBurstTime == pProcess->iInitialBurstTime && pProcess->iRemainingBurstTime > 0)
	{
		iResponseTime = getDifferenceInMilliSeconds(pProcess->oTimeCreated, oStartTime);	
		totalResponseTime += iResponseTime;
		printf("Consumer %d, Process Id = %d (%s), Priority = %d, Previous Burst Time = %d, Remaining Burst Time = %d, Response Time = %d\n", iConsumerId, pProcess->iProcessId, pProcess->iPriority < MAX_PRIORITY / 2	 ? "FCFS" : "RR",pProcess->iPriority, pProcess->iPreviousBurstTime, pProcess->iRemainingBurstTime, iResponseTime);
		return pProcess;
	} else if(pProcess->iPreviousBurstTime == pProcess->iInitialBurstTime && pProcess->iRemainingBurstTime == 0)
	{
		iResponseTime = getDifferenceInMilliSeconds(pProcess->oTimeCreated, oStartTime);	
		totalResponseTime += iResponseTime;
		iTurnAroundTime = getDifferenceInMilliSeconds(pProcess->oTimeCreated, oEndTime);
		totalTurnAroundTime += iTurnAroundTime;
		printf("Consumer %d, Process Id = %d (%s), Priority = %d, Previous Burst Time = %d, Remaining Burst Time = %d, Response Time = %d, Turnaround Time = %d\n", iConsumerId, pProcess->iProcessId, pProcess->iPriority < MAX_PRIORITY / 2 ? "FCFS" : "RR", pProcess->iPriority, pProcess->iPreviousBurstTime, pProcess->iRemainingBurstTime, iResponseTime, iTurnAroundTime);
		free(pProcess);
		return NULL;
	} else if(pProcess->iPreviousBurstTime != pProcess->iInitialBurstTime && pProcess->iRemainingBurstTime > 0)
	{
		printf("Consumer %d, Process Id = %d (%s), Priority = %d, Previous Burst Time = %d, Remaining Burst Time = %d\n", iConsumerId, pProcess->iProcessId, pProcess->iPriority < MAX_PRIORITY / 2 ? "FCFS" : "RR", pProcess->iPriority, pProcess->iPreviousBurstTime, pProcess->iRemainingBurstTime);
		return pProcess;
	} else if(pProcess->iPreviousBurstTime != pProcess->iInitialBurstTime && pProcess->iRemainingBurstTime == 0)
	{
		iTurnAroundTime = getDifferenceInMilliSeconds(pProcess->oTimeCreated, oEndTime);
		totalTurnAroundTime += iTurnAroundTime;
		printf("Consumer %d, Process Id = %d (%s), Priority = %d, Previous Burst Time = %d, Remaining Burst Time = %d, Turnaround Time = %d\n", iConsumerId, pProcess->iProcessId, pProcess->iPriority < MAX_PRIORITY / 2 ? "FCFS" : "RR", pProcess->iPriority, pProcess->iPreviousBurstTime, pProcess->iRemainingBurstTime, iTurnAroundTime);
		free(pProcess);
		return NULL;
	}
}