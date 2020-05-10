/*
    OSC Task3
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

// declaration
sem_t sSync, sDelayProducer, sCount;
int temp, items, produced, consumed;
Element *qHead = NULL;
Element *qTail = NULL;

void * consumer(void *p);
void * producer(void *p);
void display(char role);

int main(int argc, char * agrv[]){
    pthread_t prod, cons;
    int s_sync, s_delayproducer, s_count;
    sem_init(&sSync, 0, 1);                         // binary semaphore
    sem_init(&sDelayProducer, 0, 1);                // binary semaphore to let producer sleep
    sem_init(&sCount, 0, 0);                        // counting semaphore

    pthread_create(&prod, NULL, producer, NULL); 
    pthread_create(&cons, NULL, consumer, NULL); 
    pthread_join(prod, NULL);
    pthread_join(cons, NULL);

    // print out the values of semaphores
    sem_getvalue(&sSync, &s_sync);           
    sem_getvalue(&sDelayProducer, &s_delayproducer);  
    sem_getvalue(&sCount, &s_count);         
    printf("sSync = %d, sDelayProducer = %d, sCount = %d\n", s_sync, s_delayproducer, s_count);
    return 0;
}

void * producer(void * p){
    while(produced < NUMBER_OF_JOBS){
        sem_wait(&sDelayProducer);
        sem_wait(&sSync);
        
        // add an item to the buffer
        char *curValue = (char*)malloc(sizeof(char));
        *curValue = '*';
        addLast(curValue, &qHead, &qTail);
        sem_post(&sCount);
        items ++;
        produced ++;

        display('p');                   // display the items using "*"
        if(items < MAX_BUFFER_SIZE){
            sem_post(&sDelayProducer);
        }
        sem_post(&sSync);
    }
    return ((void *)0);
}

void * consumer(void * p){
    while(consumed < NUMBER_OF_JOBS){
        sem_wait(&sCount);
        sem_wait(&sSync);

        // remove an item from the buffer
        int *curValue = removeFirst(&qHead, &qTail);
        free(curValue);
        items--;
        consumed++;
        if (items == MAX_BUFFER_SIZE - 1) {
            sem_post(&sDelayProducer);
        }

        display('c');                 // display the number of elements
        sem_post(&sSync);
    }
    return ((void *)0);
}

// generate star in the buffer
void display(char role){
    if(role == 'p'){
        printf("Producer 1, produced = %d, consumed = %d: ", produced, consumed);
    } else {
        printf("Consumer 1, produced = %d, consumed = %d: ", produced, consumed);
    }
    

    // go through the linkedlist and print out the star one by one
    Element *qTemp;
    qTemp = qHead;
    while(qTemp != NULL){
        printf("%c", *((char *)(qTemp -> pData)));
        qTemp = qTemp -> pNext;
    }

    printf("\n");
    
    return;
}