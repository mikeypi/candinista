#include <stdio.h>
#include <gtk/gtk.h>

#include "units.h"
#include "candinista.h"
#include "panel.h"

/* vtable functions (implemented in the various panel files) */
void panel_draw (Panel* g, void* cr)                                   { g -> vtable -> draw (g, cr, 0, 0, NULL); }
void panel_set_value (Panel* g, double value, int offset, int can_id)  { g -> vtable -> set_value (g, value, offset, can_id); }

char*
string_from_panel_type_enum (const panel_type t) {
  switch (t) {
  case RADIAL_PRESSURE_PANEL: return ("radial_pressure");
  case RADIAL_TEMPERATURE_PANEL: return ("radial_temperature");
  case LINEAR_PRESSURE_PANEL: return ("linear_pressure");
  case LINEAR_TEMPERATURE_PANEL: return ("linear_temperature");
  case INFO_PANEL: return ("info");
  case TPMS_PANEL: return ("tpms");
  case GPS_PANEL: return ("gps");
  case UNKNOWN_PANEL: 
  default: return ("unknown panel type");
  }
}

void panel_print (FILE* fp, const Panel* g) {
  fprintf (fp, "  - type: %s\n",
	   string_from_panel_type_enum (g -> type));

  fprintf (fp, "    row_index: %d\n", g -> row_index);
  fprintf (fp, "    column_index: %d\n", g -> column_index);
  fprintf (fp, "    layer_index: %d\n", g -> layer_index);

  if (0 != g -> border) {
    fprintf (fp, "    border: %d\n", g -> border);
  }

  if (DEFAULT_FOREGROUND_RGB != g -> foreground_color) {
    fprintf (fp, "    foreground_color: 0x%x\n", g -> foreground_color);
  }

  if (DEFAULT_BACKGROUND_RGB != g -> background_color) {
    fprintf (fp, "    background_color: 0x%x\n", g -> background_color);
  }
  
  if (DEFAULT_HIGH_WARN_RGB != g -> high_warn_color) {
    fprintf (fp, "    high_warn_color: 0x%x\n", g -> high_warn_color);
  }
  
  if (DEFAULT_LOW_WARN_RGB != g -> low_warn_color) {
    fprintf (fp, "    low_warn_color: 0x%x\n", g -> low_warn_color);
  }
  
  if (0 != g -> timeout) {
    fprintf (fp, "    timeout: %d\n", g -> timeout);
  }

  fprintf (fp, "    id: %d\n", g -> id);
  g -> vtable -> print (fp, g);
}      

Panel* panel_init_base (PanelParameters* p, Panel* lg) {
  lg -> column_index = p -> column_index;
  lg -> row_index = p -> row_index;
  lg -> layer_index = p -> layer_index;
  lg -> id = p -> id;

  lg -> low_warn = p -> low_warn;
  lg -> high_warn = p -> high_warn;
  
  lg -> background_color = (1 == p -> background_color) ? DEFAULT_BACKGROUND_RGB : p -> background_color;
  lg -> foreground_color = (1 == p -> foreground_color) ? DEFAULT_FOREGROUND_RGB : p -> foreground_color;
  lg -> high_warn_color = (1 == p -> high_warn_color) ? DEFAULT_HIGH_WARN_RGB : p -> high_warn_color;
  lg -> low_warn_color = (1 == p -> low_warn_color) ? DEFAULT_LOW_WARN_RGB : p -> low_warn_color;
  
  lg -> type = p -> type;
  lg -> border = p -> border;
  lg -> timeout = p -> timeout;

  sprintf (lg -> name, "%s-%d-%d-%d-%d",
	   string_from_panel_type_enum (lg -> type),
	   lg -> column_index,
	   lg -> row_index,
	   lg -> layer_index,
	   lg -> id);

  return (lg);
}

int
get_fg_color (const Panel *g, const double value) {
  if ((g -> high_warn == g -> low_warn) || (isnan (value))) {
    return (g -> foreground_color);
  }

  if (value > g -> high_warn) {
    return (g -> high_warn_color);
  }

  if (value < g -> low_warn) {
    return (g -> low_warn_color);
  }

  return (g -> foreground_color);
}
