#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<pthread.h>
#include<semaphore.h>
#include<sys/types.h>
#include<unistd.h>

#include "CustomerQueue.h"

pthread_mutex_t queueMutexs[4];

pthread_mutex_t queueChooserMutex;

pthread_cond_t queueConds[4];

sem_t numberOfCustomersThreads;

int queueLengths[4] = {0, 0 , 0 ,0};

CustomerNode* headOfQueues[4] = {NULL,NULL,NULL,NULL};


void PrintCustomer(Customer customer)
{
    printf("customer ID: %d\n",customer.ID);
    printf("customer arrivalTime: %d\n",customer.arrivalTime);
    printf("customer serviceTime: %d\n",customer.serviceTime);
}

Customer* LineToCustomer(char* line, size_t len)
{
    char IDString[8];
    char arrivalTimeString[8];
    char serviceTimeString[8];

    int lineCharacterPtr = 0;
    int arrivalCharCounter = 0;
    int serviceCharCounter = 0;
    Customer* newCustomer = (Customer*)malloc(sizeof(Customer));
    while(line[lineCharacterPtr] != ':' && lineCharacterPtr < 7)
    {
        IDString[lineCharacterPtr] = line[lineCharacterPtr];
        lineCharacterPtr++;
    }
    IDString[lineCharacterPtr] = '\0';
    lineCharacterPtr++;
    while(line[lineCharacterPtr] != ',' && arrivalCharCounter < 7)
    {
        arrivalTimeString[arrivalCharCounter] = line[lineCharacterPtr];
        arrivalCharCounter++;
        lineCharacterPtr++;
    }
    arrivalTimeString[arrivalCharCounter] = '\0';
    lineCharacterPtr++;
    while(lineCharacterPtr < len && serviceCharCounter < 7)
    {
        serviceTimeString[serviceCharCounter] = line[lineCharacterPtr];
        serviceCharCounter++;
        lineCharacterPtr++;
    }
    serviceTimeString[serviceCharCounter] = '\0';
    newCustomer->ID = atoi(IDString);
    newCustomer->arrivalTime = atoi(arrivalTimeString);
    newCustomer->serviceTime = atoi(serviceTimeString);
    if(newCustomer->ID < 0 || newCustomer->arrivalTime <0 || newCustomer-> serviceTime <0)
    {
        return NULL;
    }
    return newCustomer;
}

void ShuffleArray(int* array, int size)
{
    int temp;
    int randIndex;
    int i;
    for(i = 0; i < size; i++)
    {
        int temp = array[i];
        int randIndex = (rand() % 4);

        array[i] = array[randIndex];
        array[randIndex] = temp;
    }
}
void FindLongestQueue(int* localQueueLengthPtr, pthread_mutex_t* localMutex, pthread_cond_t* localCondVar, CustomerNode** localQueueHeadPtr)
{    
    //because we find the shortest queue randomly, if there is a tie which min length queue is chosen will be random
    int indexArray[4] = {0,1,2,3};
    ShuffleArray(indexArray,4);
    
    int maxQueueLength= queueLengths[indexArray[0]];
    int maxQueueindex = indexArray[0];

    int currentQueue;
    int currentIndex;
    int i;
    for(i = 1; i < 4; i ++)
    {
        currentIndex = indexArray[i];
        currentQueue = queueLengths[currentIndex];

        if(currentQueue > maxQueueLength)
        {
            maxQueueLength = currentQueue;
            maxQueueindex = currentIndex;
        }
    }
    printf("found Longest Queue");
    localQueueLengthPtr = &(queueLengths[maxQueueindex]);
    localMutex = &(queueMutexs[maxQueueindex]);
    localCondVar = &(queueConds[maxQueueindex]);
    localQueueHeadPtr = &(headOfQueues[maxQueueindex]);

}
void FindShortestQueue(int* localQueueLengthPtr, pthread_mutex_t* localMutex, pthread_cond_t* localCondVar, CustomerNode** localQueueHeadPtr,int* QueueID )
{    
    //because we find the shortest queue randomly, if there is a tie which min length queue is chosen will be random
    int indexArray[4] = {0,1,2,3};
    ShuffleArray(indexArray,4);
    int minQueueLength= queueLengths[indexArray[0]];
    int minQueueIndex = indexArray[0];

    int currentQueueLength;
    int currentIndex;
    int i;
    for(i = 1; i < 4; i ++)
    {
        currentIndex = indexArray[i];
        currentQueueLength = queueLengths[currentIndex];

        if((currentQueueLength > 0 && currentQueueLength < minQueueLength) || (minQueueLength == 0) )
        {
            minQueueLength = currentQueueLength;
            minQueueIndex = currentIndex;
            *QueueID = currentIndex;
        }
    }
    printf("returning queue with head customer ID: %d\n",headOfQueues[minQueueIndex]->customer->ID );
    localQueueLengthPtr = &(queueLengths[minQueueIndex]);
    localMutex = &(queueMutexs[minQueueIndex]);
    localCondVar = &(queueConds[minQueueIndex]);
    localQueueHeadPtr = &(headOfQueues[minQueueIndex]);
}
int ClerkWhoCalled()
{

    return 0;
}
void * ClerkFunction(void * clerkVoid)
{
    int clerkID = *((int*)clerkVoid);
    int* localQueueLengthPtr;
    pthread_mutex_t* localMutex;
    pthread_cond_t* localCondVar;
    CustomerNode** localQueueHeadPtr;
    while(1)
    {
        printf("in clerk function\n");
        sem_wait(&numberOfCustomersThreads);

        printf("semaphore posted\n");
        pthread_mutex_lock(&queueChooserMutex);
        FindLongestQueue(localQueueLengthPtr,localMutex,localCondVar,localQueueHeadPtr);
        PopCustomerNode(localQueueHeadPtr);
        *localQueueLengthPtr = *localQueueLengthPtr -1;
        pthread_mutex_unlock(&queueChooserMutex);

        pthread_mutex_lock(localMutex);
        pthread_cond_signal(localCondVar);
        pthread_mutex_unlock(localMutex);
    }
    return NULL;
}
void * CustomerFunction( void * customerVoid)
{
    Customer* customer = (Customer*)customerVoid;

    printf("sleeping for %d, with customer ID: %d\n", customer->arrivalTime * 100000, customer->ID);
    usleep(customer->arrivalTime * 100000);
    printf("thread with id %d woke up\n",customer->ID);
    int* localQueueLengthPtr;
    int* QueueID;
    pthread_mutex_t* localMutex;
    pthread_cond_t* localCondVar;
    CustomerNode** localQueueHead;
    
    pthread_mutex_lock(&queueChooserMutex);

    printf("finding a shortest queue for customer %d\n", customer->ID);
    FindShortestQueue(localQueueLengthPtr, localMutex, localCondVar,localQueueHead,QueueID);
    printf("found a shortest queue\n");
    InsertAtTail(customer,localQueueHead);
    printf("inserted at tail of queue\n");
    *localQueueLengthPtr = (*localQueueLengthPtr) + 1;

    pthread_mutex_unlock(&queueChooserMutex);
    
    fprintf(stdout, "A customer enters a queue: the queue ID %1d, and length of the queue %2d. \n",*QueueID, *localQueueLengthPtr);
    sem_post(&numberOfCustomersThreads);

    pthread_mutex_lock(localMutex);
    while(customer->ID != (*localQueueHead)->customer->ID)
    {
        pthread_cond_wait(localCondVar, localMutex);
    }
    pthread_mutex_unlock(localMutex);

    int clerkID = ClerkWhoCalled();

    usleep(customer->serviceTime * 100 / sysconf(_SC_CLK_TCK));

    fprintf(stdout, "A clerk finishes serving a customer: end time [need to add], the customer ID %2d, the clerk ID %1d. \n", customer->ID,-1);

    free(customer);
    pthread_exit(NULL);

    return NULL;
}

int main( int argc, char* argv[] )
{
    CustomerNode* headOfStagingQueue = NULL;
    char* fileName = argv[1];
    char line[1024];
    pthread_t customerThreads[1024];
    pthread_t clerkThreads[2];
    sem_init(&numberOfCustomersThreads, 0 ,0);
    int clerk1ID = 1;
    int clerk2ID = 2;
    size_t len = sizeof(line);
    FILE* customerFile = fopen(fileName, "r");
    //seed random num gen
    srand(time(NULL));

    //intalize mutex and condvar
    int i;
    for(i = 0; i < 4; i++)
    {
        pthread_mutex_init(&(queueMutexs[i]),NULL);
        pthread_cond_init(&(queueConds[i]),NULL);
    }
    pthread_mutex_init(&queueChooserMutex,NULL);

    fgets(line, len, customerFile);
    int numberOfCustomers = atoi(line);

    printf("first line: %s", line);
    
    while(fgets(line, len, customerFile))
    {
        Customer* customer = LineToCustomer(line,len);
        if(customer == NULL)
        {
            printf("found negative value in customer line, skipping this customer\n");
            numberOfCustomers = numberOfCustomers -1;
            continue;
        }
        //printf("printing customer\n");
        //PrintCustomer(*customer);
        //printf("before inserting at tail \n");
        //PrintList(&headOfStagingQueue);

        InsertAtTail(customer, &headOfStagingQueue);
        PrintList(&headOfStagingQueue);
        //free(customer);
    }
    fclose(customerFile);

    //initalize clerk threads
    pthread_create(&clerkThreads[0],NULL, ClerkFunction,(void*)(&clerk1ID));
    pthread_create(&clerkThreads[1],NULL, ClerkFunction,(void*)(&clerk2ID));

    Customer* customer = PopCustomerNode(&headOfStagingQueue);
    int customerThreadIndex = 0;
    while(customer->ID != (-1) && customerThreadIndex < 1024)
    {   
        
        printf("Creating a thread with customer ID: %d, arrivalTime: %d, serviceTime: %d \n", customer->ID,customer->arrivalTime, customer->serviceTime);
        if(!(pthread_create(&customerThreads[customerThreadIndex], NULL, CustomerFunction, (void* )(customer))))
        {    
            customerThreadIndex++;
            customer = PopCustomerNode(&headOfStagingQueue);
        }
        else
        {
            printf("failed to create thread for customer with customer ID: %d\n", customer->ID);
        }
    }
    int joinIndex;
    for(joinIndex = 0; joinIndex < customerThreadIndex; joinIndex++)
    {
        printf("trying to join thread %d\n", joinIndex);
        pthread_join(customerThreads[joinIndex],NULL);
        printf("joined thread %d\n", joinIndex);
    }
    printf("all threads joined\n");
}