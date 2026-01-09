#include <stdio.h>
#include <stddef.h>

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
    printf ("    name : %s\n", sensor_name (s));
    printf ("    min  : %.3f\n", sensor_min (s));
    printf ("    max  : %.3f\n", sensor_max (s));
    printf ("    value: %.3f\n", sensor_value (s));
    printf ("    can_id: %d\n", sensor_can_id (s));
    printf ("    can_data_offset: %d\n", sensor_can_data_offset (s));
    printf ("    can_data_width: %d\n", sensor_can_data_width (s));
    printf ("    output_panel: %d\n", sensor_output_id (s));
    print_double_array ("x_values", sensor_x_values(s), sensor_number_of_interpolation_points (s));
    print_double_array ("y_values", sensor_y_values(s), sensor_number_of_interpolation_points (s));
}


void dump_panel (const Panel* g, size_t index)
{
    printf ("  panel[%zu]\n", index);

    if (!g) {
        printf ("    <null>\n");
        return;
    }

    printf ("    label : %s\n", g -> label);
    printf ("    legend : %s\n", g -> legend);
    printf ("    min : %.3f\n", g -> min);
    printf ("    max : %.3f\n", g -> max);
    printf ("    low_warn : %.3f\n", g -> low_warn);
    printf ("    high_warn: %.3f\n", g -> high_warn);
    printf ("    row: %d\n", g -> row);
    printf ("    column: %d\n", g -> column);
    printf ("    panel_id: %d\n", g -> panel_id);
    
    /* Optional: identify concrete type by vtable address */
    printf ("    vtable   : %p\n", (void*)g -> vtable);
}


void configuration_print (const Configuration* d)
{
    if (!d) {
        printf ("Configuration: (null)\n");
        return;
    }

    printf ("Configuration dump\n");
    printf ("====================\n");

    printf ("Sensors (%zu)\n", d ->sensor_count);
    for (size_t i = 0; i < d -> sensor_count; i++)
        dump_sensor (d -> sensors[i], i);

    printf ("\nPanels (%zu)\n", d -> panel_count);
    for (size_t i = 0; i < d -> panel_count; i++)
        dump_panel (d -> panels[i], i);

    printf ("====================\n");
}
