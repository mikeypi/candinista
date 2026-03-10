#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "d3-array.h"


static inline int index3d (d3_array* d3, unsigned i, unsigned j, unsigned k) 
{
  return (i * (d3 -> x_dimension * d3 -> y_dimension) + j * (d3 -> x_dimension) + k);
}

d3_array* new_d3_array (int x_dimension, int y_dimension, int z_dimension ) {
  d3_array* new = calloc (1, sizeof (d3_array));
  
  new -> x_dimension = x_dimension;
  new -> y_dimension = y_dimension;
  new -> z_dimension = z_dimension;
  
  new -> items = calloc (x_dimension * y_dimension * z_dimension, sizeof *new -> items);
  return (new);
}

void free_d3_array (d3_array* d3) {
  free (d3 -> items);
  free (d3);
}

uintptr_t* get_item_in_d3_array  (d3_array* d3, int i, int j, int k) {
  return (d3 -> items[index3d (d3, i, j, k)]);
}

void set_item_in_d3_array  (d3_array* d3, uintptr_t* item, int i, int j, int k) {
  d3 -> items[index3d (d3, i, j, k)] = item;
}
