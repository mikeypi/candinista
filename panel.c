#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>

#include "units.h"
#include "candinista.h"
#include "sensor.h"
#include "panel.h"


/* base functions (work for all panel types) */
unsigned int panel_get_x_index (const Panel* g)            { return (g -> x_index); }
unsigned int panel_get_y_index (const Panel* g)            { return (g -> y_index); }
unsigned int panel_get_z_index (const Panel* g)            { return (g -> z_index); }
unsigned int panel_get_timeout (const Panel* g)            { return (g -> timeout); }
unsigned int panel_get_id (const Panel* g)                 { return (g -> id); }
unsigned int panel_get_foreground_color(Panel* g)          { return (g -> foreground_color); }
unsigned int panel_get_background_color(Panel* g)          { return (g -> background_color); }
unsigned int panel_get_low_warn_color(Panel* g)            { return (g -> low_warn_color); }
unsigned int panel_get_high_warn_color(Panel* g)           { return (g -> high_warn_color); }
panel_type panel_get_type (Panel *g)                       { return (g -> type); }

void panel_set_border (Panel *g, unsigned char on)             { g -> border = on; }
void panel_set_id (Panel *g, unsigned int id)                  { g -> id = id; }
void panel_set_timeout (Panel *g, unsigned int tm)             { g -> timeout = tm; }
void panel_set_foreground_color(Panel* g, unsigned int color)  { g -> foreground_color = color; }
void panel_set_background_color(Panel* g, unsigned int color)  { g -> background_color = color; }
void panel_set_low_warn_color(Panel* g, unsigned int color)    { g -> low_warn_color = color; }
void panel_set_high_warn_color(Panel* g, unsigned int color)   { g -> high_warn_color = color; }
void panel_set_type (Panel *g, panel_type type)                { g -> type = type; }


/* vtable functions (implemented in the various panel files */
void panel_draw (Panel* g, void* cr)                       { g -> vtable -> draw (g, cr); }

double panel_get_min (const Panel* g)                      { return (g -> vtable -> get_min (g)); }
double panel_get_max (const Panel* g)                      { return (g -> vtable -> get_max (g)); }
double panel_get_low_warn (const Panel* g)                 { return (g -> vtable -> get_low_warn (g)); }
double panel_get_high_warn (const Panel* g)                { return (g -> vtable -> get_high_warn (g)); }
unit_type panel_get_units (Panel* g)                       { g -> vtable -> get_units (g); }
char* panel_get_label (const Panel* g)                     { return (g -> vtable -> get_label (g)); }

void panel_set_value (Panel* g, double value)              { g -> vtable -> set_value (g, value); }
void panel_set_offset (Panel* g, double value)             { g -> vtable -> set_offset (g, value); }
void panel_set_warn (Panel* g, double low, double high)    { g -> vtable -> set_warn (g, low, high); }
void panel_set_minmax (Panel* g, double min, double max)   { g -> vtable -> set_minmax (g, min, max); }
void panel_set_label (Panel* g, char* label)               { g -> vtable -> set_label (g, label); }
void panel_set_units (Panel* g, unit_type ut)              { g -> vtable -> set_units (g, ut); }
void panel_set_output_format (Panel* g, char* format)      { g -> vtable -> set_output_format (g, format); }

void panel_destroy (Panel* g) {  free (g); }


double
convert_units (double temp, unit_type to) {
  switch (to) {
  case FAHRENHEIT:
    return ((temp * 9.0 / 5.0) + 32.0);

  case PSI:
    return (temp * 14.503773773);

  default: return (temp);
    fprintf (stderr, "not convering %f to %f\n", temp);
  }
}


unsigned int
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


