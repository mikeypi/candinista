#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>

#include "units.h"
#include "candinista.h"
#include "sensor.h"
#include "panel.h"
#include "cairo-misc.h"


/* base functions (work for all panel types) */
int panel_get_x_index (const Panel* g)            { return (g -> x_index); }
int panel_get_y_index (const Panel* g)            { return (g -> y_index); }
int panel_get_z_index (const Panel* g)            { return (g -> z_index); }
int panel_get_timeout (const Panel* g)            { return (g -> timeout); }
int panel_get_id (const Panel* g)                 { return (g -> id); }
panel_type panel_get_type (const Panel *g)        { return (g -> type); }

void panel_destroy (Panel* g) { free (g); }

/* vtable functions (implemented in the various panel files) */
void panel_draw (Panel* g, void* cr)                       { g -> vtable -> draw (g, cr); }
void panel_set_value (Panel* g, double value, int offset, int can_id)  { g -> vtable -> set_value (g, value, offset, can_id); }

static char*
printable_form_of_panel_type_enum (const unit_type t) {
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

  lg -> background_color = (1 == p -> background_color) ? XBLACK_RGB : p -> background_color;
  lg -> foreground_color = (1 == p -> foreground_color) ? XORANGE_RGB : p -> foreground_color;
  lg -> high_warn_color = (1 == p -> high_warn_color) ? XRED_RGB : p -> high_warn_color;
  lg -> low_warn_color = (1 == p -> low_warn_color) ? XBLUE_RGB : p -> low_warn_color;

  lg -> type = p -> type;
  lg -> id = p -> id;
  lg -> border = p -> border;
  lg -> timeout = p -> timeout;

  return (lg);
}

int
get_active_foreground_color (Panel *g, double value, double high_warn, double low_warn) {
  if ((high_warn == low_warn) || (isnan (value))) {
    return (g -> foreground_color);
  }

  if (value > high_warn) {
    return (g -> high_warn_color);
  }

  if (value < low_warn) {
    return (g -> low_warn_color);
  }

  return (g -> foreground_color);
}


