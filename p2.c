#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<pthread.h>
#include<sys/types.h>
#include<unistd.h>

#include "CustomerQueue.h"

pthread_mutex_t queueMutexs[4];

pthread_mutex_t queueChooserMutex;

pthread_cond_t queueConds[4];

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
void FindShortestQueue(int* localQueueLengthPtr, pthread_mutex_t* localMutex, pthread_cond_t* localCondVar, CustomerNode** localQueueHeadPtr )
{    
    //because we find the shortest queue randomly, if there is a tie which min length queue is chosen will be random
    int indexArray[4] = {0,1,2,3};
    ShuffleArray(indexArray,4);
    
    int minQueueLength= queueLengths[indexArray[0]];
    int indexOfMinQueue = 0;

    int currentQueue;
    int currentIndex;
    int i;
    for(i = 1; i < 4; i ++)
    {
        currentIndex = indexArray[i];
        currentQueue = queueLengths[currentIndex];

        if(currentQueue < minQueueLength)
        {
            minQueueLength = currentQueue;
            indexOfMinQueue = currentIndex;
        }
    }
    localQueueLengthPtr = &(queueLengths[indexOfMinQueue]);
    localMutex = &(queueMutexs[indexOfMinQueue]);
    localCondVar = &(queueConds[indexOfMinQueue]);
    localQueueHeadPtr = &(headOfQueues[indexOfMinQueue]);
}
int ClerkWhoCalled()
{

    return 0;
}
void * CustomerFunction( void * customerVoid)
{
    Customer* customer = (Customer*)customerVoid;
    usleep(customer->arrivalTime * 100 / sysconf(_SC_CLK_TCK));
    int* localQueueLengthPtr;
    pthread_mutex_t* localMutex;
    pthread_cond_t* localCondVar;
    CustomerNode** localQueueHead;

    FindShortestQueue(localQueueLengthPtr, localMutex, localCondVar,localQueueHead);
    
    pthread_mutex_lock(&queueChooserMutex);
    FindShortestQueue(localQueueLengthPtr, localMutex, localCondVar,localQueueHead);
    localQueueLengthPtr = localQueueLengthPtr + 1;
    pthread_mutex_unlock(&queueChooserMutex);

    while(pthread_cond_wait(localCondVar, localMutex));

    *localQueueLengthPtr = *localQueueLengthPtr - 1;
    pthread_mutex_unlock(localMutex);

    int clerkID = ClerkWhoCalled();

    usleep(customer->serviceTime * 100 / sysconf(_SC_CLK_TCK));

    pthread_mutex_lock(localMutex);
    pthread_cond_signal(localCondVar);
    pthread_mutex_unlock(localMutex);

    pthread_exit(NULL);
}

int main( int argc, char* argv[] )
{
    CustomerNode* headOfStagingQueue = NULL;
    char* fileName = argv[1];
    char line[1024];
    pthread_t customerThreads[1024];
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

    Customer customer = PopCustomerNode(&headOfStagingQueue);
    int customerThreadIndex = 0;
    while(customer.ID != -1)
    {
        if(pthread_create(&customerThreads[customerThreadIndex], NULL, CustomerFunction, (void* )(&customer)))
        {    
            customerThreadIndex++;
            customer = PopCustomerNode(&headOfStagingQueue);
        }
    }
}