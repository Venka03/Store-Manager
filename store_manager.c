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
#include <errno.h>


int is_numeric(char *str){
    /*
    check if string is numerical or not
    meaning if string(array of chars) contain integer number or not
    */
    if (str[0] == '\0')
        return 0;
    int i = 0;
    if (str[0] == '-'){ // for case of negative number
        if (str[1] == '\0')
            return 0;
        i++; // move staring position of checking numbers
    }
        
    for (; str[i]!='\0'; i++){ // read till the end of string(array of chars)
        if (str[i] < '0' || str[i] > '9')
            return 0;
    }
    return 1;
}


struct operation *elements;
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


struct operation* save_data(char name[]){
  /*
  save data from the file about operations. The structure is following
  <product id><operation type><units>
  save it as array of elements and return it.
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
  if (!is_numeric(buffer)){
    perror("ERROR: wrong structure of the file: non numerical amount of operations to read\n");
    exit(-1);
  }
  DATA_TO_READ = atoi(buffer);
  if (DATA_TO_READ <= 0){ 
    perror("ERROR: amount of operations should be positive\n");
    exit(-1);
  }
  struct operation *elements = (struct operation *)malloc(sizeof(struct operation)*DATA_TO_READ); // used to store operations
  int j = 0;
  i = 0;
  int word = 0;
  while ((n_read = read(fd, buffer+i, 1)) > 0 && j < DATA_TO_READ){ // read data until the amount needed is read or file ends
    i++;
    if (buffer[i-1] == ' ' || buffer[i-1] == '\n'){
      buffer[i-1] = '\0';
      if (word == 0) { // first word in line is the id of product
        if (!is_numeric(buffer)){
            perror("ERROR: wrong structure of the file\n");
            exit(-1);
        }
        elements[j].product_id = atoi(buffer);
      }
      else if (word == 1) // second word in line is the operation performed
        if (strcmp(buffer, "SALE") == 0) // op == 0 is sale, other is purchase
          elements[j].op = 0;
        else if (strcmp(buffer, "PURCHASE") == 0)
          elements[j].op = 1;
        else {
          perror("ERROR: wrong structure of the file");
          exit(-1);
        }
         
      else if (word == 2) { // third word in line is the amount of products
        if (!is_numeric(buffer)){
            perror("ERROR: wrong structure of the file\n");
            exit(-1);
        }
        elements[j].units = atoi(buffer);
        j++;
      }
      word = (word + 1) % 3;
      i = 0;
    }
  }
  return elements;
}

int amount_produced = 0;

void* producer(void* arg){
    /*
    function for threads. 
    Receive as parameter the beginning index of the interval from which to read data, 
    and the length of the interval
    Read data from array of operations and save it to queue.
    Control access to critical data, to avoid problem with values of data
    */
    struct ThreadArgs* args = (struct ThreadArgs*) arg; // get arguments
    int i=*(args->arg1); // beginning of the interval
    while (i < *args->arg2 + *args->arg1){ // for every index in range(for every element of array in corresponding range)
        pthread_mutex_lock(&write_mutex); // block access of other writers
        while (queue_full(q)){ // wait for queue to be not full, to be able to add data
            pthread_cond_wait(&non_full, &write_mutex);
        }
        
        queue_put(q, elements+i);
        amount_produced++;
        i++;
        
        pthread_cond_signal(&non_empty); // signal that queue is not empty
        pthread_mutex_unlock(&write_mutex);
    }
    pthread_exit(0);
}

int elements_read = 0;

void* consumer(void* arg){
    /*
    function for threads. 
    Read data from queue, analyze it and modify profit and stock
    Control access to critical data, to avoid problem with values of data
    */
    struct ThreadArgs* args = (struct ThreadArgs*) arg; // get parameters passed
    struct operation* el;
    
    while (1){
        pthread_mutex_lock(&read_mutex); // block access to queue 
        // if the amount of elements read is same(or grater, which in general should not occur) as amount needed to be read
        if (elements_read >= DATA_TO_READ){ 
            pthread_cond_broadcast(&non_empty); // to prevent other thread from getting stuck.
            pthread_mutex_unlock(&read_mutex); // so another thread can finish its execution
            pthread_exit(0);
        }
        while (queue_empty(q)){ 
            if (amount_produced==DATA_TO_READ){ // to avoid consumers waiting for ever, when there are no producer
                pthread_cond_broadcast(&non_empty); 
                pthread_mutex_unlock(&read_mutex);
                pthread_exit(0);
            }
            pthread_cond_wait(&non_empty, &read_mutex); // wait for queue not to be empty, to read something
        }
        elements_read++;
        el = queue_get(q);
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
            default:
                perror("ERROR: wrong user id");
                pthread_exit(0);
            }
            args->arg1[el->product_id-1] -= el->units; // products are sold, so stock decreases
            *args->arg2 += money; // products are sold, so profit increases
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
                perror("ERROR: wrong user id");
                pthread_exit(0);
            }
            args->arg1[el->product_id-1] += el->units; // products are purchased, so stock increases
            *args->arg2 -= money; // products are purchased, so profit decreased
        }
        
        pthread_cond_signal(&non_full); // signal that queue is not full
        pthread_mutex_unlock(&read_mutex); // give access to other consumers
    }
    pthread_exit(0);
}

int main(int argc, char * argv[]){
    /*
    receive the file from which to read data, number of producers, consumers and length of the queue
    create threads to execute production and consuming, and execute them
    Control of work of threads in performed, including error handling.
    print the profit and amount of products left in stock
    */
    int profits = 0;
    int product_stock [5] = {0};
    int q_length, amount_p, amount_c;

    if (argc == 5){
        elements = save_data(argv[1]); // get filename 
        if (!is_numeric(argv[2]) || !is_numeric(argv[3]) || !is_numeric(argv[4])){ // check if parameters are numbers
          perror("ERROR: wrong input data\n");
          return -1;
        }
        amount_p = atoi(argv[2]); // get amount of producers 
        amount_c = atoi(argv[3]); // get amount of consumers 
        q_length = atoi(argv[4]); // get size of the queue 
        if (amount_c <= 0 || amount_p <= 0 || q_length <= 0){
          perror("ERROR: wrong input data: numbers should be positive\n");
          return -1;
        }
    }
    else {
        perror("ERROR: wrong amount of input data\nstructure: <file name><num producers><num consumers><buff size>\n");
        return -1;
    }
    q = queue_init(q_length);

    // initialize all mutexes and conditional variables used during the execution
    // handle errors if ones occur
    int thread_error_result;
    thread_error_result = pthread_mutex_init(&write_mutex, NULL);
    if (thread_error_result != 0) {
        perror("ERROR: mutex initialization");
        return -1;
    }
    thread_error_result = pthread_mutex_init(&read_mutex, NULL);
    if (thread_error_result != 0) {
        perror("ERROR: mutex initialization");
        return -1;
    }
    thread_error_result = pthread_cond_init(&non_full, NULL);
    if (thread_error_result != 0) {
        perror("ERROR: conditional value initialization");
        return -1;
    }
    thread_error_result = pthread_cond_init(&non_empty, NULL);
    if (thread_error_result != 0) {
        perror("ERROR: conditional value initialization");
        return -1;
    }


    pthread_t write[amount_p], read[amount_c]; // create array of threads (thread pools)
    
    // producer thread pool
    for (int i=0; i<amount_p; i++){
        struct ThreadArgs* args = malloc(sizeof(struct ThreadArgs));
        args->arg1 = malloc(sizeof(int));
        *args->arg1 = i*(DATA_TO_READ/amount_p); // define start of the interval of 
        args->arg2 = malloc(sizeof(int));
        *args->arg2 = DATA_TO_READ/amount_p;
        if (i >= amount_p - DATA_TO_READ % amount_p){
            *args->arg2 += 1; // the length of last (DATA_TO_READ % amount_p) data will be bigger by one than others
            *args->arg1 += i - (amount_p - DATA_TO_READ % amount_p); // start also should increase, by 1 each increment is increased
        }
        thread_error_result = pthread_create(&write[i], NULL, producer, (void*)args);
        if (thread_error_result != 0){
          perror("ERROR: pthread creation");
          return -1;
        }
    }

    // consumer thread pool
    for (int i=0; i<amount_c; i++){
        struct ThreadArgs* args = malloc(sizeof(struct ThreadArgs));
        args->arg1 = malloc(5 * sizeof(int));
        args->arg1 = product_stock;
        args->arg2 = &profits;
        thread_error_result = pthread_create(&read[i], NULL, consumer, (void*)args);
        if (thread_error_result != 0){
          perror("ERROR: pthread creation");
          return -1;
        }
    }

        
    for (int i=0; i<amount_p; i++)
        pthread_join(write[i], NULL);
    
    for (int i=0; i<amount_c; i++)
        pthread_join(read[i], NULL);
    
    // destruction of mutexes and conditional variables
    thread_error_result = pthread_mutex_destroy(&read_mutex);
    if (thread_error_result != 0) {
        perror("ERROR: mutex destruction");
        return -1;
    }
    thread_error_result = pthread_mutex_destroy(&write_mutex);
    if (thread_error_result != 0) {
        perror("ERROR: mutex destruction");
        return -1;
    }
    thread_error_result = pthread_cond_destroy(&non_full);
    if (thread_error_result != 0) {
        perror("ERROR: conditional value destruction");
        return -1;
    }
    thread_error_result = pthread_cond_destroy(&non_empty);
    if (thread_error_result != 0) {
        perror("ERROR: conditional value destruction");
        return -1;
    }

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