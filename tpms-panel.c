#define XOFFSET 0
#define YOFFSET 40

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <gtk/gtk.h>
#include <math.h>
#include <assert.h>

#include "units.h"
#include "candinista.h"
#include "sensor.h"
#include "panel.h"
#include "cairo-misc.h"


/* concrete type */
typedef struct {
  Panel base;
  int multiplexor;
  double pressure[4];
  double temperature[4];
  double voltage[4];
  int sign[4];
  double value;
  double min;
  double max;
  double low_warn;
  double high_warn;
  unit_type units;
  unit_type pressure_units;
  unit_type temperature_units;
  char label[64];
  char* output_format;  
} TPMSPanel;


void draw_tpms_sub_panel (cairo_t* cr,
			  TPMSPanel* rp,
			  double x,
			  double y,
			  double height,
			  double width,
			  int panel_number,
			  bool top,
			  bool right) {
  double xx;
  double yy;
  char abuf[80];
  char bbuf[80];
  
  int foreground_color = get_active_foreground_color (&rp -> base, rp -> pressure[panel_number], rp -> high_warn, rp -> low_warn);
  set_rgba (cr, foreground_color, 0.9);
  
  cairo_select_font_face (
			  cr,
			  "DSEG7Classic",
			  CAIRO_FONT_SLANT_ITALIC,
			  CAIRO_FONT_WEIGHT_NORMAL
			  );

  cairo_set_font_size (cr, DEFAULT_VALUE_FONT_SIZE - 40);
  cairo_set_line_width (cr, 1.0);

  /* Print Pressure and Temperature */

  sprintf (abuf, "%.0f", rp -> pressure[panel_number]);
  sprintf (bbuf, "%.0f", rp -> temperature[panel_number]);
  
  int ax = strlen (abuf);
  int bx = strlen (bbuf);
  if (bx > ax) ax = bx;

  yy = (top) ? y + 35 : y + 100;
    
  if (right) {
    show_text_right_justified (cr, x + 80, yy, abuf, 3);
    show_text_right_justified (cr, x + 80, yy + 32, bbuf, 3);
  } else {
    show_text_right_justified (cr, x + ((ax - 1) * 18), yy, abuf, 3);
    show_text_right_justified (cr, x + ((ax - 1) * 18), yy + 32, bbuf, 3);
  }

  /* Print unit labels */
  cairo_select_font_face (cr,
			  "Orbitron",
			  CAIRO_FONT_SLANT_NORMAL,
			  CAIRO_FONT_WEIGHT_NORMAL);

  cairo_set_font_size (cr, DEFAULT_LABEL_FONT_SIZE - 15);

  yy = (top) ? y + 35 : y + 100;
  xx = (right) ? x + 95 :  x + 45 + ((ax - 1) * 18);

  sprintf (abuf, "%s", str_from_unit_enum (rp -> pressure_units));
  sprintf (bbuf, "%s", str_from_unit_enum (rp -> temperature_units));
  
  if (right) {
    show_text_right_justified (cr, 34 + xx, yy, abuf, 3); 
    show_text_right_justified (cr, 34 + xx, yy + 32, bbuf, 3); 
  } else {
    show_text_unjustified (cr, xx, yy, abuf);
    show_text_unjustified (cr, xx, yy + 32, bbuf);
  }

  /* Print panel label */
  yy = (top) ? y + 135 : y + 30;
  xx = (right) ? x + 7 : x + 105;

  cairo_set_font_size (cr, DEFAULT_LABEL_FONT_SIZE);
  char* panel_names[] = {"LF", "RF", "LR", "RR"};
  show_text_unjustified (cr, xx, yy, panel_names[panel_number]);
}

 
void draw_tpms_panel (GtkDrawingArea* area,
		      cairo_t* cr,
		      int width,
		      int height,
		      gpointer user_data)
{
  TPMSPanel* rp = user_data;
  assert (NULL != rp);

  double value = convert_units (rp -> value, rp -> units);
  int foreground_color = get_active_foreground_color (&rp -> base, value, rp -> high_warn, rp -> low_warn);
  int background_color = rp -> base.background_color;
  
  double x;
  double y;

  set_rgba (cr, background_color, 1.0);
  cairo_paint (cr);

  if (0 != rp -> base.border) {
    cairo_set_line_width (cr, 1.0);
    set_rgba (cr, rp -> base.foreground_color, 0.9);
    rounded_rectangle (cr, 5.0, 5.0, height - 10, width - 10, 5.0);
    cairo_stroke (cr);
  }

  set_rgba (cr, rp -> base.foreground_color, 0.9);
  cairo_set_line_width (cr, 2.0);
  cairo_move_to (cr, width / 2, 30);
  cairo_line_to (cr, width / 2, height - 30);
    
  cairo_move_to (cr, 30, height / 2);
  cairo_line_to (cr, width - 30, height / 2);
  cairo_stroke (cr);

  x = 5.0; y = 5.0;
  draw_tpms_sub_panel (cr, rp, x, y, (height - 20) / 2.0, (width - 20) / 2.0, 0, true, false);
  x = width / 2.0 + 5.0;
  set_rgba (cr, rp -> base.foreground_color, 0.9);
  draw_tpms_sub_panel (cr, rp, x, y, (height - 20) / 2.0, (width - 20) / 2.0, 1, true, true);
  x = 5.0; y = height / 2.0 + 5.0;
  set_rgba (cr, rp -> base.foreground_color, 0.9);
  draw_tpms_sub_panel (cr, rp, x, y, (height - 20) / 2.0, (width - 20) / 2.0, 2, false, false);
  x = width / 2.0 + 5.0;
  set_rgba (cr, rp -> base.foreground_color, 0.9);
  draw_tpms_sub_panel (cr, rp, x, y, (height - 20) / 2.0, (width - 20) / 2.0, 3, false, true);
  set_rgba (cr, rp -> base.foreground_color, 0.9);
  cairo_stroke (cr);
}


static void set_value (Panel* g, double value, int sensor_offset, int can_id) {
  TPMSPanel* rp = (TPMSPanel*) g;
  fprintf (stderr, "Can_id = %x\n", can_id);
  switch (sensor_offset % 4) {
  case 0:
    rp -> multiplexor = (int) value;
    rp -> multiplexor %= 4;
    break;
  case 1: rp -> pressure[rp -> multiplexor] = value; break; 
  case 2: rp -> temperature[rp -> multiplexor] = value;
  case 3: rp -> voltage[rp -> multiplexor] = value; break;
  case 4: rp -> sign[rp -> multiplexor] = value; break;
  }
}

void print_tpms_panel (const Panel* g)
{
}

static void set_minmax (Panel* g, double min, double max) { TPMSPanel* rp = (TPMSPanel*) g; rp -> min = min; rp -> max = max; }
static void set_warn (Panel* g, double low, double high) { TPMSPanel* rp = (TPMSPanel*) g; rp -> low_warn = low; rp -> high_warn = high; }

static void set_units (Panel* g, unit_type ut)  {
  TPMSPanel* rp = (TPMSPanel*) g;
  switch (ut) {
  case CELSIUS: 
  case FAHRENHEIT:
    rp -> temperature_units = ut;
    break;
  case BAR:
  case PSI:
  case KPA:
  default: 
    rp -> pressure_units = ut;
  }
}

static char* get_output_format (const Panel* g) {TPMSPanel* rp = (TPMSPanel*) g; return (rp -> output_format); }
static void set_output_format (Panel* g, char* format) { TPMSPanel* rp = (TPMSPanel*) g; rp -> output_format = strdup (format); }
static void set_label (Panel* g, char* label) { TPMSPanel* rp = (TPMSPanel*) g; strcpy (rp -> label, label); }

static const struct PanelVTable linear_vtable = {
  .draw = (void (*)(const struct Panel*, void *))draw_tpms_panel,  
  .print = (void (*) (const Panel*)) print_tpms_panel,
  .set_minmax = (void (*) (Panel*, double, double))set_minmax,
  .set_warn = (void (*) (Panel*, double, double)) set_warn,
  .set_units = (void (*) (Panel*, unit_type)) set_units,
  .set_label = (void (*) (Panel*, char*)) set_label,
  .set_value = (void (*) (Panel*, double, int, int)) set_value,
  .set_output_format = (void (*) (Panel*, char*)) set_output_format  
};

Panel* create_tpms_panel (PanelParameters* p) {
  TPMSPanel *lg = g_new0 (typeof (*lg), 1);

  lg = (TPMSPanel*) panel_init_base (p, (Panel*) lg);
  lg -> base.draw = (void (*)(void*, cairo_t*, int, int, void*))draw_tpms_panel;
  lg -> base.vtable = &linear_vtable;

  lg -> output_format = "%.0f";

  return (Panel*) lg;
}

