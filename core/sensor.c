#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <math.h>

#include "units.h"
#include "sensor.h"

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
  s -> column_index = st -> column_index;
  s -> row_index = st -> row_index;
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

  fprintf (fp, "]\n");
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

    fprintf (fp, "    column_index: %d\n", s -> column_index);
    fprintf (fp, "    row_index: %d\n", s -> row_index);

    if (!isnan (s -> offset) && (0 != s -> offset)) {
      fprintf (fp, "    offset: %f\n", s -> offset);
    }
    if (!isnan (s -> scale) && (1.0 != s -> scale)) {
      fprintf (fp, "    scale: %f\n", s -> scale);
    }

    fprintf (fp, "    id: %d\n", s -> id);
}
