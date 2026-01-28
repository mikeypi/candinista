#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <math.h>

#include "units.h"
#include "sensor.h"
#include "panel.h"
#include "d3-array.h"
#include "yaml-loader.h"

struct Sensor {
  char  *name;
  int can_id;
  int can_data_offset;
  int can_data_width;
  double* x_values;
  double* y_values;
  int n_values;
  int x_index;
  int y_index;
  int z_index;
  double offset;
  double scale;
  int id;
};

Sensor* sensor_create (SensorParameters* st) {
  Sensor *s = calloc (1, sizeof *s);
  s -> name  = strdup (st -> name);
  s -> can_id = st -> can_id;
  s -> can_data_offset = st -> can_data_offset;
  s -> can_data_width = st -> can_data_width;
  s -> x_values = st -> x_values;
  s -> y_values = st -> y_values;
  s -> n_values = st -> n_values;
  s -> scale = isnan (st -> scale) ? 1 : st -> scale;
  s -> offset = isnan (st -> offset) ? 0 : st -> offset;
  s -> x_index = st -> x_index;
  s -> y_index = st -> y_index;
  s -> id = st -> id;
  return s;
}

void sensor_destroy (Sensor *s) {
  fprintf (stderr, "freeing %s\n", s -> name);
  free (s -> name);
  free (s -> x_values);
  free (s -> y_values);
  free (s);
}

static void print_double_array (FILE* fp, const char *label,
                               const double *v,
                               size_t count)
{
  fprintf (fp, "%s: [", label);
  for (size_t i = 0; i < count; i++) {
    fprintf (fp, "%g", v[i]);
    if (i + 1 < count)
      fprintf (fp, ", ");
  }

  printf ("]\n");
}

void sensor_print (FILE* fp, const Sensor* s)
{
    fprintf (fp, "  - name: \"%s\"\n", s -> name);

    if (0 != s -> n_values) {
      print_double_array (fp, "    x_values", s -> x_values, s -> n_values);
      print_double_array (fp, "    y_values", s -> y_values, s -> n_values);
    }

    fprintf (fp, "    can_id: 0x%x\n", s -> can_id);
    fprintf (fp, "    can_data_offset: %d\n", s -> can_data_offset);
    fprintf (fp, "    can_data_width: %d\n", s -> can_data_width);

    fprintf (fp, "    x_index: %d\n", s -> x_index);
    fprintf (fp, "    y_index: %d\n", s -> y_index);
    fprintf (fp, "    z_index: %d\n", s -> z_index);

    if (!isnan (s -> offset) && (0 != s -> offset)) {
      fprintf (fp, "    offset: %f\n", s -> offset);
    }
    if (!isnan (s -> scale) && (0 != s -> scale)) {
      fprintf (fp, "    scale: %f\n", s -> scale);
    }

    fprintf (fp, "    id: %d\n", s -> id);
}

const char *sensor_get_name (const Sensor *s)                         { return s -> name; }
int sensor_get_can_id (const Sensor *s)                               { return s -> can_id; }
int sensor_get_can_data_offset (const Sensor *s)                      { return s -> can_data_offset; }
int sensor_get_can_data_width (const Sensor *s)                       { return s -> can_data_width; }
double* sensor_get_x_values (const Sensor *s)                         { return s -> x_values; }
double* sensor_get_y_values (const Sensor *s)                         { return s -> y_values; }
int sensor_get_n_values (const Sensor *s)                             { return s -> n_values; };

int sensor_get_x_index (const Sensor *s)                              { return s -> x_index; }
int sensor_get_y_index (const Sensor *s)                              { return s -> y_index; }
int sensor_get_z_index (const Sensor *s)                              { return s -> z_index; }

double sensor_get_offset (const Sensor *s)                            { return s -> offset; }
double sensor_get_scale (const Sensor *s)                             { return s -> scale; }
int sensor_get_id (const Sensor *s)                                   { return s -> id; }


