/*
    OSC Task1b
    Author: Muyuan Chen  20031527 scymc1
            Pinyuan Feng 20028407 scypf1
*/
#include <stdio.h>
#include <stdlib.h>
#include "coursework.h"
#include "linkedlist.h"
#include <sys/time.h>

typedef struct process Process;
typedef struct element Element;
typedef struct timeval Timeval;

int main(int argc, char* agrv[]){
    //initialization
    Element *pHead = NULL;
    Element *pTail = NULL;
    Timeval curStartTime;
    Timeval curEndTime;

    long int response_time = 0;          // the response time of one process
    long int total_response_time = 0;    // the total reponse time till now
    long int turnaround_time = 0;        // the turn around time of one process
    long int total_turnaround_time = 0;  // the total turn around time till now
    
    // initialize the process and print the initial process list
    printf("PROCESS LIST:\n");
    for(int i = 0; i < NUMBER_OF_JOBS; i++){
        Process *p = generateProcess();
        addLast(p, &pHead, &pTail);
        printf("\t ");
        printf(
            "Process Id = %d, "
            "Priority = %d, "
            "Initial Burst Time = %d, "
            "Remaining Burst Time = %d\n",
            p -> iProcessId, p -> iPriority, p -> iInitialBurstTime, p -> iRemainingBurstTime
        );
    }
    printf("END\n\n");
    
    int count = 0;
    while(pHead != NULL){
        // choose the first process from the linked list and run it
        Process *curP = removeFirst(&pHead, &pTail);
        runPreemptiveJob(curP, &curStartTime, &curEndTime);
        
        printf(
            "Process Id = %d, "
            "Priority = %d, "
            "Previous Burst Time = %d, "
            "New Burst Time = %d",
            curP -> iProcessId, curP -> iPriority, curP -> iPreviousBurstTime, curP -> iRemainingBurstTime
        );
        
        // print each process's response time
        if(count < NUMBER_OF_JOBS){
            response_time = getDifferenceInMilliSeconds(curP -> oTimeCreated, curStartTime);
            total_response_time += response_time;
            count ++;
            printf(", Response Time = %ld", response_time);
        }
        
        // check whether the process finishes running
        if(curP -> iRemainingBurstTime == 0){
            turnaround_time = getDifferenceInMilliSeconds(curP -> oTimeCreated, curEndTime);
            total_turnaround_time += turnaround_time;
            printf(", Turnaround Time = %ld", turnaround_time);
            free(curP);
        } else {
            addLast(curP, &pHead, &pTail);
        }

        printf("\n");

    }

    printf("Average response time = %6f\n",(total_response_time/(double)NUMBER_OF_JOBS));
    printf("Average turn around time = %6f\n", (total_turnaround_time/(double)NUMBER_OF_JOBS));
}
