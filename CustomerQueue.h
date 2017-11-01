#include<stdio.h>
#include<stdlib.h>
#include<semaphore.h>
typedef struct Customer
{
    int ID;
    int arrivalTime;
    int serviceTime;
    int servicedBy; 
    sem_t* clerkSemToPost;
}Customer;
typedef struct CustomerNode
{
  Customer* customer;
  struct CustomerNode* next;
}CustomerNode;

CustomerNode* NewCustomerNode(Customer* customer);
void PrintList(CustomerNode** head);
CustomerNode* FindCustomerNodePointer(Customer customer, CustomerNode* head);
Customer* PopCustomerNode(CustomerNode** headPointer);
void InsertAtTail(Customer* customer, CustomerNode** head);