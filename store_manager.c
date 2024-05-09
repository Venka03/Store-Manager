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
pthread_mutex_t mutex; /* mutex to access shared buffer */
pthread_cond_t non_full; /* can we add more elements? */
pthread_cond_t non_empty; /* can we remove elements? */






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
      printf("Line read: %s\n", buffer);
      break;
    }
  }
  int size = atoi(buffer);
  DATA_TO_READ = size;
  printf("%d\n", size);
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
void* add_to_queue(){
    int i=0;
    while (i < DATA_TO_READ){
        pthread_mutex_lock(&mutex);
        while (queue_full(q))
            pthread_cond_wait(&non_full, &mutex);
        
        queue_put(q, elements+i);
        i++;
        pthread_cond_signal(&non_empty); /* buffer is not empty */
        pthread_mutex_unlock(&mutex);
    }
    pthread_exit(0);
}

void* take_from_queue(void* profit){
    struct element* el;
    int i=0;
    //int* arr = (int *)profit;
    while (i < DATA_TO_READ){
        pthread_mutex_lock(&mutex); /* access to buffer */
        while (queue_empty(q))
            pthread_cond_wait(&non_empty, &mutex);
        
        i++;
        el = queue_get(q);
        printf("%d, %d, %d\n", el->product_id, el->op, el->units);
        int money = 0;
        if (el->op == 0){
            switch (el->product_id)
            {
            case 1:
                money = el->units * 3;
                *((int *)profit + el->op) += money;
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
            //arr[el->product_id-1] += money;
            //*((int *)profit + el->product_id-1) += money;
            (*(int *)profit) += money;
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
            (*(int *)profit) -= money;
            //*((int *)profit + el->product_id-1) -= money;
            //arr[el->product_id-1] -= money;
        }
        pthread_cond_signal(&non_full); /* buffer is not full */
        pthread_mutex_unlock(&mutex);
    }
    /*
    for (int i=0; i<50; i++){
        sem_wait(&elem);
        el = queue_get(q);
        printf("%d, %d, %d\n", el->product_id, el->op, el->units);
        int money;
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
        default:
            break;
        }
        (*(int *)profit) += money;
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
        default:
            break;
        }
        (*(int *)profit) -= money;
        }
        sem_post(&hollow);
    }
    */
    pthread_exit(0);

}

int main (int argc, const char * argv[]){
    int profits = 0;
    int product_stock [5] = {0};

    // struct element *elements = save_data("file.txt");

    elements = save_data("file.txt");
    q = queue_init(10);

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&non_full, NULL);
    pthread_cond_init(&non_empty, NULL);

    pthread_t write, read;
    pthread_create(&write, NULL, add_to_queue, NULL);
    pthread_create(&read, NULL, take_from_queue, (void *)&profits);
    pthread_join(write, NULL);
    pthread_join(read, NULL);

    pthread_mutex_destroy(&mutex);
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