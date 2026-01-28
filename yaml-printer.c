#include <stdio.h>
#include <stddef.h>
#include <gtk/gtk.h>

#include "d3-array.h"
#include "units.h"
#include "candinista.h"
#include "sensor.h"
#include "panel.h"
#include "yaml-loader.h"
#include "yaml-printer.h"

    
void sensor_print_config (FILE* fp, Configuration* cfg )
{
    for (size_t i = 0; i < cfg -> sensor_count; i++) {
      sensor_print (fp, cfg -> sensors[i]);
      fprintf (fp, "\n");
    }
}

void configuration_print (FILE* fp, const Configuration* d)
{
  if (!d) {
    fprintf (fp, "Configuration: (null)\n");
    return;
  }

  fprintf (fp, "sensors:\n");
  for (size_t i = 0; i < d -> sensor_count; i++) {
    sensor_print (fp, d -> sensors[i]);
    fprintf (fp, "\n");
  }

  fprintf (fp, "\npanels:\n");
  for (size_t i = 0; i < d -> panel_count; i++) {
    panel_print (fp, d -> panels[i]);
    fprintf (fp, "\n");
  }
}
