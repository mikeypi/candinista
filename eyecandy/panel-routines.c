#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <gtk/gtk.h>
#include <math.h>
#include <assert.h>

#include "units.h"
#include "sensor.h"
#include "panel.h"
#include "cairo-misc.h"
#include "gtk-glue.h"
#include "dropdowns.h"
#include "candinista.h"
#include "panel_specs.h"

GtkWidget*
widget_for_radial_panel (const RadialPanel *p, int row, int column) {
  GtkGrid* grid = GTK_GRID (gtk_grid_new ());

  gtk_grid_attach (grid, GTK_WIDGET (new_label_for_string ("min")), column, row++, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_label_for_string ("max")), column, row++, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_label_for_string ("label")), column, row++, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_label_for_string ("output_format")), column, row++, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_label_for_string ("units")), column, row++, 1, 1);

  row = 0; column++;

  gtk_grid_attach (grid, GTK_WIDGET (new_entry_for_double ((gpointer) &(p -> min))), column, row++, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_entry_for_double ((gpointer) &(p -> max))), column, row++, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_entry_for_string ((void*) p -> label)), column, row++, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_entry_for_string (p -> output_format)), column, row++, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (widget_for_unit_type_dropdown ((Panel*) p, p -> units)), column, row++, 1, 1);
  
  return (GTK_WIDGET (grid));
}


GtkWidget*
widget_for_linear_panel (const LinearPanel *p, int row, int column) {
  GtkGrid* grid = GTK_GRID (gtk_grid_new ());

  gtk_grid_attach (grid, GTK_WIDGET (new_label_for_string ("min")), column, row++, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_label_for_string ("max")), column, row++, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_label_for_string ("label")), column, row++, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_label_for_string ("output_format")), column, row++, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_label_for_string ("units")), column, row++, 1, 1);

  row = 0; column++;

  gtk_grid_attach (grid, GTK_WIDGET (new_entry_for_double ((gpointer) &(p -> min))), column, row++, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_entry_for_double ((gpointer) &(p -> max))), column, row++, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_entry_for_string ((void*) p -> label)), column, row++, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_entry_for_string (p -> output_format)), column, row++, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (widget_for_unit_type_dropdown ((Panel*) p, p -> units)), column, row++, 1, 1);
  
  return (GTK_WIDGET (grid));
}


GtkWidget*
widget_for_tpms_panel (const TPMSPanel *p, int row, int column) {
  GtkGrid* grid = GTK_GRID (gtk_grid_new ());

  gtk_grid_attach (grid, GTK_WIDGET (new_label_for_string ("min")), column, row++, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_label_for_string ("max")), column, row++, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_label_for_string ("label")), column, row++, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_label_for_string ("output_format")), column, row++, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_label_for_string ("pressure_units")), column, row++, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_label_for_string ("temperature_units")), column, row++, 1, 1);

  row = 0; column++;
  
  gtk_grid_attach (grid, GTK_WIDGET (new_entry_for_double ((gpointer) &(p -> min))), column, row++, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_entry_for_double ((gpointer) &(p -> max))), column, row++, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_entry_for_string ((void*) p -> label)), column, row++, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_entry_for_string (p -> output_format)), column, row++, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (widget_for_unit_type_dropdown ((Panel*) p, p -> pressure_units)), column, row++, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (widget_for_unit_type_dropdown ((Panel*) p, p -> temperature_units)), column, row++, 1, 1);

  return (GTK_WIDGET (grid));
}


GtkWidget*
widget_for_gps_panel (const GPSPanel *p, int row, int column) {
  GtkGrid* grid = GTK_GRID (gtk_grid_new ());

  gtk_grid_attach (grid, GTK_WIDGET (new_label_for_string ("speed units")), column, row++, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_label_for_string ("altitude units")), column, row++, 1, 1);
  
  row = 0; column++;

  gtk_grid_attach (grid, GTK_WIDGET (widget_for_unit_type_dropdown ((Panel*) p, p -> speed_units)), column, row++, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (widget_for_unit_type_dropdown ((Panel*) p,  p -> altitude_units)), column, row++, 1, 1);

  return (GTK_WIDGET (grid));
}

