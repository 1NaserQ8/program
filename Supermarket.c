#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>

#define MaxItems 2 // Maximum items a cashier can scan or a customer can add
#define BufferSize 2 // Size of the buffer

sem_t empty;
sem_t full;
int in = 0;
int out = 0;
int buffer[BufferSize];
pthread_mutex_t mutex;	// maintain mutual exclusion

void *customer(void *pno)
{   
    int item;
    for(int i = 0; i < MaxItems; i++) {
        item = rand()%100; // Produce a random item

      //  take an action regarding the customer 
      //  customer must wait for empty slot before adding an item
        
        buffer[in] = item;
        printf("Customer %d: adds Item %d to cashier counter %d\n", *((int *)pno),buffer[in],in);
        in = (in+1)%BufferSize;

        //  customer must release the cashier counter to be accessed by other customer 
    }
}
void *cashier(void *cno)
{   
    for(int i = 0; i < MaxItems; i++) {

      //  take an action regarding the cashier 
      //  cashier must wait for an item to be added

        int item = buffer[out];
        printf("Cashier %d: scans Item %d from cashier counter %d\n",*((int *)cno),item, out);
        out = (out+1)%BufferSize;
        
     //  cashier must empty the counter by removing an item  
    }
}

int main()
{   

    pthread_t cust[2],cash[2];

    // Define and initiate your mutex here
	
    int a[2] = {1,2}; //Just used for numbering the cashiers and customers

    for(int i = 0; i < 2; i++) {
        pthread_create(&cust[i], NULL, (void *)customer, (void *)&a[i]);
    }
    for(int i = 0; i < 2; i++) {
        pthread_create(&cash[i], NULL, (void *)cashier, (void *)&a[i]);
    }

    for(int i = 0; i < 2; i++) {
        pthread_join(cust[i], NULL);
    }
    for(int i = 0; i < 2; i++) {
        pthread_join(cash[i], NULL);
    }

  //  clean up here;


    return 0;
    
}
