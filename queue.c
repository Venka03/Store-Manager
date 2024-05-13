//SSOO-P3 23/24

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "queue.h"

//To create a queue
queue* queue_init(int size)
{
  queue * q = (queue *)malloc(sizeof(queue)); // allocate space for object queue
  q->array = (struct operation *)malloc(sizeof(struct operation)*size); // allocate space for array that will be used as queue
  q->max_size = size;
  return q;
}

// To Enqueue an element
int queue_put(queue *q, struct operation* x)
{
  if (q->size == 0){ // check if queue is empty, thus setting head and tail and adding element
    q->head = 0;
    q->tail = 0;
    q->array[q->tail] = *x; // assign to tail position the element
    q->size++;
  }
  else if(q->size == q->max_size){ // in case when queue is already full
    perror("ERROR: queue is full");
    exit(-1);
  }
  else {
    q->tail = (q->tail + 1) % q->max_size; // queue is circular array
    q->array[q->tail] = *x; // assign to the new tail position the element
    q->size++;
  }
  return 0;
}

// To Dequeue an element.
struct operation* queue_get(queue *q)
{
  if (q->size == 0){
    perror("ERROR: queue is empty");
    exit(-1);
  }
  struct operation* element = (struct operation *)malloc(sizeof(struct operation)); // allocate space for return object
  *element = q->array[q->head];
  q->head = (q->head + 1) % q->max_size; // move to next
  q->size--;
  return element;
}

//To check if queue is empty
int queue_empty(queue *q){
  return q->size == 0;
}
//To check if queue is full
int queue_full(queue *q) {
  return q->size == q->max_size;
}

//destroy the queue and free the resources
int queue_destroy(queue *q)
{
  free(q->array);
  free(q);
  return 0;
}