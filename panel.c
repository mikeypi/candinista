#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>

#include "units.h"
#include "candinista.h"
#include "sensor.h"
#include "panel.h"


/* base functions (work for all panel types) */
int panel_get_x_index (const Panel* g)            { return (g -> x_index); }
int panel_get_y_index (const Panel* g)            { return (g -> y_index); }
int panel_get_z_index (const Panel* g)            { return (g -> z_index); }
int panel_get_timeout (const Panel* g)            { return (g -> timeout); }
int panel_get_id (const Panel* g)                 { return (g -> id); }
int panel_get_foreground_color(const Panel* g)    { return (g -> foreground_color); }
int panel_get_background_color(const Panel* g)    { return (g -> background_color); }
int panel_get_low_warn_color(const Panel* g)      { return (g -> low_warn_color); }
int panel_get_high_warn_color(const Panel* g)     { return (g -> high_warn_color); }
panel_type panel_get_type (const Panel *g)                 { return (g -> type); }

void panel_set_border (Panel *g, unsigned char on)             { g -> border = on; }
void panel_set_id (Panel *g, int id)                  { g -> id = id; }
void panel_set_timeout (Panel *g, int tm)             { g -> timeout = tm; }
void panel_set_foreground_color(Panel* g, int color)  { g -> foreground_color = color; }
void panel_set_background_color(Panel* g, int color)  { g -> background_color = color; }
void panel_set_low_warn_color(Panel* g, int color)    { g -> low_warn_color = color; }
void panel_set_high_warn_color(Panel* g, int color)   { g -> high_warn_color = color; }
void panel_set_type (Panel *g, panel_type type)                { g -> type = type; }


/* vtable functions (implemented in the various panel files) */
void panel_draw (Panel* g, void* cr)                       { g -> vtable -> draw (g, cr); }

double panel_get_min (const Panel* g)                      { return (g -> vtable -> get_min (g)); }
double panel_get_max (const Panel* g)                      { return (g -> vtable -> get_max (g)); }
double panel_get_low_warn (const Panel* g)                 { return (g -> vtable -> get_low_warn (g)); }
double panel_get_high_warn (const Panel* g)                { return (g -> vtable -> get_high_warn (g)); }
unit_type panel_get_units (const Panel* g)                 { return (g -> vtable -> get_units (g)); }
unit_type panel_get_pressure_units (const Panel* g)        { return (g -> vtable -> get_pressure_units (g)); }
unit_type panel_get_temperature_units (const Panel* g)     { return (g -> vtable -> get_temperature_units (g)); }
char* panel_get_label (const Panel* g)                     { return (g -> vtable -> get_label (g)); }
char* panel_get_output_format (const Panel* g)             { return (g -> vtable -> get_output_format (g)); }

void panel_set_minmax (Panel* g, double min, double max)   { g -> vtable -> set_minmax (g, min, max); }
void panel_set_warn (Panel* g, double low, double high)    { g -> vtable -> set_warn (g, low, high); }
void panel_set_units (Panel* g, unit_type ut)              { g -> vtable -> set_units (g, ut); }
void panel_set_label (Panel* g, char* label)               { g -> vtable -> set_label (g, label); }

void panel_set_output_format (Panel* g, char* format)      { g -> vtable -> set_output_format (g, format); }
void panel_set_value (Panel* g, double value, int offset)  { g -> vtable -> set_value (g, value, offset); }


void panel_destroy (Panel* g) { free (g); }


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


