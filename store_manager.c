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
  printf("%d\n", size);
  struct element *elements = (struct element *)malloc(sizeof(struct element)*size); // used to store operations(?)
  int num_elem = 0;
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


int main (int argc, const char * argv[]){

  int profits = 0;
  int product_stock [5] = {0};
  
  struct element *elements = save_data("file.txt");
  





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
