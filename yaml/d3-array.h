#ifndef D3_ARRAY_H
#define D3_ARRAY_H

typedef struct {
  int x_dimension;
  int y_dimension;
  int z_dimension;
  void** items;
  int n_items;
} d3_array;

d3_array* new_d3_array (int x_dimension, int y_dimension, int z_dimension);
void* get_item_in_d3_array  (d3_array* d3, int i, int j, int k);
void  set_item_in_d3_array  (d3_array* d3, void* item, int i, int j, int k);
int get_x_dimension_from_d3_array  (d3_array* d3);
int get_y_dimension_from_d3_array  (d3_array* d3);
int get_z_dimension_from_d3_array  (d3_array* d3);

typedef struct {
  int x_dimension;
  int y_dimension;
  void** items;
  int n_items;
} d2_vp_array;

d2_vp_array* new_d2_vp_array (int x_dimension, int y_dimension);
void* get_item_in_d2_vp_array  (d2_vp_array* d2, int i, int j);
void  set_item_in_d2_vp_array  (d2_vp_array* d2, void* item, int i, int j);
int get_x_dimension_from_d2_vp_array  (d2_vp_array* d2);
int get_y_dimension_from_d2_vp_array  (d2_vp_array* d2);
int get_z_dimension_from_d2_vp_array  (d2_vp_array* d2);

typedef struct {
  int x_dimension;
  int y_dimension;
  int* items;
  int n_items;
} d2_int_array;

d2_int_array* new_d2_int_array (int x_dimension, int y_dimension);
int get_item_in_d2_int_array  (d2_int_array* d2, int i, int j);
void  set_item_in_d2_int_array  (d2_int_array* d2, int item, int i, int j);
int get_x_dimension_from_d2_int_array  (d2_int_array* d2);
int get_y_dimension_from_d2_int_array  (d2_int_array* d2);
int get_z_dimension_from_d2_int_array  (d2_int_array* d2);

#endif

