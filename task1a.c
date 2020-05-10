/*
    OSC Task1a
    Author: Muyuan Chen  20031527 scymc1
            Pinyuan Feng 20028407 scypf1
*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include "linkedlist.h"
#include "coursework.h"

int main(int argc, char * argv[]) {

    //Setup
    int i;
    long int totalResponseTime = 0;
    long int totalTurnAroundTime = 0;
    struct element *qHead = NULL;
    struct element *qTail = NULL;
    for(i = 0; i < NUMBER_OF_JOBS; i++) {

        //Create processes and add to queue
        struct process * curProcess = (struct process *) malloc (sizeof(struct process));
        curProcess = generateProcess();
        addLast(curProcess, &qHead, &qTail);
    }
    for(i = 0; i< NUMBER_OF_JOBS; i++) {

        //Run each process, FIFO
        struct process * curProcess = (struct process *) malloc (sizeof(struct process));
        curProcess = removeFirst(&qHead, &qTail);
        struct timeval curStartTime;
        struct timeval curEndTime;
        runNonPreemptiveJob(curProcess, &curStartTime, &curEndTime);

        //Calculate response & turn around time 
        long int curResponseTime = getDifferenceInMilliSeconds(curProcess->oTimeCreated, curStartTime);
        totalResponseTime += curResponseTime;
        long int curTurnAroundTime = getDifferenceInMilliSeconds(curProcess->oTimeCreated, curEndTime);
        totalTurnAroundTime += curTurnAroundTime;

        //Print results
        printf("Process Id = %d, ", curProcess->iProcessId);
        printf("Previous Burst Time = %d, ", curProcess->iPreviousBurstTime);
        printf("New Burst Time = %d, ", curProcess->iRemainingBurstTime);
        printf("Response Time = %ld, ", curResponseTime);
        printf("Turn Around Time = %ld\n", curTurnAroundTime);

        //Cleanup
        free(curProcess);
    }
    
    //Continue print results
    double avgResopnseTime = (double) totalResponseTime / NUMBER_OF_JOBS;
    double avgTurnAroundTime = (double) totalTurnAroundTime / NUMBER_OF_JOBS;
    printf("Average response time = %6f\n", avgResopnseTime);
    printf("Average turn around time = %6f\n", avgTurnAroundTime);

}
