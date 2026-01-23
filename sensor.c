#include <stdio.h>
#include "sensor.h"
#include <stdlib.h>
#include <string.h>

struct Sensor {
  char  *name;
  double value;
  int can_id;
  int can_data_offset;
  int can_data_width;
  double* x_values;
  double* y_values;
  int n_values;
  int x_index;
  int y_index;
  int z_index;
  int id;
  int atomic;
};

Sensor *sensor_create (int x_index, int y_index, const char *name, int can_id, int offset, int width) {
  Sensor *s = calloc (1, sizeof *s);
  s -> x_index = x_index;
  s -> y_index = y_index;
  s -> name  = strdup (name);
  s -> can_id = can_id;
  s -> can_data_offset = offset;
  s -> can_data_width = width;
  s -> atomic = 0;
  
  return s;
}

void sensor_destroy (Sensor *s) {
  fprintf (stderr, "freeing %s\n", s -> name);
  free (s -> name);
  free (s -> x_values);
  free (s -> y_values);
  free (s);
}

const char *sensor_get_name (const Sensor *s)                         { return s -> name; }
int sensor_get_can_id (const Sensor *s)                               { return s -> can_id; }
int sensor_get_can_data_offset (const Sensor *s)                      { return s -> can_data_offset; }
int sensor_get_can_data_width (const Sensor *s)                       { return s -> can_data_width; }
double* sensor_get_x_values (const Sensor *s)                         { return s -> x_values; }
double* sensor_get_y_values (const Sensor *s)                         { return s -> y_values; }
int sensor_get_n_values (const Sensor *s)                             { return s -> n_values; };
int sensor_get_id (const Sensor *s)                          { return s -> id; }
int sensor_get_atomic (const Sensor *s)                      { return s -> atomic; }

int sensor_get_x_index (const Sensor *s)                              { return s -> x_index; }
int sensor_get_y_index (const Sensor *s)                              { return s -> y_index; }
int sensor_get_z_index (const Sensor *s)                              { return s -> z_index; }

void sensor_set_id (Sensor *s, int id)                       { s -> id = id; }
void sensor_set_x_values (Sensor *s, double *v, int n)                { s -> x_values = v; s -> n_values = n; };
void sensor_set_y_values (Sensor *s, double *v, int n)                { s -> y_values = v; s -> n_values = n; };

