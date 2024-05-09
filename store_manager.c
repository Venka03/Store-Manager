//SSOO-P3 23/24

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stddef.h>
#include <sys/stat.h>
#include <pthread.h>
#include "queue.h"
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <semaphore.h>


/*
struct element {
  int product_id; //Product identifier
  int op;         //Operation
  int units;      //Product units
};
typedef struct queue {
  // Define the struct yourself
  int max_size, size;
  struct element *array;
  int head;
  int tail;
}queue;

// queue.c
queue* queue_init(int size)
{
  //queue * q = (queue *)malloc(size * sizeof(struct element));
  queue * q = (queue *)malloc(sizeof(queue));
  q->array = (struct element *)malloc(sizeof(struct element)*size);
  q->max_size = size;
  return q;
}
int queue_put(queue *q, struct element* x)
{
  if (q->size == 0){
    q->head = 0;
    q->tail = 0;
    q->array[q->head] = *x;
    q->size++;
  }
  else if(q->size == q->max_size){
    perror("ERROR: queue is full");
    exit(-1);
  }
  else {
    q->tail = (q->tail + 1) % q->max_size;
    q->array[q->tail] = *x;
    q->size++;
  }
  return 0;
}
struct element* queue_get(queue *q)
{
  struct element* element = (struct element *)malloc(sizeof(struct element)); // should be like this to avoid problem
  *element = q->array[q->head];
  q->head = (q->head + 1) % q->max_size; // move to next
  q->size--;
  return element;
}
int queue_empty(queue *q) {
  if (q->size == 0)
    return 1;
  return 0;
}
int queue_full(queue *q) {
  if (q->size == q->max_size)
    return 1;
  return 0;
}
int queue_destroy(queue *q)
{
  return 0;
}
*/



struct element *elements;
queue *q;
int DATA_TO_READ;
pthread_mutex_t write_mutex; /* mutex to access shared buffer */
pthread_mutex_t read_mutex;
pthread_cond_t non_full; /* can we add more elements? */
pthread_cond_t non_empty; /* can we remove elements? */

struct ThreadArgs {
    int* arg1;
    int* arg2;
};


struct element* save_data(char name[]){
  /*
  save data from the file about operations. The structure is following
  <product id><operation type><units>
  save it as array of elements and return pointer to it.
  */
  int fd = open(name, O_RDONLY);
  if (fd == -1){
    perror("ERROR: open file");
    exit(-1);
  }
  char buffer[128];
  int n_read, i=0;
  while ((n_read = read(fd, buffer+i, 1)) > 0){ // get amount of data to read
    i++;
    if (buffer[i-1] == '\n'){
      buffer[i-1] = '\0';
      break;
    }
  }
  int size = atoi(buffer);
  DATA_TO_READ = size;
  struct element *elements = (struct element *)malloc(sizeof(struct element)*size); // used to store operations(?)
  int j = 0;
  i = 0;
  int word = 0;
  while ((n_read = read(fd, buffer+i, 1)) > 0 && j < size){
    i++;
    if (buffer[i-1] == ' ' || buffer[i-1] == '\n'){
      buffer[i-1] = '\0';
      if (word == 0)
        elements[j].product_id = atoi(buffer); // add number check
      else if (word == 1)
        elements[j].op = strcmp(buffer, "SALE"); // op == 0 is sale, other is purchase
      else {
        elements[j].units = atoi(buffer);
        j++;
      }
        
      word = (word + 1) % 3;
      i = 0;
    }
  }
  return elements;
}
void* producer(void* arg){
    struct ThreadArgs* args = (struct ThreadArgs*) arg;
    int i=*(args->arg1);
    while (i < *args->arg2){
        pthread_mutex_lock(&write_mutex);
        while (queue_full(q)){
            pthread_cond_wait(&non_full, &write_mutex);
        }
        
        queue_put(q, elements+i);
        i++;
        
        pthread_cond_signal(&non_empty); /* buffer is not empty */
        
        //pthread_mutex_lock(&read_mutex);
        pthread_mutex_unlock(&write_mutex);
        //pthread_mutex_unlock(&read_mutex);   
    }
    pthread_exit(0);
}

int elements_read = 0;

void* consumer(void* arg){
    struct ThreadArgs* args = (struct ThreadArgs*) arg;
    struct element* el;
    //int i=0;
    //int* arr = (int *)profit;
    while (1){
        pthread_mutex_lock(&read_mutex); /* access to buffer */
        if (elements_read >= DATA_TO_READ){
            pthread_mutex_unlock(&read_mutex); // so another thread can finish its execution
            //pthread_cond_broadcast(&non_empty);
            break;
        }
        
        while (queue_empty(q)){
            pthread_cond_wait(&non_empty, &read_mutex);
        }
            
        
        //i++;
        elements_read++;
        el = queue_get(q);
        //pthread_mutex_unlock(&read_mutex);
        int money = 0;
        if (el->op == 0){
            switch (el->product_id)
            {
            case 1:
                money = el->units * 3;
                break;
            case 2:
                money = el->units * 10;
                break;
            case 3:
                money = el->units * 20;
                break;
            case 4:
                money = el->units * 40;
                break;
            case 5:
                money = el->units * 125;
                break;
            }
            args->arg1[el->product_id-1] -= el->units;
            //arr[el->product_id-1] += money;
            //*((int *)profit + el->product_id-1) += money;
            *args->arg2 += money;
            //(*(int *)profit) += money;
        }
        else {
            switch (el->product_id)
            {
            case 1:
                money = el->units * 2;
                break;
            case 2:
                money = el->units * 5;
                break;
            case 3:
                money = el->units * 15;
                break;
            case 4:
                money = el->units * 25;
                break;
            case 5:
                money = el->units * 100;
                break;
            }
            args->arg1[el->product_id-1] += el->units;
            *args->arg2 -= money;
            //(*(int *)profit) -= money;
            //*((int *)profit + el->product_id-1) -= money;
            //arr[el->product_id-1] -= money;
        }
        
        
        pthread_mutex_unlock(&read_mutex);

        pthread_mutex_lock(&write_mutex);
        pthread_cond_signal(&non_full); /* buffer is not full */
        pthread_mutex_unlock(&write_mutex);
        
        
    }
    pthread_exit(0);

}

int main (int argc, char * argv[]){
    int profits = 0;
    int product_stock [5] = {0};
    // struct element *elements = save_data("file.txt");
    int q_length = 10, amount_p = 3, amount_c = 3;

    if (argc == 5){
        elements = save_data(argv[1]);
        amount_p = atoi(argv[2]);
        amount_c = atoi(argv[3]);
        q_length = atoi(argv[4]);
    }
    else {
        perror("ERROR: wrong input data");
        return -1;
    }
    q = queue_init(q_length);

    pthread_mutex_init(&write_mutex, NULL);
    pthread_mutex_init(&read_mutex, NULL);
    pthread_cond_init(&non_full, NULL);
    pthread_cond_init(&non_empty, NULL);

    pthread_t write[amount_p], read[amount_c];
    for (int i=0; i<amount_p; i++){
        struct ThreadArgs* args = malloc(sizeof(struct ThreadArgs));
        args->arg1 = malloc(sizeof(int));
        *args->arg1 = i*(DATA_TO_READ/amount_p);
        args->arg2 = malloc(sizeof(int));
        *args->arg2 = (i+1)*(DATA_TO_READ/amount_p);
        if (i == amount_p-1)
            args->arg2 = &DATA_TO_READ;
        pthread_create(&write[i], NULL, producer, (void*)args);
    }
    //struct ThreadArgs args = {0, DATA_TO_READ};
    //pthread_create(&write, NULL, producer, (void*)&args);
    for (int i=0; i<amount_c; i++){
        struct ThreadArgs* args = malloc(sizeof(struct ThreadArgs));
        args->arg1 = malloc(5 * sizeof(int));
        args->arg1 = product_stock;
        args->arg2 = &profits;
        pthread_create(&read[i], NULL, consumer, (void*)args);
    }

        
    for (int i=0; i<amount_c; i++)
        pthread_join(write[i], NULL);
    
    for (int i=0; i<amount_c; i++)
        pthread_join(read[i], NULL);
    
    //pthread_join(read, NULL);

    pthread_mutex_destroy(&read_mutex);
    pthread_mutex_destroy(&write_mutex);
    pthread_cond_destroy(&non_full);
    pthread_cond_destroy(&non_empty);

    // Output
    printf("Total: %d euros\n", profits);
    printf("Stock:\n");
    printf("  Product 1: %d\n", product_stock[0]);
    printf("  Product 2: %d\n", product_stock[1]);
    printf("  Product 3: %d\n", product_stock[2]);
    printf("  Product 4: %d\n", product_stock[3]);
    printf("  Product 5: %d\n", product_stock[4]);

    return 0;
}