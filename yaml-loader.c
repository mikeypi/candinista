#include <yaml.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <ctype.h>
#include <yaml.h>
#include <gtk/gtk.h>

#include "candinista.h"
#include "yaml-loader.h"

static unit_type
enum_from_unit_str (const char* temp) {
  char buffer[80];
  int i;
  for (i = 0; i < strlen (temp); i++) {
    buffer[i] = tolower (temp[i]);
  }

  buffer[i] = '\0';

  if ((0 == strcmp (buffer, "celsius")) || (0 == strcmp (buffer, "c"))) { return (CELSIUS); }
  if ((0 == strcmp (buffer, "fahrenheit")) || (0 == strcmp (buffer, "f"))) { return (FAHRENHEIT); }
  if (0 == strcmp (buffer, "bar")) { return (BAR); }
  if (0 == strcmp (buffer, "psi")) { return (PSI); }
  if (0 == strcmp (buffer, "none")) { return (NONE); }
  
  fprintf (stderr, "unknown unit type\n");
  return (NONE);
}

/*
 * These are sort of fake. They can't be the real structures because the details of these structures are (or can be)
 * hidden in their implementations.
 */
typedef struct {
  char name[64];
  double min, max, value;
  int can_id;
  int can_data_offset;
  int can_data_width;
  double* x_values;
  double* y_values;
  size_t number_of_interpolation_points;
  int output_id;
} SensorTmp;


typedef struct {
  char type[16];
  double min;
  double max;
  double low_warn;
  double high_warn;
  double offset;
  unit_type units; 
  char label[64];
  char legend[64];
  int border;
  unsigned int row;
  unsigned int column;
  int panel_id;
  unsigned int timeout;
} PanelTmp;


#if 0
static Sensor *find_sensor (Configuration *d, const char *name) {
  for (size_t i = 0; i < d -> sensor_count; i++)
    if (!strcmp (sensor_name (d -> sensors[i]), name))
      return d -> sensors[i];
  return NULL;
}
#endif


void print_yaml_event_type (const yaml_event_t *event)
{
  const char *name = "UNKNOWN";

  switch (event -> type) {
  case YAML_NO_EVENT:             name = "YAML_NO_EVENT"; break;
  case YAML_STREAM_START_EVENT:   name = "YAML_STREAM_START_EVENT"; break;
  case YAML_STREAM_END_EVENT:     name = "YAML_STREAM_END_EVENT"; break;
  case YAML_DOCUMENT_START_EVENT: name = "YAML_DOCUMENT_START_EVENT"; break;
  case YAML_DOCUMENT_END_EVENT:   name = "YAML_DOCUMENT_END_EVENT"; break;
  case YAML_ALIAS_EVENT:          name = "YAML_ALIAS_EVENT"; break;
  case YAML_SCALAR_EVENT:         name = "YAML_SCALAR_EVENT"; break;
  case YAML_SEQUENCE_START_EVENT: name = "YAML_SEQUENCE_START_EVENT"; break;
  case YAML_SEQUENCE_END_EVENT:   name = "YAML_SEQUENCE_END_EVENT"; break;
  case YAML_MAPPING_START_EVENT:  name = "YAML_MAPPING_START_EVENT"; break;
  case YAML_MAPPING_END_EVENT:    name = "YAML_MAPPING_END_EVENT"; break;
  default:
    break;
  }

  printf ("YAML event: %s\n", name);
}


static double scalar_double (yaml_event_t *event) {
  return atof ( (char *)event -> data.scalar.value);
}


static void load_double_array (yaml_parser_t *parser,
			       double **out,
			       size_t *count) {
  yaml_event_t event;
  size_t capacity = 8;
  *count = 0;
  *out = malloc (capacity * sizeof (double));

  while (1) {
    yaml_parser_parse (parser, &event);

    if (event.type == YAML_SEQUENCE_END_EVENT) {
      yaml_event_delete (&event);
      break;
    }

    if (*count == capacity) {
      capacity *= 2;
      *out = realloc (*out, capacity * sizeof (double));
    }

    (*out)[ (*count)++] = scalar_double (&event);
    yaml_event_delete (&event);
  }
}


Configuration configuration_load_yaml (const char *path) {
  Configuration d = {0};
  FILE *f = fopen (path, "r");
  if (!f) return d;

  yaml_parser_t parser;
  yaml_event_t event;
  yaml_parser_initialize (&parser);
  yaml_parser_set_input_file (&parser, f);

  char key[32] = {0};
  int in_sensors = 0;
  int in_panels = 0;
  SensorTmp st = {0};
  PanelTmp gt = {0};

  while (yaml_parser_parse (&parser, &event)) {
    if (event.type == YAML_SCALAR_EVENT) {
      const char *v = (const char *)event.data.scalar.value;
      if (!strcmp (v, "sensors")) in_sensors = 1, in_panels = 0;
      else if (!strcmp (v, "panels")) in_panels = 1, in_sensors = 0;
      else if (!key[0]) strncpy (key, v, sizeof key - 1);
      else {
	if (in_sensors) {
	  if (!strcmp (key, "name")) strncpy (st.name, v, 63);
	  else if (!strcmp (key, "min")) st.min = atof (v);
	  else if (!strcmp (key, "max")) st.max = atof (v);
	  else if (!strcmp (key, "value")) st.value = atof (v);
	  else if (!strcmp (key, "can_data_offset")) st.can_data_offset = atoi (v);
	  else if (!strcmp (key, "can_data_width")) st.can_data_width = atoi (v);
	  else if (!strcmp (key, "x_values")) load_double_array (&parser, &st.x_values, &st.number_of_interpolation_points);
	  else if (!strcmp (key, "y_values")) load_double_array (&parser, &st.y_values, &st.number_of_interpolation_points);
	  else if (!strcmp (key, "output_id")) st.output_id = atoi (v);
	  else if (!strcmp (key, "can_id")) st.can_id = strtol(v, NULL, 16);
	} else if (in_panels) {
	  if (!strcmp (key, "type")) strncpy (gt.type, v, 15);
	  else if (!strcmp (key, "low_warn")) gt.low_warn = atof (v);
	  else if (!strcmp (key, "high_warn")) gt.high_warn = atof (v);
	  else if (!strcmp (key, "min_value")) gt.min = atof (v);
	  else if (!strcmp (key, "max_value")) gt.max = atof (v);
	  else if (!strcmp (key, "offset")) gt.offset = atof (v);
	  else if (!strcmp (key, "label")) strncpy (gt.label, v, 63);
	  else if (!strcmp (key, "legend")) strncpy (gt.legend, v, 63);
	  else if (!strcmp (key, "border")) gt.border = atoi (v);	 
	  else if (!strcmp (key, "row")) gt.row = atoi (v);
	  else if (!strcmp (key, "column")) gt.column = atoi (v);
	  else if (!strcmp (key, "panel_id")) gt.panel_id = atoi (v);
	  else if (!strcmp (key, "timeout")) gt.timeout = atoi (v);
	  else if (!strcmp (key, "units")) { char temp[80]; strncpy (temp, v, 15); gt.units = enum_from_unit_str (temp); }
	}

	key[0] = 0;
      }
    }

    if (event.type == YAML_MAPPING_END_EVENT) {
      if (in_sensors && st.name[0]) {
	Sensor *s = sensor_create (st.name, st.can_id, st.can_data_offset, st.can_data_width);
	interpolation_array_sort (st.x_values, st.y_values, st.number_of_interpolation_points);
	sensor_set_x_values (s, st.x_values, st.number_of_interpolation_points);
	sensor_set_y_values (s, st.y_values, st.number_of_interpolation_points);
	sensor_set_output_id (s, st.output_id);
	d.sensors = realloc (d.sensors, sizeof *d.sensors * (d.sensor_count + 1));
	d.sensors[d.sensor_count++] = s;
	memset (&st, 0, sizeof st);
      }

      if (in_panels && gt.type[0]) {
	Panel *g;
	if (0 == strcmp (gt.type, "radial")) { g = create_radial_gauge_panel (gt.row, gt.column, gt.max, gt.min); }
	else if (0 == strcmp (gt.type, "linear")) { g = create_linear_gauge_panel (gt.row, gt.column, gt.max, gt.min); }
	else if (0 == strcmp (gt.type, "info")) { g = create_info_panel (gt.row, gt.column); }

	panel_set_warn (g, gt.low_warn, gt.high_warn);
	panel_set_offset (g, gt.offset);
	panel_set_label (g, gt.label);
	panel_set_legend (g, gt.legend);
	panel_set_border (g, gt.border);
	panel_set_units (g, gt.units);
	panel_set_panel_id (g, gt.panel_id);
	panel_set_timeout (g, gt.timeout);
	d.panels = realloc (d.panels, sizeof *d.panels * (d.panel_count + 1));
	d.panels[d.panel_count++] = g;
	memset (&gt, 0, sizeof gt);
      }
    }

    if (event.type == YAML_NO_EVENT) {
      fclose (f);
      return d;
    }

    yaml_event_delete (&event);
    if (event.type == YAML_STREAM_END_EVENT) break;
  }

  yaml_parser_delete (&parser);
  fclose (f);
  return d;
}


void configuration_free (Configuration *d) {
  for (size_t i = 0; i < d -> panel_count; i++)
    panel_destroy (d -> panels[i]);
  for (size_t i = 0; i < d -> sensor_count; i++)
    sensor_destroy (d -> sensors[i]);
  free (d -> panels);
  free (d -> sensors);
}
