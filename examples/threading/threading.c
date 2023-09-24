#include "threading.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>



#define USECS_IN_A_MILISEC 1000


// Optional: use these functions to add debug or error prints to your application
//#define DEBUG_LOG(msg,...)
#define DEBUG_LOG(msg,...) printf("threading: " msg "\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg,...) printf("threading ERROR: " msg "\n" , ##__VA_ARGS__)

void* threadfunc(void* thread_param)
{
    int rc;
    //First retrieve the param by casting and dereferencing
    struct thread_data * threadParams = (struct thread_data *)thread_param;

    //Now first we wait to obtain the mutex
    usleep(threadParams->obtain_delay*USECS_IN_A_MILISEC);

    //Now we obtain the mutex
    rc = pthread_mutex_lock(threadParams->mutex);
    if (rc!=0){
        DEBUG_LOG("Failed to obtain mutex!");
        return thread_param;
    }
    DEBUG_LOG("Mutex has been obtained!");
    //We wait to release the mutex
    usleep(threadParams->release_delay*USECS_IN_A_MILISEC);

    //Now we release the mutex

    rc = pthread_mutex_unlock(threadParams->mutex);
    if (rc!=0){
        DEBUG_LOG("Failed to release mutex!");
        return thread_param;
    }
    DEBUG_LOG("Mutex has been released!");
    //Set the result in the thread_param
    threadParams->thread_complete_success = true;

    return thread_param;
}


bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex,int wait_to_obtain_ms, int wait_to_release_ms)
{
    int rc;
    struct thread_data * threadData;
    threadData = (struct thread_data *) malloc(sizeof(struct thread_data));

    if(threadData == NULL){
        ERROR_LOG("Failed to allocate memory for thread paramaters");
        return false;

    }
    DEBUG_LOG("Thread param allocation success!");

    //Setup threadData
    threadData->mutex = mutex;
    threadData->obtain_delay = wait_to_obtain_ms;
    threadData->release_delay = wait_to_release_ms;
    threadData->thread_complete_success = false;


    DEBUG_LOG("Mutex Initialization success!");

    //Create the thread
    rc = pthread_create(thread,NULL,threadfunc,(void *)threadData);

    if(rc != 0){
        ERROR_LOG("Failed to create the p thread");
        return false;
    }
    DEBUG_LOG("P Thread creation success!");


    return true;
}

