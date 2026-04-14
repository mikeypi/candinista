#include <stdio.h>
#include <stddef.h>
#include <gtk/gtk.h>

#include "units.h"
#include "sensor.h"
#include "panel.h"
#include "yaml-loader.h"
#include "yaml-printer.h"
#include "candinista.h"

    
void sensor_print_config (FILE* fp, Configuration* cfg) {
  for (int i = 0; i < cfg -> sensor_count; i++) {
    sensor_print (fp, &cfg -> sensors[i]);
    fprintf (fp, "\n");
  }
}

void configuration_print (FILE* fp, const Configuration* d) {
  if (!d) {
    fprintf (fp, "Configuration: (null)\n");
    return;
  }

  fprintf (fp, "sensors:\n");
  for (int i = 0; i < d -> sensor_count; i++) {
    sensor_print (fp, &d -> sensors[i]);
    fprintf (fp, "\n");
  }

  fprintf (fp, "\npanels:\n");
  for (int i = 0; i < d -> panel_count; i++) {
    panel_print (fp, d -> panels[i]);
    fprintf (fp, "\n");
  }
}

void group_print(FILE *fp, Configuration *cfg) {
    sensor_group *sg = cfg -> sensor_groups;
    while (sg < cfg -> sensor_groups + cfg -> sensor_group_count) {
      Sensor* s = sg -> first;
      fprintf(fp, "sensor group: %ld, can_id: %x\n",
              ((unsigned long) sg - (unsigned long) cfg -> sensor_groups) / sizeof (sensor_group), sg -> can_id);
      while (s <= sg -> last) {
        fprintf(fp,
                "\tsensor: %ld, name: %s, can_id: %x, row_index: %d, "
                "column_index: %d\n",
                ((unsigned long)s - (unsigned long)sg -> first) / sizeof(Sensor),
                s -> name, s -> can_id, s -> row_index, s -> column_index);

        fprintf(fp, "\t\toffset: %d, width: %d, id: %d\n", s -> can_data_offset, s -> can_data_width, s -> id);

        s++;
      }

      fprintf (fp, "\n");
      sg++;
    }

    panel_group* pg = cfg -> panel_groups;
    while (pg < cfg -> panel_groups + cfg -> panel_group_count) {
      fprintf(fp, "panel group: %ld\n",
              ((unsigned long) pg - (unsigned long) cfg -> panel_groups) / sizeof (panel_group));

      Panel** p = pg -> first;
      int count = 0;

      while (p <= pg -> last) {

        fprintf(fp, "\tpanel: %d, name: %s", count++, (*p) -> name);
        fprintf (fp, " row_index: %d, column_index: %d, layer_index %d\n", 
                 (*p) -> row_index, (*p) -> column_index,  (*p) -> layer_index);
        p++;
      }

      fprintf (fp, "\n");
      pg++;
    }
}
