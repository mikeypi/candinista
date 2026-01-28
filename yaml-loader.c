#include <yaml.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <ctype.h>
#include <yaml.h>
#include <gtk/gtk.h>

#include "units.h"
#include "d3-array.h"
#include "candinista.h"
#include "yaml-loader.h"
#include "cairo-misc.h"
#include "sensor.h"
#include "panel.h"

static unit_type
enum_from_type_str (const char* temp) {
  char buffer[80];
  int i;
  for (i = 0; i < strlen (temp); i++) {
    buffer[i] = tolower (temp[i]);
  }

  buffer[i] = '\0';

  if ((0 == strcmp (buffer, "radial_pressure"))) { return (RADIAL_PRESSURE_PANEL); }
  if ((0 == strcmp (buffer, "linear_pressure"))) { return (LINEAR_PRESSURE_PANEL); }
  if ((0 == strcmp (buffer, "radial_temperature"))) { return (RADIAL_TEMPERATURE_PANEL); }
  if ((0 == strcmp (buffer, "linear_temperature"))) { return (LINEAR_TEMPERATURE_PANEL); }
  if ((0 == strcmp (buffer, "info"))) { return (INFO_PANEL); }
  if ((0 == strcmp (buffer, "tpms"))) { return (TPMS_PANEL); }
  if ((0 == strcmp (buffer, "gps"))) { return (GPS_PANEL); }

  fprintf (stderr, "unknown panel type %s\n", temp);
  return (UNKNOWN_PANEL);
}

static double scalar_double (yaml_event_t *event) {
  return atof ( (char *)event -> data.scalar.value);
}

static void load_double_array (yaml_parser_t *parser,
			       yaml_event_t* last_event,
			       double **out,
			       size_t *count) {
  yaml_event_t event;
  size_t capacity = 8;
  *count = 0;
  *out = malloc (capacity * sizeof (double));

  (*out)[ (*count)++] = scalar_double (last_event);
  
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


Configuration* configuration_load_yaml (const char *path) {
  Configuration* d = (Configuration*) calloc (sizeof (Configuration), 1);

  FILE *f = fopen (path, "r");
  if (!f) {
    free (d);
    return (NULL);
  }
  
  yaml_parser_t parser;
  yaml_event_t event;

  yaml_parser_initialize (&parser);
  yaml_parser_set_input_file (&parser, f);

  char key[32] = {0};
  int in_sensors = 0;
  int in_panels = 0;
  SensorParameters st = {0};
  PanelParameters gt = {0};

  gt.background_color = gt.foreground_color = gt.high_warn_color = gt.low_warn_color = 1;
  st.scale = st.offset = NAN;
  
  while (yaml_parser_parse (&parser, &event)) {
    if (event.type == YAML_SCALAR_EVENT) {
      const char *v = (const char *)event.data.scalar.value;
      if (!strcmp (v, "sensors")) in_sensors = 1, in_panels = 0;
      else if (!strcmp (v, "panels")) in_panels = 1, in_sensors = 0;
      else if (!key[0]) strncpy (key, v, sizeof key - 1);
      else {
	if (in_sensors) {
	  if (!strcmp (key, "name")) strncpy (st.name, v, 63);
	  else if (!strcmp (key, "can_data_offset")) st.can_data_offset = atoi (v);
	  else if (!strcmp (key, "can_data_width")) st.can_data_width = atoi (v);
	  else if (!strcmp (key, "x_values")) { load_double_array (&parser, &event, &st.x_values, &st.n_values); }
	  else if (!strcmp (key, "y_values"))  { load_double_array (&parser, &event, &st.y_values, &st.n_values);}
	  else if (!strcmp (key, "x_index")) st.x_index = atoi (v);
	  else if (!strcmp (key, "y_index")) st.y_index = atoi (v);
	  else if (!strcmp (key, "can_id")) st.can_id = strtol(v, NULL, 16);
	  else if (!strcmp (key, "scale")) st.scale = atof (v);
	  else if (!strcmp (key, "offset")) st.offset = atof (v);
	  else if (!strcmp (key, "id")) st.id = atoi (v);
	} else if (in_panels) {
	  if (!strcmp (key, "type")) { gt.type = enum_from_type_str (v); }
	  else if (!strcmp (key, "low_warn")) gt.low_warn = atof (v);
	  else if (!strcmp (key, "high_warn")) gt.high_warn = atof (v);
	  else if (!strcmp (key, "min_value")) gt.min = atof (v);
	  else if (!strcmp (key, "max_value")) gt.max = atof (v);
	  else if (!strcmp (key, "label")) strncpy (gt.label, v, 63);
	  else if (!strcmp (key, "border")) gt.border = atoi (v);	 
	  else if (!strcmp (key, "x_index")) gt.x_index = atoi (v);
	  else if (!strcmp (key, "y_index")) gt.y_index = atoi (v);
	  else if (!strcmp (key, "z_index")) gt.z_index = atoi (v);
	  else if (!strcmp (key, "timeout")) gt.timeout = atoi (v);
	  else if (!strcmp (key, "foreground_color")) gt.foreground_color = strtol(v, NULL, 16);
	  else if (!strcmp (key, "background_color")) gt.background_color = strtol(v, NULL, 16);
	  else if (!strcmp (key, "low_warn_color")) gt.low_warn_color = strtol(v, NULL, 16); 
	  else if (!strcmp (key, "high_warn_color")) gt.high_warn_color = strtol(v, NULL, 16);
	  else if (!strcmp (key, "id")) gt.id = atoi (v);
	  else if (!strcmp (key, "output_format")) strncpy (gt.output_format, v, 63);
	  else if (!strcmp (key, "units")) { gt.units = enum_from_unit_str (v); }
	  else if (!strcmp (key, "pressure_units")) { gt.pressure_units = enum_from_unit_str (v); }
	  else if (!strcmp (key, "temperature_units")) { gt.temperature_units = enum_from_unit_str (v); }
	}

	key[0] = 0;
      }
    }

    if (event.type == YAML_MAPPING_END_EVENT) {
      if (in_sensors && st.name[0] && (0 != st.can_id)) {
	interpolation_array_sort (st.x_values, st.y_values, st.n_values);
	Sensor *s = sensor_create (&st);
	if (st.x_index > d -> x_dimension) d -> x_dimension = st.x_index;
	if (st.y_index > d -> y_dimension) d -> y_dimension = st.y_index;
	d -> sensors = realloc (d -> sensors, sizeof *d -> sensors * (d -> sensor_count + 1));
	d -> sensors[d -> sensor_count++] = s;
	memset (&st, 0, sizeof st);
	st.scale = st.offset = NAN;
      }

      if (in_panels && gt.type != UNKNOWN_PANEL) {
	Panel *g;
	switch (gt.type) {
	case RADIAL_PRESSURE_PANEL:
	case RADIAL_TEMPERATURE_PANEL:
	  g = create_radial_gauge_panel (&gt);
	  break;
	  
	case LINEAR_PRESSURE_PANEL:
	case LINEAR_TEMPERATURE_PANEL:
	  g = create_linear_gauge_panel (&gt);
	  break;
	  
        case INFO_PANEL:
	  g = create_info_panel (&gt);
	  break;
	  
        case TPMS_PANEL:
	  g = create_tpms_panel (&gt);
	  break;
	  
	case GPS_PANEL:
	  g = create_gps_panel (&gt);
	  break;
	}
	
	if (gt.x_index > d -> x_dimension) d -> x_dimension = gt.x_index;
	if (gt.y_index > d -> y_dimension) d -> y_dimension = gt.y_index;
	if (gt.z_index > d -> panel_z_dimension) d -> panel_z_dimension = gt.z_index;
	d -> panels = realloc (d -> panels, sizeof *d -> panels * (d -> panel_count + 1));
	d -> panels[d -> panel_count++] = g;
	memset (&gt, 0, sizeof gt);

	/* The color 0x1 cannot be used. Sorry. */
	gt.background_color = gt.foreground_color = gt.high_warn_color = gt.low_warn_color = 1;
	gt.output_format[0] = '\0';
	gt.type = UNKNOWN_PANEL;
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

void build_tables (Configuration* cfg) {
  int i;
  int j;
  int k;
  
  cfg -> x_dimension += 1;
  cfg -> y_dimension += 1;
  cfg -> panel_z_dimension += 1;
  cfg -> sensor_z_dimension += 1;

  cfg -> panel_array = new_d3_array (cfg -> x_dimension, cfg -> y_dimension, cfg -> panel_z_dimension);

  Panel** p = cfg -> panels;
  while (p < cfg -> panels + cfg -> panel_count) {
    i = panel_get_x_index (*p);
    j = panel_get_y_index (*p);
    k = panel_get_z_index (*p);
    set_item_in_d3_array (cfg -> panel_array, *p, i, j, k);
    p++;
  }

  cfg -> sensor_array = new_d2_vp_array (cfg -> x_dimension, cfg -> y_dimension);

  Sensor** s = cfg -> sensors;
  while (s < cfg -> sensors + cfg -> sensor_count) {
    i = sensor_get_x_index (*s);
    j = sensor_get_y_index (*s);
    set_item_in_d2_vp_array (cfg -> sensor_array, *s, i, j);
    s++;
  }

  cfg -> active_z_index = new_d2_int_array (cfg -> x_dimension, cfg -> y_dimension);
}

Panel* cfg_get_panel (Configuration* cfg, int i, int j, int k) {
  return (get_item_in_d3_array (cfg -> panel_array, i, j, k));
}

int get_active_z  (Configuration* cfg, int i, int j) {
  return (get_item_in_d2_int_array (cfg -> active_z_index, i, j));
}

void set_active_z (Configuration* cfg, int x_index, int y_index, int value) {
  set_item_in_d2_int_array (cfg -> active_z_index, value, x_index, y_index);
}

void configuration_free (Configuration *d) {
  for (size_t i = 0; i < d -> panel_count; i++)
    panel_destroy (d -> panels[i]);
  for (size_t i = 0; i < d -> sensor_count; i++)
    sensor_destroy (d -> sensors[i]);
  free (d -> panels);
  free (d -> sensors);
}

