#include <stdio.h>
#include <stdlib.h>

#include "d3-array.h"


static inline int index3d (d3_array* d3, unsigned i, unsigned j, unsigned k) 
{
  return (i * (d3 -> x_dimension * d3 -> y_dimension) + j * (d3 -> x_dimension) + k);
}

static inline int index2d (void* d2, unsigned i, unsigned j) 
{
  d2_vp_array* d2vp = (d2_vp_array*) d2;
  return (i * (d2vp -> y_dimension) + j);
}

d3_array* new_d3_array (int x_dimension, int y_dimension, int z_dimension ) {
  d3_array* new = calloc (1, sizeof (d3_array));
  
  new -> x_dimension = x_dimension;
  new -> y_dimension = y_dimension;
  new -> z_dimension = z_dimension;
  
  new -> items = calloc (x_dimension * y_dimension * z_dimension, sizeof (void*));
  return (new);
}

void free_d3_array (d3_array* d3) {
  free (d3 -> items);
  free (d3);
}

void* get_item_in_d3_array  (d3_array* d3, int i, int j, int k) {
  return (d3 -> items[index3d (d3, i, j, k)]);
}

void set_item_in_d3_array  (d3_array* d3, void* item, int i, int j, int k) {
  d3 -> items[index3d (d3, i, j, k)] = item;
}

d2_vp_array* new_d2_vp_array (int x_dimension, int y_dimension) {
  d2_vp_array* new = calloc (1, sizeof (d2_vp_array));
  
  new -> x_dimension = x_dimension;
  new -> y_dimension = y_dimension;
  
  new -> items = calloc (x_dimension * y_dimension, sizeof (void*));
  return (new);
}

void free_d2_vp_array (d2_vp_array* d2) {
  free (d2 -> items);
  free (d2);
}

void* get_item_in_d2_vp_array  (d2_vp_array* d2, int i, int j) {
  return (d2 -> items[index2d (d2, i, j)]);
}

void set_item_in_d2_vp_array  (d2_vp_array* d2, void* item, int i, int j) {
  d2 -> items[index2d (d2, i, j)] = item;
}

d2_int_array* new_d2_int_array (int x_dimension, int y_dimension) {
  d2_int_array* new = calloc (1, sizeof (d2_vp_array));
  
  new -> x_dimension = x_dimension;
  new -> y_dimension = y_dimension;
  
  new -> items = calloc (x_dimension * y_dimension, sizeof (void*));
  return (new);
}

void free_d2_int_array (d2_int_array* d2) {
  free (d2 -> items);
  free (d2);
}

int get_item_in_d2_int_array  (d2_int_array* d2, int i, int j) {
  return (d2 -> items[index2d (d2, i, j)]);
}

void set_item_in_d2_int_array  (d2_int_array* d2, int item, int i, int j) {
  d2 -> items[index2d (d2, i, j)] = item;
}


