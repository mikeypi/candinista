#ifndef D3_ARRAY_H
#define D3_ARRAY_H

typedef union {
  void* vp;
  int i[2];
} item;
  
typedef struct {
  int x_dimension;
  int y_dimension;
  int z_dimension;
  item** items;
  int n_items;
} d3_array;

d3_array* new_d3_array (int x_dimension, int y_dimension, int z_dimension);
void* get_item_in_d3_array  (d3_array* d3, int i, int j, int k);
void  set_item_in_d3_array  (d3_array* d3, void* item, int i, int j, int k);
static inline int get_x_dimension_from_d3_array  (d3_array* d3) { return (d3 -> x_dimension); }
static inline int get_y_dimension_from_d3_array  (d3_array* d3) { return (d3 -> y_dimension); }
static inline int get_z_dimension_from_d3_array  (d3_array* d3) { return (d3 -> z_dimension); }
#endif

