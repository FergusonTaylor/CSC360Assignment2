#include "CustomerQueue.h"

CustomerNode* NewCustomerNode(Customer customer)
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
  printf("customer ID: %3d , ", currentCustomerNode.customer.ID);
  while(currentCustomerNode.next != NULL )
  {
    currentCustomerNode = *(currentCustomerNode.next);
    printf("Customer ID: %3d ,", currentCustomerNode.customer.ID);
  }
  printf("\n");
}
CustomerNode* FindCustomerNodePointer(Customer customer, CustomerNode* head)
{
  CustomerNode* currentCustomerNode = head;
  while(currentCustomerNode !=NULL)
  {
    if(currentCustomerNode->customer.ID == customer.ID)
    {
      break;
    }
    currentCustomerNode = currentCustomerNode->next;
  }
  return currentCustomerNode;
}
Customer PopCustomerNode(CustomerNode** headPointer)
{
    Customer customer = (*headPointer)->customer;
    CustomerNode* toFree = (*headPointer);
    *headPointer = (*headPointer)->next;
    //not sure if this frees the customer
    //free(toFree);
    return customer;
}
void InsertAtTail(Customer customer, CustomerNode** head)
{
  CustomerNode* newCustomerNode = NewCustomerNode(customer);
  if(*head == NULL)
  {
    *head = newCustomerNode;
    return;
  }
  CustomerNode* currentCustomerNode = *head;
  while(currentCustomerNode->next != NULL)
  {
      currentCustomerNode = currentCustomerNode->next;
  }
  currentCustomerNode->next = newCustomerNode;

}
int main()
{

}