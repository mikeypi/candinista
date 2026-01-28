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
    

void configuration_print (const Configuration* d)
{
    if (!d) {
        printf ("Configuration: (null)\n");
        return;
    }

    printf ("sensors:\n");
    for (size_t i = 0; i < d -> sensor_count; i++) {
        sensor_print (d -> sensors[i]);
	printf ("\n");
    }

    printf ("\npanels:\n");
    for (size_t i = 0; i < d -> panel_count; i++) {
        panel_print (d -> panels[i]);
	printf ("\n");
    }
}
