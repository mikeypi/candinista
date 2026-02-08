#include <stdio.h>
#include <gtk/gtk.h>

#include "units.h"
#include "candinista.h"
#include "panel.h"

/* vtable functions (implemented in the various panel files) */
void panel_draw (Panel* g, void* cr)                                   { g -> vtable -> draw (g, cr, 0, 0, NULL); }
void panel_set_value (Panel* g, double value, int offset, int can_id)  { g -> vtable -> set_value (g, value, offset, can_id); }

static char*
printable_form_of_panel_type_enum (const panel_type t) {
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

void panel_print (FILE* fp, const Panel* g)
{
    fprintf (fp, "  - type: %s\n",
	    printable_form_of_panel_type_enum (g -> type));

    fprintf (fp, "    x_index: %d\n", g -> x_index);
    fprintf (fp, "    y_index: %d\n", g -> y_index);
    fprintf (fp, "    z_index: %d\n", g -> z_index);
    fprintf (fp, "    border: %d\n", g -> border);
    fprintf (fp, "    foreground_color: 0x%x\n", g -> foreground_color);
    fprintf (fp, "    background_color: 0x%x\n", g -> background_color);
    fprintf (fp, "    high_warn_color: 0x%x\n", g -> high_warn_color);
    fprintf (fp, "    low_warn_color: 0x%x\n", g -> low_warn_color);
    fprintf (fp, "    timeout: %d\n", g -> timeout);
    fprintf (fp, "    id: %d\n", g -> id);
    g -> vtable -> print (fp, g);
}      

Panel* panel_init_base (PanelParameters* p, Panel* lg) {
  lg -> x_index = p -> x_index;
  lg -> y_index = p -> y_index;
  lg -> z_index = p -> z_index;

  lg -> low_warn = p -> low_warn;
  lg -> high_warn = p -> high_warn;
  
  lg -> background_color = (1 == p -> background_color) ? DEFAULT_BACKGROUND_RGB : p -> background_color;
  lg -> foreground_color = (1 == p -> foreground_color) ? DEFAULT_FOREGROUND_RGB : p -> foreground_color;
  lg -> high_warn_color = (1 == p -> high_warn_color) ? DEFAULT_HIGH_WARN_RGB : p -> high_warn_color;
  lg -> low_warn_color = (1 == p -> low_warn_color) ? DEFAULT_LOW_WARN_RGB : p -> low_warn_color;
  
  lg -> type = p -> type;
  lg -> id = p -> id;
  lg -> border = p -> border;
  lg -> timeout = p -> timeout;

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

