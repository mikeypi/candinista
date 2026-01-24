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
    printf ("    %s (%zu): [", label, count);
    for (size_t i = 0; i < count; i++) {
        printf ("%g", v[i]);
        if (i + 1 < count)
            printf (", ");
    }

    printf ("]\n");
}


static void dump_sensor (const Sensor* s, size_t index)
{
    printf ("#  Sensor[%zu]\n", index);
    printf ("\t-name: \"%s\"\n", sensor_get_name (s));
    printf ("\tcan_id: 0x%x\n", sensor_get_can_id (s));
    printf ("\tcan_data_offset: %d\n", sensor_get_can_data_offset (s));
    printf ("\tcan_data_width: %d\n", sensor_get_can_data_width (s));
    if (0 != sensor_get_n_values (s)) {
      print_double_array ("\tx_values:", sensor_get_x_values (s), sensor_get_n_values (s));
      print_double_array ("\ty_values:", sensor_get_y_values (s), sensor_get_n_values (s));
      printf ("#\tn_values: %d\n", sensor_get_n_values (s));
    }
    printf ("\tx_index: %d\n", sensor_get_x_index (s));
    printf ("\ty_index: %d\n", sensor_get_y_index (s));
    printf ("\tz_index: %d\n", sensor_get_z_index (s));
    printf ("\toffset: %f\n", sensor_get_offset (s));
    printf ("\tscale: %f\n", sensor_get_scale (s));
    printf ("\tid: %d\n", sensor_get_id (s));
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
    printf ("#  panel[%zu]\n", index);

    if (!g) {
        printf ("#    <null panel>\n\n");
        return;
    }

    printf ("\t-type: \"%s\"\n", str_from_type_enum (g -> type));
    if (NULL != panel_get_label (g)) printf ("\tlabel: \"%s\"\n", panel_get_label (g));
    printf ("\tx_index: %d\n", g -> x_index);
    printf ("\ty_index: %d\n", g -> y_index);
    printf ("\tz_index: %d\n", g -> z_index);
    printf ("\tborder: %d\n", g -> border);
    printf ("\tforeground_color: 0x%x\n", g -> foreground_color);
    printf ("\tbackground_color: 0x%x\n", g -> background_color);
    printf ("\thigh_warn_color: 0x%x\n", g -> high_warn_color);
    printf ("\tlow_warn_color: 0x%x\n", g -> low_warn_color);
    printf ("\ttimeout: %d\n", g -> timeout);

    if ((UNKNOWN_PANEL != g -> type) && (INFO_PANEL != g -> type)) {
      printf ("\tid: %d\n", g -> id);
      printf ("\tmin : %.3f\n", panel_get_min (g));
      printf ("\tmax : %.3f\n", panel_get_max (g));
      printf ("\tlow_warn : %.3f\n", panel_get_low_warn (g));
      printf ("\thigh_warn: %.3f\n", panel_get_high_warn (g));
      printf ("\tunits: \"%s\"\n", str_from_unit_enum (panel_get_units (g)));
      printf ("\toutput_format: \"%s\"\n", panel_get_output_format (g));
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
