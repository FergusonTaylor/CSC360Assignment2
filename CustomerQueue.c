#include "CustomerQueue.h"

CustomerNode* NewCustomerNode(Customer customer)
{
  CustomerNode* newCustomerNode = (CustomerNode*)malloc(sizeof(CustomerNode));
  newCustomerNode->customer = customer;
  newCustomerNode->next = NULL;
  newCustomerNode->prev = NULL;
  return newCustomerNode;
}
void PrintList(CustomerNode** head)
{
  if(*head == NULL)
  {
    printf("the list is empty\n");
    return;
  }
  CustomerNode currentCustomerNode = **head;
  printf("pid: %3d , ", currentCustomerNode.pid);
  while(currentCustomerNode.next != NULL )
  {
    currentCustomerNode = *(currentCustomerNode.next);
    printf(" %3d ,", currentCustomerNode.pid);
  }
  printf("\n");
}
CustomerNode* FindCustomerNodePointer(int pid, CustomerNode* head)
{
  CustomerNode* currentCustomerNode = head;
  while(currentCustomerNode !=NULL)
  {
    if(currentCustomerNode->pid == pid)
    {
      break;
    }
    currentCustomerNode = currentCustomerNode->next;
  }
  return currentCustomerNode;
}
void RemoveCustomerNode(CustomerNode* toDelete,CustomerNode** headPointer)
{
  if(*headPointer == toDelete)
  {
    *headPointer = toDelete->next;
  }
  if(toDelete->next != NULL)
  {
    toDelete->next->prev = toDelete->prev;
  }
  if(toDelete->prev !=NULL)
  {
    toDelete->prev->next = toDelete->next;
  }
  free(toDelete);
}
void InsertAtHead(int pid, CustomerNode** head)
{
  CustomerNode* newCustomerNode = NewCustomerNode(pid);
  if(*head == NULL)
  {
    *head = newCustomerNode;
    return;
  }
  (*head)->prev = newCustomerNode;
  newCustomerNode->next = *head;
  *head = newCustomerNode;
}