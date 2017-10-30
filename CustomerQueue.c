#include "CustomerQueue.h"


CustomerNode* NewCustomerNode(Customer* customer)
{
  CustomerNode* newCustomerNode = (CustomerNode*)malloc(sizeof(CustomerNode));
  
  newCustomerNode->customer = customer;
  newCustomerNode->next = NULL;
  printf("made new customer Node the customer has ID, %d\n", customer->ID);
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
  printf("customer ID: %3d , ", currentCustomerNode.customer->ID);
  while(currentCustomerNode.next != NULL )
  {
    currentCustomerNode = *(currentCustomerNode.next);
    printf("Customer ID: %3d ,", currentCustomerNode.customer->ID);
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
/*Customer CopyOf(Customer customerToCopy)
{
  Customer customer = (Customer)malloc(sizeof(Customer));
  customer.ID = customerToCopy.ID;
  customer.arrivalTime = customerToCopy.arrivalTime;
  customer.serviceTime = customerToCopy.serviceTime;
  return customer;
}*/

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
  printf("inserting Customer into queue with ID: %d\n", newCustomerNode->customer->ID);
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