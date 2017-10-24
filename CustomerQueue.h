#include<stdio.h>
#include<stdlib.h>
typedef struct CustomerNode
{
  int pid;
  struct CustomerNode* next;
  struct CustomerNode* prev;
}CustomerNode;
typedef struct Customer
{
    int ID;
    int arrivalTime;
    int serviceTime; 
}Customer;
CustomerNode* NewCustomerNode(int pid);
void PrintList(CustomerNode** head);
CustomerNode* FindCustomerNodePointer(int pid, CustomerNode* head);
void RemoveCustomerNode(CustomerNode* toDelete,CustomerNode** headPointer);
void InsertAtHead(int pid, CustomerNode** head);