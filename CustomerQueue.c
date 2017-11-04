#include "CustomerQueue.h"

//Customer Queue functions for adding and removing customer nodes from a given queue
//pretty standard queue stuff
CustomerNode* NewCustomerNode(Customer* customer)
{
  CustomerNode* newCustomerNode = (CustomerNode*)malloc(sizeof(CustomerNode));
  
  newCustomerNode->customer = customer;
  newCustomerNode->next = NULL;
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
  while(currentCustomerNode.next != NULL )
  {
    currentCustomerNode = *(currentCustomerNode.next);
  }
  printf("\n");
}
CustomerNode* FindCustomerNodePointer(Customer customer, CustomerNode* head)
{
  CustomerNode* currentCustomerNode = head;
  while(currentCustomerNode !=NULL)
  {
    if(currentCustomerNode->customer->ID == customer.ID)
    {
      break;
    }
    currentCustomerNode = currentCustomerNode->next;
  }
  return currentCustomerNode;
}


Customer* PopCustomerNode(CustomerNode** headPointer)
{
    if(*headPointer == NULL)
    {
      return NULL;
    }
    Customer* customer = (*headPointer)->customer;    
    CustomerNode* toFree = (*headPointer);
    *headPointer = (*headPointer)->next;

    free(toFree);
    return customer;
}
void InsertAtTail(Customer* customer, CustomerNode** head)
{
  CustomerNode* newCustomerNode = NewCustomerNode(customer);
  if(*head == NULL)
  {
    *head = newCustomerNode;
    return;
  }
  CustomerNode* currentCustomerNode = (*head);
  while(currentCustomerNode->next != NULL)
  {
      currentCustomerNode = currentCustomerNode->next;
  }
  currentCustomerNode->next = newCustomerNode;
}