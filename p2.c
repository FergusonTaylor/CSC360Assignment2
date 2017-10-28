#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<pthread.h>
#include<sys/types.h>
#include<unistd.h>

#include "CustomerQueue.h"

pthread_mutex_t queueMutexs[4];

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
    
    int minQueueLength;
    int indexOfMinQueue;
    int i;
    for(i = 0; i < 4; i ++)
    {
        minQueueLength = queueLengths[indexArray[i]];
        indexOfMinQueue = indexArray[i];
        if(minQueueLength <= queueLengths[indexArray[(i+1)%4]] && minQueueLength <= queueLengths[indexArray[(i+2)%4]] &&
        minQueueLength <= queueLengths[indexArray[(i+3)%4]])
        {
            break;
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
void CustomerFunction(Customer customer)
{
    usleep(customer.arrivalTime * 100 / sysconf(_SC_CLK_TCK));
    int* localQueueLengthPtr;
    pthread_mutex_t* localMutex;
    pthread_cond_t* localCondVar;
    CustomerNode** localQueueHead;

    FindShortestQueue(localQueueLengthPtr, localMutex, localCondVar,localQueueHead);
    
    pthread_mutex_lock(localMutex);
    FindShortestQueue(localQueueLengthPtr, localMutex, localCondVar,localQueueHead);
    localQueueLengthPtr = localQueueLengthPtr + 1;

    while(pthread_cond_wait(localCondVar, localMutex));

    *localQueueLengthPtr = *localQueueLengthPtr - 1;
    pthread_mutex_unlock(localMutex);

    int clerkID = ClerkWhoCalled();

    usleep(customer.serviceTime * 100 / sysconf(_SC_CLK_TCK));

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
    size_t len = sizeof(line);
    FILE* customerFile = fopen(fileName, "r");
    //seed random num gen
    srand(time(NULL));

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
        printf("after inserting at tail \n");
        PrintList(&headOfStagingQueue);
        printf("finding shortest queue\n");
        FindShortestQueue(NULL,NULL,NULL,NULL);
        //free(customer);
    }
    fclose(customerFile);

}