#include <stdio.h>
#include <stddef.h>
#include <gtk/gtk.h>

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
    printf ("  Sensor[%zu]\n", index);
    printf ("    id: %d\n", sensor_get_id (s));
    printf ("    name : %s\n", sensor_get_name (s));
    printf ("    can_id: %x\n", sensor_get_can_id (s));
    printf ("    can_data_offset: %d\n", sensor_get_can_data_offset (s));
    printf ("    can_data_width: %d\n", sensor_get_can_data_width (s));
    print_double_array ("x_values:", sensor_get_x_values(s), sensor_get_n_values (s));
    print_double_array ("y_values:", sensor_get_y_values(s), sensor_get_n_values (s));
    printf ("\n\tLayout: x_index: %d y_index: %d\n", sensor_get_x_index (s), sensor_get_y_index (s));
    printf ("\n");
}


void dump_panel (const Panel* g, size_t index)
{
    printf ("  panel[%zu]\n", index);

    if (!g) {
        printf ("    <null panel>\n\n");
        return;
    }

    if ((UNKNOWN_PANEL != g -> type) && (INFO_PANEL != g -> type)) {
      printf ("    label : %s\n", panel_get_label (g));
      printf ("    id: %d\n", g -> id);
      printf ("    min : %.3f\n", panel_get_min (g));
      printf ("    max : %.3f\n", panel_get_max (g));
      printf ("    low_warn : %.3f\n", panel_get_low_warn (g));
      printf ("    high_warn: %.3f\n", panel_get_high_warn (g));
      printf ("    offset: %.3f\n", panel_get_offset (g));
      printf ("    units: %s\n", str_from_unit_enum (panel_get_units (g)));
    }
    
    printf ("\n\tLayout: x_index: %d y_index: %d z_index: %d\n", g -> x_index, g -> y_index, g -> z_index);
    printf ("\t\tborder: %d\n\n", g -> border);

    printf ("    colors: foreground: %x, background: %x high_warn: %x low_warn: %x\n",
	    g -> foreground_color,
	    g -> background_color,
	    g -> high_warn_color,
	    g -> low_warn_color
	    );

    /* Optional: identify concrete type by vtable address */
    printf ("    vtable   : %p\n", (void*)g -> vtable);
    printf ("\n");
}


void configuration_print (const Configuration* d)
{
    if (!d) {
        printf ("Configuration: (null)\n");
        return;
    }

    printf ("Configuration dump\n");
    printf ("====================\n");

    printf ("sensors: %d panels: %d\n", d -> sensor_count, d -> panel_count);
    printf ("panel array: %d X %d X %d\n", d -> x_dimension, d -> y_dimension, d -> z_dimension);
    printf ("Sensors (%zu)\n", d -> sensor_count);
    for (size_t i = 0; i < d -> sensor_count; i++)
        dump_sensor (d -> sensors[i], i);

    printf ("\nPanels (%zu)\n", d -> panel_count);
    for (size_t i = 0; i < d -> panel_count; i++)
        dump_panel (d -> panels[i], i);

    printf ("====================\n");
}
