//SSOO-P3 23/24

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "queue.h"

//To create a queue
queue* queue_init(int size)
{
  //queue * q = (queue *)malloc(size * sizeof(struct element));
  queue * q = (queue *)malloc(sizeof(queue));
  q->array = (struct element *)malloc(sizeof(struct element)*size);
  q->max_size = size;
  return q;
}

// To Enqueue an element
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
    q->tail++;
    q->array[q->tail] = *x;
    q->size++;
  }
  return 0;
}

// To Dequeue an element.
struct element* queue_get(queue *q)
{
  struct element* element = (struct element *)malloc(sizeof(struct element)); // should be like this to avoid problem
  *element = q->array[q->head];
  q->head++; // move to next
  q->size--;
  return element;
}

//To check queue state
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

//To destroy the queue and free the resources
int queue_destroy(queue *q)
{
  return 0;
}


int main(){
  queue * q = queue_init(5);
  struct element elem;
  elem.product_id = 1;
  elem.op = 2;
  elem.units = 4;
  queue_put(q, &elem);

  elem.product_id = 4;
  elem.op = 4;
  elem.units = 4;
  queue_put(q, &elem);

  elem.product_id = 5;
  elem.op = 5;
  elem.units = 5;
  queue_put(q, &elem);

  struct element *elem1 =  queue_get(q);

  printf("%d %d %d\n", elem1->product_id, elem1->op, elem1->units);
  return 0;
}