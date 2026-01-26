#include <stdio.h>
#include <stddef.h>
#include <gtk/gtk.h>

#include "d3-array.h"
#include "units.h"
#include "candinista.h"
#include "yaml-loader.h"
#include "yaml-printer.h"
#include "sensor.h"
#include "panel.h"
    

static void print_double_array (const char *label,
                               const double *v,
                               size_t count)
{
  //    printf ("    %s:[", label);
  printf ("%s: [", label);
    for (size_t i = 0; i < count; i++) {
        printf ("%g", v[i]);
        if (i + 1 < count)
            printf (", ");
    }

    printf ("]\n");
}


static void dump_sensor (const Sensor* s, size_t index)
{
  //    printf ("#  Sensor[%zu]\n", index);
    printf ("  - name: \"%s\"\n", sensor_get_name (s));

    if (0 != sensor_get_n_values (s)) {
      print_double_array ("    x_values", sensor_get_x_values (s), sensor_get_n_values (s));
      print_double_array ("    y_values", sensor_get_y_values (s), sensor_get_n_values (s));
      //      printf ("#    n_values: %d\n", sensor_get_n_values (s));
    }
    printf ("    can_id: 0x%x\n", sensor_get_can_id (s));
    printf ("    can_data_offset: %d\n", sensor_get_can_data_offset (s));
    printf ("    can_data_width: %d\n", sensor_get_can_data_width (s));

    printf ("    x_index: %d\n", sensor_get_x_index (s));
    printf ("    y_index: %d\n", sensor_get_y_index (s));
    printf ("    z_index: %d\n", sensor_get_z_index (s));

    if (0 != sensor_get_offset (s)) {
      printf ("    offset: %f\n", sensor_get_offset (s));
    }
    if (0 != sensor_get_scale (s)) {
      printf ("    scale: %f\n", sensor_get_scale (s));
    }

    printf ("    id: %d\n", sensor_get_id (s));
    printf ("\n");
}


static char*
str_from_type_enum (const unit_type t) {
  switch (t) {
  case RADIAL_PRESSURE_PANEL: return ("radial_pressure");
  case LINEAR_PRESSURE_PANEL: return ("linear_pressure");
  case RADIAL_TEMPERATURE_PANEL: return ("radial_temperature");
  case LINEAR_TEMPERATURE_PANEL: return ("linear_temperature");
  case INFO_PANEL: return ("info");
  case TPMS_PANEL: return ("tpms");
  case GPS_PANEL: return ("gps");
  default: return ("unknown panel type");
  }
}


void dump_panel (const Panel* g, size_t index)
{
  //    printf ("#  panel[%zu]\n", index);

    if (!g) {
      //        printf ("#    <null panel>\n\n");
        return;
    }

    char* t =  panel_get_label (g);
    printf ("  - type: %s\n", str_from_type_enum (g -> type));
    if ((NULL != t) && ('\0' != t[0])) printf ("    label: \"%s\"\n", t);
    printf ("    x_index: %d\n", g -> x_index);
    printf ("    y_index: %d\n", g -> y_index);
    printf ("    z_index: %d\n", g -> z_index);
    printf ("    border: %d\n", g -> border);
    printf ("    foreground_color: 0x%x\n", g -> foreground_color);
    printf ("    background_color: 0x%x\n", g -> background_color);
    printf ("    high_warn_color: 0x%x\n", g -> high_warn_color);
    printf ("    low_warn_color: 0x%x\n", g -> low_warn_color);
    printf ("    timeout: %d\n", g -> timeout);
    printf ("    id: %d\n", g -> id);

    if ((UNKNOWN_PANEL != g -> type) && (INFO_PANEL != g -> type)) {
      printf ("    min_value: %.3f\n", panel_get_min (g));
      printf ("    max_value: %.3f\n", panel_get_max (g));
      printf ("    low_warn : %.3f\n", panel_get_low_warn (g));
      printf ("    high_warn: %.3f\n", panel_get_high_warn (g));
      printf ("    output_format: \"%s\"\n", panel_get_output_format (g));
      printf ("    units: \"%s\"\n", str_from_unit_enum (panel_get_units (g)));
    }

    if (TPMS_PANEL == g -> type) {
        printf ("    pressure_units: \"%s\"\n", str_from_unit_enum (panel_get_pressure_units (g)));
        printf ("    temperature_units: \"%s\"\n", str_from_unit_enum (panel_get_temperature_units (g)));
    }
    
    printf ("\n");
}


void configuration_print (const Configuration* d)
{
    if (!d) {
        printf ("Configuration: (null)\n");
        return;
    }
#if 0
    printf ("Configuration dump\n");
    printf ("====================\n");

    printf ("sensors: %d panels: %d\n", d -> sensor_count, d -> panel_count);


    printf ("panel array: %d X %d X %d\n",
	    get_x_dimension_from_d3_array (d -> panel_array),
	    get_y_dimension_from_d3_array (d -> panel_array),
	    get_z_dimension_from_d3_array (d -> panel_array));
#endif    

    printf ("sensors:\n");
    for (size_t i = 0; i < d -> sensor_count; i++)
        dump_sensor (d -> sensors[i], i);

    printf ("\npanels:\n");
    for (size_t i = 0; i < d -> panel_count; i++)
        dump_panel (d -> panels[i], i);
}
