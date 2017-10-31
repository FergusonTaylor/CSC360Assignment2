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
    newCustomer->servicedBy = -1;
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
        int randIndex = (rand() % size);

        array[i] = array[randIndex];
        array[randIndex] = temp;
    }
}
void FindLongestQueue(int* queueID)
{    
    //because we find the shortest queue randomly, if there is a tie which max length queue is chosen will be random
    int indexArray[4] = {0,1,2,3};
    ShuffleArray(indexArray,4);
    //printf("shuffled Array: %d, %d, %d, %d\n",indexArray[0],indexArray[1],indexArray[2],indexArray[3]);
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
            //printf("changing queue ID to %d\n", currentIndex);
        }
    }
    //printf("found Longest Queue");
    *queueID = maxQueueindex;


}
void FindShortestQueue(int* queueID )
{    
    //because we find the shortest queue randomly, if there is a tie which min length queue is chosen will be random
    int indexArray[4] = {0,1,2,3};
    ShuffleArray(indexArray,4);
    int minQueueLength= queueLengths[indexArray[0]];
    int minQueueIndex = indexArray[0];

    int currentQueueLength;
    int currentIndex;
    int i;
   // printf("Entering for loop in Shorest Queue finder\n");
    for(i = 1; i < 4; i ++)
    {
        currentIndex = indexArray[i];
        currentQueueLength = queueLengths[currentIndex];

        if((currentQueueLength > 0 && currentQueueLength < minQueueLength) || (minQueueLength == 0) )
        {
            minQueueLength = currentQueueLength;
            minQueueIndex = currentIndex;
        }
    }
    //printf("minQueueIndex: %d minQueueLength: %d\n", minQueueIndex, minQueueLength);
    //printf("returning queue with head customer ID: %d\n",headOfQueues[minQueueIndex]->customer->ID );
    *queueID = currentIndex;

}
int ClerkWhoCalled()
{

    return 0;
}
void * ClerkFunction(void * clerkVoid)
{
    int clerkID = *((int*)clerkVoid);
    int queueID;

    while(1)
    {
        //printf("in clerk function\n");
        sem_wait(&numberOfCustomersThreads);

        pthread_mutex_lock(&queueChooserMutex);
        printf("clerk %d printList 0 \n",clerkID);
        PrintList(&headOfQueues[0]);
        printf("clerk %d printList 1 \n",clerkID);
        PrintList(&headOfQueues[1]);
        printf("clerk %d printList 2 \n",clerkID);
        PrintList(&headOfQueues[2]);
        printf("clerk %d printList 3 \n",clerkID);
        PrintList(&headOfQueues[3]);
        FindLongestQueue(&queueID);

        printf("clerk %d, longest queue found was queue %d with  length of %d\n",clerkID, queueID, queueLengths[queueID]);
        PrintList(&headOfQueues[queueID]);

        Customer* customer = PopCustomerNode(&headOfQueues[queueID]);
        printf("clerk popped customer with memory location: %p\n", (void*) customer);

        customer->servicedBy = clerkID;
        queueLengths[queueID] = queueLengths[queueID] -1;
        if(customer == NULL)
        {
            printf("customer popped off by clerk was null\n");
        }
        else
        {
            printf("customer popped off by clerk with ID %d \n", customer->ID);
        }
        if(queueLengths[queueID] < 0)
        {
            printf("queue length went below 0 we have a problem\n");
        }
        
        pthread_mutex_unlock(&queueChooserMutex);

        pthread_mutex_lock(&queueMutexs[queueID]);
        printf("clerk %d signaling condition variable with queue ID %d\n",clerkID, queueID);
        pthread_cond_signal(&queueConds[queueID]);
        pthread_mutex_unlock(&queueMutexs[queueID]);
        //printf("unlocked clerk queue chooser mutex, queueID is currently %d\n",queueID);

        //printf("locking mutex at memory location %p\n",(void * )&queueMutexs[queueID]);

        //usleep(100000);
    }
    return NULL;
}
void * CustomerFunction( void * customerVoid)
{

    Customer* customer = (Customer*)customerVoid;

    //printf("sleeping for %d, with customer ID: %d\n", customer->arrivalTime * 100000, customer->ID);
    usleep(customer->arrivalTime * 100000);
    int queueID;

    pthread_mutex_lock(&queueChooserMutex);
    FindShortestQueue(&queueID);
    InsertAtTail(customer,&headOfQueues[queueID]);
    printf("customer inserted at tail of queue: %d\n", queueID);
    printf("queue that node was added to has memory address %p\n",(void *)&headOfQueues[queueID]);
    PrintList(&headOfQueues[queueID]);
    queueLengths[queueID] = queueLengths[queueID] + 1;
    fprintf(stdout, "A customer enters a queue: the queue ID %1d, and length of the queue %2d. \n",queueID, queueLengths[queueID]);
    pthread_mutex_unlock(&queueChooserMutex);

    printf("mutex was unlocked\n");

    sem_post(&numberOfCustomersThreads);

    printf("about to lock queue mutex %d\n", queueID);
    pthread_mutex_lock(&queueMutexs[queueID]);
    //printf("waiting for clerk to signal\n");
    printf("locked mutex at memory location %p\n",(void * )&queueMutexs[queueID]);

    pthread_cond_wait(&queueConds[queueID], &queueMutexs[queueID]);
    printf("checking while loop\n");
    printf("customer thread customer has memory location: %p\n", (void*) customer);
    while(customer->servicedBy == -1)
    {
        printf("WAITING!!! \n\n\n\n\n");
        pthread_cond_wait(&queueConds[queueID],&queueMutexs[queueID]);
    }
    printf("passed while loop\n");
    pthread_mutex_unlock(&queueMutexs[queueID]);
    printf("customer woke up\n");
    int clerkID = ClerkWhoCalled();

    usleep(customer->serviceTime * 100000);

    fprintf(stdout, "A clerk finishes serving a customer: end time [need to add], the customer ID %2d, the clerk ID %1d. \n", customer->ID,customer->servicedBy);

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
        pthread_mutex_init(&queueMutexs[i],NULL);
        pthread_cond_init(&queueConds[i],NULL);
    }
    pthread_mutex_init(&queueChooserMutex,NULL);

    fgets(line, len, customerFile);
    int numberOfCustomers = atoi(line);
    
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
    
    while(customer != NULL && customerThreadIndex < 1024)
    {   
        
        //printf("Creating a thread with customer ID: %d, arrivalTime: %d, serviceTime: %d \n", customer->ID,customer->arrivalTime, customer->serviceTime);
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