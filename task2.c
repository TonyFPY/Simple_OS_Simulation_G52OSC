/*
    OSC Task2
    Author: Muyuan Chen  20031527 scymc1
            Pinyuan Feng 20028407 scypf1
*/
#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include "coursework.h"
#include "linkedlist.h"

// initialization as global variable
sem_t sSync, sDelayConsumer;
int items,temp;
int num = NUMBER_OF_JOBS;
int countP, countC;                      // count how many items are produced and consumed

void * consumer(void *p);
void * producer(void *p);
void display(char role, int num_of_elements);

int main(int argc, char * agrv[]){
    // initialization
    pthread_t prod, cons;
    int ss, sd;
    sem_init(&sSync, 0, 1);             // sSync is initialized to 1
    sem_init(&sDelayConsumer, 0, 0);    // sDelayConsumer is initialized to 0

    // run the threads
    pthread_create(&prod, NULL, producer, NULL); 
    pthread_create(&cons, NULL, consumer, NULL); 
    pthread_join(prod, NULL);
    pthread_join(cons, NULL);
    
    // print out the values of sSync and sDelayConsumer
    sem_getvalue(&sSync, &ss);           // get the value of sSync
    sem_getvalue(&sDelayConsumer, &sd);  // get the value of sDelayConsumer
    printf("sSync = %d, sDelayConsumer = %d\n", ss, sd);

    return 0;
}

void * consumer(void *p){
    sem_wait(&sDelayConsumer);
    while(1){
        sem_wait(&sSync);
        items --;
        countC ++;
        display('c', items);                 // display the number of elements
        temp = items;
        sem_post(&sSync);
        if(countC == num){                   // no item to be consumed
            break;
        }
        if(temp == 0){                       // no item in the buffer
            sem_wait(&sDelayConsumer);       // go to sleep
        }
    }
    return ((void *)0);
    
}

void * producer(void *p){
    while(countP < num){
        sem_wait(&sSync);
        items ++;
        countP ++;
        display('p', items);                // display the items using "*"
        if(items == 1){
            sem_post(&sDelayConsumer);
        }
        sem_post(&sSync);
    }
    return ((void *)0);
}

void display(char role, int num_of_elements){
    switch(role){
        case 'p':
            printf("Producer, Produced = %d, Consumed = %d: ", countP, countC);
            break;
        case 'c':
            printf("Consumer, Produced = %d, Consumed = %d: ", countP, countC);
            break;
        default:
            break;
    }

    for(int i = num_of_elements; i > 0; i--){
        printf("*");
    }

    printf("\n");

    return;
}