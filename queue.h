//SSOO-P3 23/24

#ifndef HEADER_FILE
#define HEADER_FILE


struct operation {
  int product_id; //Product identifier
  int op;         //Operation
  int units;      //Product units
};

typedef struct queue {
  // Define the struct yourself
  int max_size, size;
  struct operation *array;
  int head;
  int tail;
}queue;

queue* queue_init (int size);
int queue_destroy (queue *q);
int queue_put (queue *q, struct operation* elem);
struct operation * queue_get(queue *q);
int queue_empty (queue *q);
int queue_full(queue *q);

#endif