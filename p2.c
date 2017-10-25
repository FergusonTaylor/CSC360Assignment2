#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<pthread.h>
#include<sys/types.h>
#include<unistd.h>

#include "CustomerQueue.h"

pthread_mutex_t queue1Mutex;
pthread_mutex_t queue2Mutex;
pthread_mutex_t queue3Mutex;
pthread_mutex_t queue4Mutex;

pthread_cond_t queue1Cond;
pthread_cond_t queue2Cond;
pthread_cond_t queue3Cond;
pthread_cond_t queue4Cond;

int queue1Length = 0;
int queue2Length = 0;
int queue3Length = 0;
int queue4Length = 0;

CustomerNode* headOfQueue1 = NULL;
CustomerNode* headOfQueue2 = NULL;
CustomerNode* headOfQueue3 = NULL;
CustomerNode* headOfQueue4 = NULL;

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
//TODO: find shortest queue
void FindShortestQueue(int* localQueueLengthPtr, pthread_mutex_t* localMutex, pthread_cond_t* localCondVar)
{

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
    FindShortestQueue(localQueueLengthPtr, localMutex, localCondVar);
    
    pthread_mutex_lock(localMutex);
    FindShortestQueue(localQueueLengthPtr, localMutex, localCondVar);
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
       // PrintList(&headOfStagingQueue);

        InsertAtTail(customer, &headOfStagingQueue);
        printf("after inserting at tail \n");
        PrintList(&headOfStagingQueue);
        //free(customer);
    }
    fclose(customerFile);
}