#define XOFFSET 0
#define YOFFSET 40

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
#include "candinista.h"
#include "panel_specs.h"

void draw_text_panel (GtkDrawingArea* area,
				cairo_t* cr,
				int width,
				int height,
				gpointer user_data)
{
  (void) area;
  TextPanel* rp = user_data;
  char buffer[80];

  assert (NULL != rp);

  double value = convert_units (rp -> value, rp -> units);
  int foreground_color = get_fg_color ((Panel*)rp, value);
  int background_color = rp -> base.background_color;
  
  set_rgba (cr, background_color, 1.0);
  cairo_paint (cr);
  
  if (0 != rp -> base.border) {
    cairo_set_line_width (cr, 1.0);
    set_rgba (cr, rp -> base.foreground_color, 1.0);
    rounded_rectangle (cr, 5.0, 5.0, height - 10, width - 10, 5.0);
    cairo_stroke (cr);
  }

  /*
   * Draw labels and current value
   */
  cairo_select_font_face (
			  cr,
			  "Orbitron",
			  CAIRO_FONT_SLANT_NORMAL,
			  CAIRO_FONT_WEIGHT_NORMAL
			  );

  cairo_set_font_size (cr, DEFAULT_LABEL_FONT_SIZE);
  set_rgba (cr, foreground_color, 1.0);

  cairo_set_font_size (cr, DEFAULT_LABEL_FONT_SIZE - 4);
  sprintf (buffer, "%s", str_from_unit_enum (rp -> units));
  show_text_left_justified (cr, 238 + XOFFSET + 40, 65 + YOFFSET, buffer);
  cairo_set_font_size (cr, DEFAULT_LABEL_FONT_SIZE);

  // Print Label
  if ('\0' != rp -> label[0]) {
    sprintf (buffer, "%s", rp -> label);
    show_text_unjustified (cr, 20 + XOFFSET, 10 + YOFFSET, buffer);
    cairo_set_font_size (cr, DEFAULT_LABEL_FONT_SIZE);
  }
    
  // Print value field and burn-in 

  cairo_select_font_face (
			  cr,
			  "DSEG7Classic",
			  CAIRO_FONT_SLANT_ITALIC,
			  CAIRO_FONT_WEIGHT_NORMAL
			  );

  cairo_set_font_size (cr, DEFAULT_VALUE_FONT_SIZE + 20);
  sprintf (buffer, rp -> output_format, value);
    
  if (3 < strlen (buffer)) {
    show_text_right_justified (cr, width / 2.0 - 20, 180 + YOFFSET, buffer, 4);
    set_rgba (cr, foreground_color, 0.14);
//    show_text_burn_in (cr, width / 2.0, 160 + YOFFSET, buffer, 4);
//    set_rgba (cr, foreground_color, 1.0);
//    show_text_box (cr, width / 2.0, 160 + YOFFSET, buffer, 4);
  } else {
    show_text_right_justified (cr, width / 2.0 - 20, 180 + YOFFSET, buffer, 3);
    set_rgba (cr, foreground_color, 0.14);
    //  show_text_burn_in (cr, width / 2.0, 160 + YOFFSET, buffer, 3);
//    set_rgba (cr, foreground_color, 1.0);
//    show_text_box (cr, width / 2.0, 160 + YOFFSET, buffer, 3);
  }
}

void print_text_panel (FILE* fp, const Panel* g) {
  TextPanel* rp = (TextPanel*) g;

  fprintf (fp, "    label: \"%s\"\n", rp -> label);
  fprintf (fp, "    low_warn : %.1f\n", rp -> base.low_warn);
  fprintf (fp, "    high_warn: %.1f\n", rp -> base.high_warn);
  fprintf (fp, "    output_format: \"%s\"\n", rp -> output_format);
  fprintf (fp, "    units: \"%s\"\n", str_from_unit_enum (rp -> units));

}

static void set_value (Panel* g, double value, int sensor_count, int can_id) {
  (void) sensor_count;
  (void) can_id;

  TextPanel* rp = (TextPanel*) g;
  rp -> value = value;
}

static const struct PanelVTable text_vtable = {
  .draw = (void (*)(void *, cairo_t*, int, int, void*)) draw_text_panel,
  .print = (void (*) (FILE*, const Panel*)) print_text_panel,
  .set_value = (void (*) (Panel*, double, int, int)) set_value,
};

Panel* create_text_panel (PanelParameters* p) {
  TextPanel *lg = g_new0 (typeof (*lg), 1);
  
  if (p -> min > p -> max) {
    fprintf (stderr, "error: min greater than max for text panel\n");
  }

  lg = (TextPanel*) panel_init_base (p, (Panel*) lg);
  lg -> base.draw = (void (*)(void*, cairo_t*, int, int, void*))draw_text_panel;
  lg -> base.vtable = &text_vtable;
  
  lg -> output_format = DEFAULT_OUTPUT_FORMAT;
  lg -> low_warn_color = p -> low_warn_color;
  lg -> high_warn_color = p -> high_warn_color;
  lg -> units = p -> units;
  strcpy (lg -> label, p -> label);
  
  return (Panel*) lg;
}
