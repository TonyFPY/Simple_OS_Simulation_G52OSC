//UoN OSC Coursework 2 Task 5
//Muyuan Chen scymc1@nottingham.ac.uk
//Pinyuan Feng
//Note that only priority boosting has been completed in this code

#include <stdio.h>

#include <stdlib.h>

#include <semaphore.h>

#include <pthread.h>

#include <sys/time.h>

#include <stdbool.h>

#include "coursework.h"

#include "linkedlist.h"

//Create data structure of each level
struct level {
	struct element * qHead; //Job list head
	struct element * qTail; //Job list tail
	sem_t sSync;
	int curTotal;
};

struct timeRecord {
	int iProcessId;
	int iPreviousTime;	
};

//Initialize each level
struct level priorityLevel[MAX_PRIORITY];
struct timeRecord prevTimes[NUMBER_OF_JOBS];

sem_t sLevel;
sem_t sDelayProducer; //Flag to hold producer
sem_t sSyncGlobal; //Flag for global critical section
sem_t sCount; //Flag of jobs in stack

int cProducer = 0; //Count of produced jobs
int cConsumer = 0; //Count of consumed jobs
int curTotalGlobal; //Current jobs in all stacks
int nowTime = 0;

double totalResponseTime = 0;
double totalTurnaroundTime = 0;

void * processor(int consumerID, struct process * curProcess, struct timeval * oStartTime, struct timeval * oEndTime);
void * producer(void * p);
void * consumer(void * p);
bool finder(int level);
bool checker(int cCounter);
void printer();
void printerProducer(struct process * printProcess, int producerID);
void printerConsumer(int flag, struct process * printProcess, int consumerID, int responseTime, int turnaroundTime);
void addTimeRecord(int ProcessId);
void updateTimeRecord(int ProcessId, int recentTime);
int checkTimeRecord(int ProcessId, int currentTime);

int main() {
	pthread_t tProducer[NUMBER_OF_PRODUCERS]; //Producer thread
	pthread_t tConsumer[NUMBER_OF_CONSUMERS]; //Consumer thread

	int producerID[NUMBER_OF_PRODUCERS];
	int consumerID[NUMBER_OF_CONSUMERS];

	sem_init( & sLevel, 0, MAX_BUFFER_SIZE); //Initialize ssemaphore
	sem_init( & sSyncGlobal, 0, 1); //Initialize ssemaphore
	sem_init( & sDelayProducer, 0, 1); //Initialize ssemaphore
	sem_init( & sCount, 0, 0); //Initialize ssemaphore

	for (int i = 0; i < MAX_BUFFER_SIZE; i++) {
		//Setup
		priorityLevel[i].qHead = NULL;
		priorityLevel[i].qTail = NULL;
		priorityLevel[i].curTotal = 0;
		sem_init(&priorityLevel[i].sSync, 0, 1);
	}
	
	for (int i=0; i< NUMBER_OF_JOBS; i++) {
		prevTimes[i].iProcessId = -1;
		//prevTimes[i].iPreviousTime = -1;
	}

	for (int i = 0; i < NUMBER_OF_PRODUCERS; i++) {
			//Setup producers
			producerID[i] = i; //Assign producer ID
	}

	for (int i = 0; i < NUMBER_OF_PRODUCERS; i++) {
			pthread_create( & tProducer[i], NULL, producer, & producerID[i]); //Create producer process with unique producer ID
	}

	for (int i = 0; i < NUMBER_OF_PRODUCERS; i++) {
			pthread_join(tProducer[i], NULL); //Join producer process
	}

	//Cannot be done in a single for loop, don't know why
	for (int i = 0; i < NUMBER_OF_CONSUMERS; i++) {
		//Setup consumers
		consumerID[i] = i;
	}

	for (int i = 0; i < NUMBER_OF_CONSUMERS; i++) {
		pthread_create( & tConsumer[i], NULL, consumer, & consumerID[i]); //Create consumer process with unique consumer ID
	}

	for (int i = 0; i < NUMBER_OF_CONSUMERS; i++) {
		pthread_join(tConsumer[i], NULL); //Join consumer process
	}
	printer();
}

void * producer(void * p) {
	int curProducerID = * ((int*)p);
	//Produce jobs and add to respective stack
	for (int i = 0; i < NUMBER_OF_JOBS; i++) {
		sem_wait( & sLevel);

		//Create new job
		struct process * curProcess = (struct process * ) malloc(sizeof(struct process));
		curProcess = generateProcess();

		//Hold relevant critical section
		sem_wait( & priorityLevel[curProcess -> iPriority].sSync);
		addLast(curProcess, & priorityLevel[curProcess -> iPriority].qHead, & priorityLevel[curProcess -> iPriority].qTail);
		priorityLevel[curProcess -> iPriority].curTotal++;
		addTimeRecord(curProcess->iProcessId);
		printerProducer(curProcess, curProducerID);
		//printf("Produce Total in %d is %d\n", curProcess -> iPriority, priorityLevel[curProcess -> iPriority].curTotal);
		sem_post(&priorityLevel[curProcess -> iPriority].sSync);
	}
	//pthread_exit(NULL);
}

void * consumer(void * p) {
	//Consume jobs from most important, add back if necessary
	struct process * curProcess = (struct process *) malloc(sizeof(struct process));
	int curConsumerID = * ((int*)p);
	bool finish = false;

	while (1) {
		//Always execute, find next available job
		for (int i = 0; i < MAX_PRIORITY; i++) {
			//printf("%d,", i);
			//sem_wait(&priorityLevel[i].sSync);
			if (finder(i)) {
				//Found
				sem_wait(&priorityLevel[i].sSync);
				//Remove job
				curProcess = removeFirst(&priorityLevel[i].qHead, & priorityLevel[i].qTail);
				
				//Decrement counter
				sem_post(&priorityLevel[i].sSync);
				priorityLevel[i].curTotal--;
				
				//Handle it by processor
				struct timeval timeStart, timeEnd;
				processor(curConsumerID, curProcess, & timeStart, & timeEnd);
				sem_post(&priorityLevel[i].sSync);
				break;
			}
			//sem_post(&priorityLevel[i].sSync);

			if (checker(cConsumer)) {
				//Check if all process consumed
				finish = true;
				break;
			}
		}
		if (finish) {
			break;
		}
	}
	//pthread_exit(NULL);
}

bool finder(int level) {
	if (priorityLevel[level].curTotal != 0) {
		return true;
	} else {
		return false;
	}
}

bool checker(int cConsumer) {
	//Check if consumers has finished everything
	sem_wait(&sSyncGlobal);
	if (cConsumer == MAX_NUMBER_OF_JOBS) {
		sem_post(&sSyncGlobal);
		return true;
	} else {
		sem_post(&sSyncGlobal);
		return false;
	}
}

void printer() {
	double avgResopnseTime = (double) totalResponseTime / NUMBER_OF_JOBS;
	double avgTurnaroundTime = (double) totalTurnaroundTime / NUMBER_OF_JOBS;
	printf("Average response time = %6f\n", avgResopnseTime);
	printf("Average turn around time = %6f\n", avgTurnaroundTime);
}

void printerProducer(struct process * printProcess, int producerID) {
	printf("Producer %d, Process Id = %d (%s), Priority = %d, Initial Burst Time = %d\n", producerID, printProcess->iProcessId, printProcess->iPriority < MAX_PRIORITY / 2 ? "FCFS" : "RR", printProcess->iPriority, printProcess->iInitialBurstTime);
}

void printerConsumer(int flag, struct process * printProcess, int consumerID, int responseTime, int turnaroundTime) {
	//Print consumer results
	printf("Consumer %d, Process Id = %d (%s), Priority = %d, Previous Burst Time = %d, Remaining Burst Time = %d", consumerID, printProcess->iProcessId, printProcess -> iPriority < MAX_PRIORITY / 2 ? "FCFS" : "RR", printProcess->iPriority, printProcess->iPreviousBurstTime, printProcess->iRemainingBurstTime);
	if(flag == 1 || flag == 3) {
		printf(", Response Time = %d", responseTime);
	}
	if(flag == 1 || flag == 2) {
		printf(", Turnaround Time = %d", turnaroundTime);
	}
	printf("\n");
	return;
}

void * processor(int consumerID, struct process * curProcess, struct timeval * startTime, struct timeval * endTime) {
	//Call to print consumer result and add process back if necessary
	//Heavily modified from printFunction.c, separated section of printf into printerConsumer
	runJob(curProcess, startTime, endTime);
	int responseTime;
	int turnaroundTime;
	
	if(curProcess->iRemainingBurstTime == 0) {
		//Finished
		if(curProcess -> iPreviousBurstTime == curProcess -> iInitialBurstTime) {
			//New process
			responseTime = getDifferenceInMilliSeconds(curProcess->oTimeCreated, *startTime);
			turnaroundTime = getDifferenceInMilliSeconds(curProcess->oTimeCreated, *endTime);
			sem_wait(&sSyncGlobal);
			totalResponseTime += responseTime;
			totalTurnaroundTime += turnaroundTime;
			sem_post(&sSyncGlobal);
			printerConsumer(1, curProcess, consumerID, responseTime, turnaroundTime);
		}
		else {
			//Existing process
			turnaroundTime = getDifferenceInMilliSeconds(curProcess->oTimeCreated, *endTime);
			sem_wait(&sSyncGlobal);
			totalTurnaroundTime += turnaroundTime;
			sem_post(&sSyncGlobal);
			printerConsumer(2, curProcess, consumerID, 0, turnaroundTime);
		}
		free(curProcess);
		//Increment consumed counter
		sem_wait(&sSyncGlobal);
		cConsumer++;
		sem_post(&sSyncGlobal);
		sem_post(&sLevel);
	}
	else {
		//Not finished
		if (curProcess -> iPreviousBurstTime == curProcess -> iInitialBurstTime) {
			//New process
			responseTime = getDifferenceInMilliSeconds(curProcess->oTimeCreated, *startTime);
			sem_wait(&sSyncGlobal);
			totalResponseTime += responseTime;
			sem_post(&sSyncGlobal);
			printerConsumer(3, curProcess, consumerID, responseTime, 0);
			
			//Add back process
			sem_wait(&priorityLevel[curProcess->iPriority].sSync);
			addLast(curProcess, &priorityLevel[curProcess->iPriority].qHead, &priorityLevel[curProcess->iPriority].qTail);
			priorityLevel[curProcess->iPriority].curTotal++;
			sem_post(&priorityLevel[curProcess->iPriority].sSync);
		}
		else {
			//Existing process, considering boost
			printerConsumer(4, curProcess, consumerID, 0, 0);
			nowTime = getDifferenceInMilliSeconds(curProcess->oTimeCreated, *startTime);
			int timeDifference = checkTimeRecord(curProcess->iProcessId, nowTime);
			if(timeDifference > BOOST_INTERVAL) {
				printf("Boost priority: Process Id = %d, Priority = %d, New Priority = %d\n", curProcess->iProcessId, curProcess->iPriority, MAX_PRIORITY/2);
				//Add back process, BOOST
				sem_wait(&priorityLevel[MAX_PRIORITY/2].sSync);
				curProcess->iPriority = MAX_PRIORITY/2;
				addFirst(curProcess, &priorityLevel[curProcess->iPriority].qHead, &priorityLevel[curProcess->iPriority].qTail);
				priorityLevel[curProcess->iPriority].curTotal++;
				sem_post(&priorityLevel[MAX_PRIORITY/2].sSync);
			}
			else {
				//Add back process
				sem_wait(&priorityLevel[curProcess->iPriority].sSync);
				addLast(curProcess, &priorityLevel[curProcess->iPriority].qHead, &priorityLevel[curProcess->iPriority].qTail);
				priorityLevel[curProcess->iPriority].curTotal++;
				sem_post(&priorityLevel[curProcess->iPriority].sSync);
			}
		}
	}
	updateTimeRecord(curProcess->iProcessId, getDifferenceInMilliSeconds(curProcess->oTimeCreated, *endTime));
}

void addTimeRecord(int ProcessId) {
	for(int i=0; i<NUMBER_OF_JOBS; i++) {
		if(prevTimes[i].iProcessId == -1) {
			prevTimes[i].iProcessId = ProcessId;
			prevTimes[i].iPreviousTime = 0;
			break;
		}
	}
}

void updateTimeRecord(int ProcessId, int recentTime) { //TA time here
	for(int i=0; i<NUMBER_OF_JOBS; i++) {
		if(prevTimes[i].iProcessId == ProcessId) {
			prevTimes[i].iPreviousTime = recentTime;
			break;
		}
	}
}

int checkTimeRecord(int ProcessId, int currentTime) {
	for(int i=0; i<NUMBER_OF_JOBS; i++) {
		if(prevTimes[i].iProcessId == ProcessId) {
			int returnVal = currentTime - prevTimes[i].iPreviousTime;
			//printf("DIFF: %d\n", returnVal);
			return returnVal;
		}
	}
}