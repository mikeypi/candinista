#include "sensor.h"
#include <stdlib.h>
#include <string.h>

struct Sensor {
  char  *name;
  double value;
  double min;
  double max;
  unsigned int can_id;
  unsigned int can_data_offset;
  unsigned int can_data_width;
  double* x_values;
  double* y_values;
  int number_of_interpolation_points;
  unsigned int row;
  unsigned int column;
};

Sensor *sensor_create (unsigned int row, unsigned int column, const char *name, int can_id, int offset, int width) {
  Sensor *s = calloc (1, sizeof *s);
  s -> row = row;
  s -> column = column;
  s -> name  = strdup (name);
  s -> can_id = can_id;
  s -> can_data_offset = offset;
  s -> can_data_width = width;
  s -> row = row;
  s -> column = column;

  return s;
}

void sensor_destroy (Sensor *s) {
  free (s -> name);
  free (s);
}

const char *sensor_name (const Sensor *s) { return s -> name; }
double sensor_value (const Sensor *s)     { return s -> value; }
double sensor_min (const Sensor *s)       { return s -> min; }
double sensor_max (const Sensor *s)       { return s -> max; }
int sensor_can_id (const Sensor *s)       { return s -> can_id; }
int sensor_can_data_offset (const Sensor *s)       { return s -> can_data_offset; }
int sensor_can_data_width (const Sensor *s)       { return s -> can_data_width; }
double* sensor_x_values (const Sensor *s)       { return s -> x_values; }
double* sensor_y_values (const Sensor *s)       { return s -> y_values; }
int sensor_number_of_interpolation_points (const Sensor *s)       { return s -> number_of_interpolation_points; };

void sensor_set_x_values (Sensor *s, double *v, int n) { s -> x_values = v; s -> number_of_interpolation_points = n; };
void sensor_set_y_values (Sensor *s, double *v, int n) { s -> y_values = v; s -> number_of_interpolation_points = n; };
void sensor_set_value (Sensor *s, double v) {
  if  (v < s -> min) v = s -> min;
  if  (v > s -> max) v = s -> max;
  s -> value = v;
}

int sensor_row (const Sensor *s)       { return s -> row; }
int sensor_column (const Sensor *s)       { return s -> column; }

