#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>

#include "candinista.h"
#include "sensor.h"
#include "panel.h"


double panel_get_min (Panel* g) { return (g -> min); }
double panel_get_max (Panel* g) { return (g -> max); }
unsigned int panel_get_row (Panel* g) { return (g -> row); }
unsigned int panel_get_column (Panel* g) { return (g -> column); }
unsigned int panel_get_panel_id (Panel* g) { return (g -> panel_id); }
void panel_set_value (Panel* g, double value) { g -> value = value; }
void panel_set_offset (Panel* g, double value) { g -> offset = value; }
void panel_set_warn (Panel* g, double low, double high) { g -> low_warn = low; g -> high_warn = high; }
void panel_set_minmax (Panel* g, double low, double high) { g -> min = low; g -> max = high; }
void panel_set_coordinates (Panel* g, unsigned int row, unsigned int column) { g -> row = row; g -> column = column;}
void panel_set_label (Panel *g, char* label) { strcpy (g -> label, label); }
void panel_set_legend (Panel *g, char* legend) { strcpy (g -> legend, legend); }
void panel_set_border (Panel *g, unsigned char on) { g -> border = on; }
void panel_set_units (Panel *g, unit_type ut) { g -> units = ut; }
void panel_set_panel_id (Panel *g, unsigned int id) { g -> panel_id = id; }
void panel_draw (Panel* g, void* cr) { g -> vtable -> draw (g, cr); }
void panel_destroy (Panel* g) {  free (g); }

double
convert_units (double temp, unit_type to) {
  switch (to) {
  case FAHRENHEIT: return ((temp * 9.0 / 5.0) + 32.0);
  case PSI: return (temp * 14.503773773);
  default: return (temp);
  }
}


