#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<pthread.h>
#include<semaphore.h>
#include<sys/types.h>
#include<unistd.h>
#include<sys/time.h>

#include "CustomerQueue.h"

typedef struct Clerk 
{
    int ID;
    sem_t* servingCustomer;
}Clerk;
struct timeval programStartTime;

pthread_mutex_t queueMutexs[4];

pthread_mutex_t queueChooserMutex;

pthread_cond_t queueConds[4];

sem_t numberOfCustomersThreads;
sem_t clerk1ServingCustomer;
sem_t clerk2ServingCustomer;
int queueLengths[4] = {0, 0 , 0 ,0};

CustomerNode* headOfQueues[4] = {NULL,NULL,NULL,NULL};


void PrintCustomer(Customer customer)
{
    printf("customer ID: %d\n",customer.ID);
    printf("customer arrivalTime: %d\n",customer.arrivalTime);
    printf("customer serviceTime: %d\n",customer.serviceTime);
}

float timeDifference(struct timeval t1, struct timeval t2)
{
    return ((t1.tv_sec - t2.tv_sec) + (t1.tv_usec - t2.tv_usec) / 1000000.0f);
}
//function parses a line from the input file and creates a new customer struct from it
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
    
    *queueID = maxQueueindex;
}
void FindShortestQueue(int* queueID )
{    
    //because we find the shortest queue randomly, if there is a tie which min length queue is chosen will be random
    int indexArray[4] = {0,1,2,3};
    ShuffleArray(indexArray,4);
    
    int minQueueIndex = indexArray[0];
    int minQueueLength= queueLengths[minQueueIndex];
    int currentQueueLength;
    int currentIndex;
    int i;
    for(i = 1; i < 4; i ++)
    {
        currentIndex = indexArray[i];
        
        currentQueueLength = queueLengths[currentIndex];

        if(currentQueueLength < minQueueLength)
        {
            minQueueLength = currentQueueLength;
            minQueueIndex = currentIndex;
        }
    }
    *queueID = minQueueIndex;
}

//handles all of the clerk thread's functionality
void * ClerkFunction(void * clerkVoid)
{
    Clerk* clerk = (Clerk*)clerkVoid;

    int clerkID = clerk->ID;
    sem_t* servingCustomer = clerk->servingCustomer;
    int queueID;

    while(1)
    {
        //waits for customers to enter the queue
        sem_wait(&numberOfCustomersThreads);

        pthread_mutex_lock(&queueChooserMutex);

        FindLongestQueue(&queueID);
        Customer* customer = PopCustomerNode(&headOfQueues[queueID]);

        customer->servicedBy = clerkID;
        customer->clerkSemToPost = servingCustomer;
        queueLengths[queueID] = queueLengths[queueID] -1;

        pthread_mutex_unlock(&queueChooserMutex);

        pthread_mutex_lock(&queueMutexs[queueID]);
        pthread_cond_signal(&queueConds[queueID]);
        pthread_mutex_unlock(&queueMutexs[queueID]);
        //waits for customer to be finished being served
        sem_wait(servingCustomer);
    }
    return NULL;
}
void * CustomerFunction( void * customerVoid)
{

    Customer* customer = (Customer*)customerVoid;
    
    //wait for customer to arrive then put him in a queue
    usleep(customer->arrivalTime * 100000);
    fprintf(stdout,"A customer arrives: customer ID %2d.\n", customer->ID);

    //calculate arrival time
    struct timeval rawCustomerArrivalTime;
    gettimeofday(&rawCustomerArrivalTime,NULL);

    float customerArrivalTime = timeDifference(rawCustomerArrivalTime, programStartTime);
    //figures out which queue is the shortest to put customer in
    int queueID;
    pthread_mutex_lock(&queueChooserMutex);
    FindShortestQueue(&queueID);
    InsertAtTail(customer,&headOfQueues[queueID]);
    queueLengths[queueID] = queueLengths[queueID] + 1;
    fprintf(stdout, "A customer enters a queue: the queue ID %1d, and length of the queue %2d. \n",queueID, queueLengths[queueID]);
    pthread_mutex_unlock(&queueChooserMutex);
    //lets clerk thread know there is now another customer in the queue
    sem_post(&numberOfCustomersThreads);

    pthread_mutex_lock(&queueMutexs[queueID]);
    //servicedBy == -1 means no clerk has flagged the customer to be served yet
    while(customer->servicedBy == -1)
    {
        pthread_cond_wait(&queueConds[queueID],&queueMutexs[queueID]);
    }
    pthread_mutex_unlock(&queueMutexs[queueID]);

    struct timeval rawStartServicedTime;
    gettimeofday(&rawStartServicedTime,NULL);
    float startServicedTime = timeDifference(rawStartServicedTime,programStartTime);
    fprintf(stdout, "A clerk starts serving a customer, start time: %.2f, the customer ID %2d, the clerk ID %1d. \n",startServicedTime,customer->ID,customer->servicedBy);
    //serving the customer
    usleep(customer->serviceTime * 100000);
    struct timeval rawEndTime;
    gettimeofday(&rawEndTime, NULL);

    float endTime = timeDifference(rawEndTime,programStartTime);
    fprintf(stdout, "A clerk finishes serving a customer: end time %.2f, the customer ID %2d, the clerk ID %1d. \n",endTime, customer->ID,customer->servicedBy);
    sem_post(customer->clerkSemToPost);
    //done with this customer so we can free it
    free(customer);
    //only time we use waitTime is at the very end of the program so dont have to worry about freeing it
    float* waitTime = (float*)malloc(sizeof(float));
    *waitTime = (endTime-customerArrivalTime);
    pthread_exit((void *)waitTime);

    return NULL;
}

int main( int argc, char* argv[] )
{
    //program start time
    gettimeofday(&programStartTime,NULL);

    CustomerNode* headOfStagingQueue = NULL;
    char* fileName = argv[1];
    char line[1024];
    pthread_t customerThreads[1024];
    pthread_t clerkThreads[2];
    
    int clerk1ID = 1;
    int clerk2ID = 2;
    size_t len = sizeof(line);
    
    //seed random num gen
    srand(time(NULL));
    //initalize semaphores
    sem_init(&numberOfCustomersThreads, 0 ,0);
    sem_init(&clerk1ServingCustomer,0,0);
    sem_init(&clerk2ServingCustomer,0,0);

    //intalize mutex and condvar
    int i;
    for(i = 0; i < 4; i++)
    {
        pthread_mutex_init(&queueMutexs[i],NULL);
        pthread_cond_init(&queueConds[i],NULL);
    }
    pthread_mutex_init(&queueChooserMutex,NULL);
    //parsing input file
    FILE* customerFile = fopen(fileName, "r");
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

        InsertAtTail(customer, &headOfStagingQueue);
    }
    fclose(customerFile);
    //set up clerk information
    Clerk* clerk1 = (Clerk*)malloc(sizeof(Clerk));
    clerk1->ID = 1;
    clerk1->servingCustomer = &clerk1ServingCustomer;
    Clerk* clerk2 = (Clerk*)malloc(sizeof(Clerk));
    clerk2->ID = 2;
    clerk2->servingCustomer = &clerk2ServingCustomer;

    //initalize clerk threads
    pthread_create(&clerkThreads[0],NULL, ClerkFunction,(void*)(clerk1));
    pthread_create(&clerkThreads[1],NULL, ClerkFunction,(void*)(clerk2));

    Customer* customer = PopCustomerNode(&headOfStagingQueue);
    int customerThreadIndex = 0;
    
    while(customer != NULL && customerThreadIndex < 1024)
    {   
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
    void* waitingTime[1024];
    int joinIndex;
    for(joinIndex = 0; joinIndex < customerThreadIndex; joinIndex++)
    {
        pthread_join(customerThreads[joinIndex],&waitingTime[joinIndex]);
    }

    //calculate average wait time
    float averageWaitTime = 0;
    int avgIndex;
    for(avgIndex = 0; avgIndex < customerThreadIndex; avgIndex++)
    {
        averageWaitTime = averageWaitTime + *((float *)waitingTime[avgIndex]);
    }
    averageWaitTime = averageWaitTime/customerThreadIndex;
    fprintf(stdout,"average wait time for customers was %.2f\n", averageWaitTime);
}